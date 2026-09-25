/*
* Electra One firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

/**
 * @file Trace.cpp
 *
 * @brief Behavioural trace used by the compatibility test harness.
 */

#include "Trace.h"

#if defined(ELECTRA_TRACE) && ELECTRA_TRACE

#include "SEGGER_RTT.h"
#include "core_pins.h"

namespace Trace {

/** RTT up-channel the trace is written to. Channel 0 is the channel every
 *  J-Link host tool reads by default, and nothing else in this firmware
 *  writes to RTT, so the trace has it to itself. Its buffer is sized by
 *  BUFFER_SIZE_UP, which the Makefile sets from TRACE_BUFFER_SIZE. */
static const unsigned int channel = 0;

static uint32_t sequence = 0;
static uint32_t dropped = 0;
static bool ready = false;

/**
 * @brief Hash used to identify long messages without recording every byte.
 *
 * FNV-1a. Two messages that differ anywhere produce different hashes, which
 * is all a compatibility comparison needs.
 */
static uint32_t hashBytes(const uint8_t *data, uint16_t length)
{
    uint32_t hash = 2166136261u;

    for (uint16_t i = 0; i < length; i++) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return (hash);
}

void initialise(void)
{
    if (ready) {
        return;
    }
    SEGGER_RTT_Init();

    /* Records are dropped rather than stalling the firmware when no host is
     * reading. A gap in the sequence numbers tells the host that happened. */
    SEGGER_RTT_SetNameUpBuffer(channel, "electra-trace");
    SEGGER_RTT_SetFlagsUpBuffer(channel, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
    ready = true;
}

uint32_t droppedRecords(void)
{
    return (dropped);
}

Record::Record(const char *category) : length(0)
{
    uint32_t seq = __atomic_fetch_add(&sequence, 1, __ATOMIC_RELAXED);

    appendNum((int32_t)seq);
    appendChar(' ');
    appendNum((int32_t)micros());
    appendChar(' ');
    append(category);
}

void Record::appendChar(char c)
{
    if (length < (sizeof(buffer) - 1)) {
        buffer[length++] = c;
    }
}

void Record::append(const char *value)
{
    if (value == nullptr) {
        value = "?";
    }
    while (*value && (length < (sizeof(buffer) - 1))) {
        buffer[length++] = *value++;
    }
}

/**
 * @brief Appends a string with separators and newlines neutralised.
 *
 * Trace records are line based and space separated, so a Lua string that
 * happens to contain a space or a newline must not be able to forge a record.
 */
void Record::appendEscaped(const char *value)
{
    if (value == nullptr) {
        value = "?";
    }
    while (*value && (length < (sizeof(buffer) - 1))) {
        char c = *value++;

        if ((c == '\n') || (c == '\r')) {
            c = '`';
        } else if (c == ' ') {
            c = '_';
        } else if ((c < 0x20) || (c > 0x7e)) {
            c = '.';
        }
        buffer[length++] = c;
    }
}

void Record::appendNum(int32_t value)
{
    char digits[12];
    uint8_t count = 0;
    uint32_t magnitude;

    if (value < 0) {
        appendChar('-');
        magnitude = (uint32_t)(-(int64_t)value);
    } else {
        magnitude = (uint32_t)value;
    }

    do {
        digits[count++] = (char)('0' + (magnitude % 10));
        magnitude /= 10;
    } while (magnitude && (count < sizeof(digits)));

    while (count--) {
        appendChar(digits[count]);
    }
}

Record &Record::text(const char *value)
{
    appendChar(' ');
    appendEscaped(value);
    return (*this);
}

Record &Record::num(const char *key, int32_t value)
{
    appendChar(' ');
    append(key);
    appendChar('=');
    appendNum(value);
    return (*this);
}

Record &Record::str(const char *key, const char *value)
{
    appendChar(' ');
    append(key);
    appendChar('=');
    appendEscaped(value);
    return (*this);
}

Record &Record::bytes(const char *key, const uint8_t *data, uint16_t dataLength)
{
    static const char hexDigits[] = "0123456789ABCDEF";
    uint16_t inlineLength =
        (dataLength > maxInlineBytes) ? maxInlineBytes : dataLength;

    appendChar(' ');
    append("len=");
    appendNum((int32_t)dataLength);

    appendChar(' ');
    append(key);
    appendChar('=');

    if (data == nullptr) {
        append("null");
        return (*this);
    }

    for (uint16_t i = 0; i < inlineLength; i++) {
        appendChar(hexDigits[(data[i] >> 4) & 0x0f]);
        appendChar(hexDigits[data[i] & 0x0f]);
    }

    if (dataLength > inlineLength) {
        append("..");
        appendChar(' ');
        append("hash=");
        appendNum((int32_t)hashBytes(data, dataLength));
    }
    return (*this);
}

void Record::emit(void)
{
    if (!ready) {
        return;
    }
    appendChar('\n');

    unsigned written = SEGGER_RTT_Write(channel, buffer, length);

    if (written != length) {
        dropped++;
    }
}

/**
 * @brief Number of MIDI bytes carried by a USB MIDI event packet.
 *
 * Indexed by the code index number, the low nibble of the packet header.
 * Entries 0 and 1 are reserved and never produced by this firmware.
 */
static const uint8_t usbMidiPacketLength[16] = {
    0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1
};

void boot(const char *firmwareVersion, const char *applicationName)
{
    Record("BOOT")
        .str("fw", firmwareVersion)
        .str("app", applicationName)
        .num("trace", 1)
        .emit();
}

void mark(const char *label)
{
    Record("MARK").text(label).emit();
}

} // namespace Trace

/**
 * @brief Records one outgoing USB MIDI event packet.
 *
 * Every byte the firmware sends on the USB device port passes through
 * usb_midi_write_packed(), so this is the complete and correctly ordered
 * picture of what leaves the unit, the SysEx logger included.
 */
extern "C" void traceUsbMidiPacket(uint32_t packed)
{
    uint8_t header = (uint8_t)(packed & 0xff);
    uint8_t cable = (uint8_t)((header >> 4) & 0x0f);
    uint8_t codeIndex = (uint8_t)(header & 0x0f);
    uint8_t data[3];

    data[0] = (uint8_t)((packed >> 8) & 0xff);
    data[1] = (uint8_t)((packed >> 16) & 0xff);
    data[2] = (uint8_t)((packed >> 24) & 0xff);

    Trace::Record("MO")
        .str("if", "UsbDev")
        .num("cable", cable)
        .num("cin", codeIndex)
        .bytes("d", data, Trace::usbMidiPacketLength[codeIndex])
        .emit();
}

#endif // ELECTRA_TRACE
