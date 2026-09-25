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

/**
 * @brief Microsecond timestamp that is safe to take from anywhere.
 *
 * The trace deliberately does not call micros(). micros() ends with an
 * unconditional __enable_irq(), so calling it from an interrupt handler or
 * from inside a critical section re-enables interrupts the caller had masked,
 * and the trace runs in exactly those places. It also cannot use the debug
 * unit's cycle counter, because an attached debugger resets it.
 *
 * This reads the same two values micros() does, without touching the
 * interrupt mask. The millisecond counter is re-read to detect a tick landing
 * between the two reads, which is the only race that matters.
 */
static uint32_t traceMicros(void)
{
    const uint32_t reload = (F_CPU / 1000) - 1;
    uint32_t milliseconds;
    uint32_t current;

    do {
        milliseconds = systick_millis_count;
        current = SYST_CVR;
    } while (milliseconds != systick_millis_count);

    return (milliseconds * 1000 + (reload - current) / (F_CPU / 1000000));
}

namespace Trace {

/** RTT up-channel the trace is written to.
 *
 * Channel 1 rather than 0, because SEGGER_RTT_ConfigUpBuffer() only accepts a
 * caller supplied buffer for a non-zero channel; for channel 0 it changes the
 * flags and keeps its own small built-in buffer. The trace needs its own
 * buffer, both for the size and for the alignment described below.
 */
static const unsigned int channel = 1;

#ifndef ELECTRA_TRACE_BUFFER_SIZE
#define ELECTRA_TRACE_BUFFER_SIZE 16384
#endif

/*
 * The buffer is aligned to its own size, and that is not cosmetic.
 *
 * This part splits its RAM into two banks, SRAM_L below 0x20000000 reached
 * over the code bus and SRAM_U at and above it reached over the system bus.
 * A single access may not span the boundary between them; one that does
 * raises a bus fault. The linker lays .bss straight across the boundary, so a
 * large buffer landing there would be written across it by memcpy sooner or
 * later and take the firmware down.
 *
 * A block whose size is a power of two and whose alignment equals its size
 * cannot straddle any boundary that is a multiple of that size, and
 * 0x20000000 is such a multiple. That makes the buffer safe wherever the
 * linker decides to put it.
 */
static_assert((ELECTRA_TRACE_BUFFER_SIZE
               & (ELECTRA_TRACE_BUFFER_SIZE - 1))
                  == 0,
              "the trace buffer size must be a power of two, so that the "
              "buffer cannot straddle the SRAM_L/SRAM_U boundary");

static char traceBuffer[ELECTRA_TRACE_BUFFER_SIZE]
    __attribute__((aligned(ELECTRA_TRACE_BUFFER_SIZE)));

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
    SEGGER_RTT_ConfigUpBuffer(channel,
                              "electra-trace",
                              traceBuffer,
                              sizeof(traceBuffer),
                              SEGGER_RTT_MODE_NO_BLOCK_SKIP);
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
    appendUnsigned(traceMicros());
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

void Record::appendUnsigned(uint32_t magnitude)
{
    char digits[12];
    uint8_t count = 0;

    do {
        digits[count++] = (char)('0' + (magnitude % 10));
        magnitude /= 10;
    } while (magnitude && (count < sizeof(digits)));

    while (count--) {
        appendChar(digits[count]);
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

void logged(uint8_t level, const char *message)
{
    Record("LOG").num("lvl", level).text(message).emit();
}

void memory(const char *stage,
            int32_t freeRam,
            int32_t heapUsed,
            uint32_t vramUsed,
            uint32_t vramRejected)
{
    Record("MEM")
        .str("stage", stage)
        .num("free", freeRam)
        .num("heap", heapUsed)
        .num("vram", (int32_t)vramUsed)
        .num("rejected", (int32_t)vramRejected)
        .emit();
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
