/*
* Electra One MIDI Controller Firmware
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

#include "MidiOutput.h"
#include "Base64.h"
#include "ArduinoJson.h"
#include "App.h"
#include "Trace.h"
#include "Profile.h"
#include "MidiUsbHost.h"

/** Constructor
 *
 */
MidiOutput::MidiOutput(MidiInterface::Type newInterface, uint8_t newPort)
    : MidiJack(newInterface, newPort), channel(0), rate(0), tsLastMessage(0)
{
}

MidiOutput::MidiOutput(MidiInterface::Type newInterface,
                       uint8_t newPort,
                       uint8_t newChannel,
                       uint16_t newRate)
    : MidiJack(newInterface, newPort),
      channel(newChannel),
      rate(0),
      tsLastMessage(0)
{
}

void MidiOutput::sendSetLogger(MidiInterface::Type interface,
                               uint8_t port,
                               bool status)
{
    uint8_t data[9] = { 0xF0,   0x00,   0x21, 0x45, SYSTEM_CALL,
                        LOGGER, status, 0x00, 0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendPresetSlotChanged(MidiInterface::Type interface,
                                       uint8_t port)
{
    uint8_t data[7] = { 0xF0, 0x00,          0x21,
                        0x45, EVENT_MESSAGE, PRESET_LIST_CHANGE,
                        0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendSnapshotBankChanged(MidiInterface::Type interface,
                                         uint8_t port,
                                         uint8_t bankNumber)
{
    uint8_t data[8] = { 0xF0,       0x00,          0x21,
                        0x45,       EVENT_MESSAGE, SNAPSHOT_BANK_SWITCH,
                        bankNumber, 0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendSnapshotChanged(MidiInterface::Type interface,
                                     uint8_t port)
{
    uint8_t data[7] = { 0xF0, 0x00, 0x21, 0x45, EVENT_MESSAGE, SNAPSHOT_CHANGE,
                        0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendPresetSwitched(MidiInterface::Type interface,
                                    uint8_t port,
                                    uint8_t bankNumber,
                                    uint8_t slotId)
{
    uint8_t data[9] = { 0xF0,          0x00,       0x21,   0x45, EVENT_MESSAGE,
                        PRESET_SWITCH, bankNumber, slotId, 0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendPageSwitched(MidiInterface::Type interface,
                                  uint8_t port,
                                  uint8_t pageNumber)
{
    uint8_t data[8] = { 0xF0,          0x00,        0x21,       0x45,
                        EVENT_MESSAGE, PAGE_SWITCH, pageNumber, 0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendPresetBankSwitched(MidiInterface::Type interface,
                                        uint8_t port,
                                        uint8_t bankNumber)
{
    uint8_t data[8] = { 0xF0,       0x00,          0x21,
                        0x45,       EVENT_MESSAGE, PRESET_BANK_SWITCH,
                        bankNumber, 0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendControlSetSwitched(MidiInterface::Type interface,
                                        uint8_t port,
                                        uint8_t controlSetId)
{
    uint8_t data[8] = { 0xF0,         0x00,          0x21,
                        0x45,         EVENT_MESSAGE, CONTROL_SET_SWITCH,
                        controlSetId, 0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendPotTouchEvent(MidiInterface::Type interface,
                                   uint8_t port,
                                   uint8_t potId,
                                   uint16_t controlId,
                                   bool touched)
{
    uint8_t data[11] = { 0xF0,
                         0x00,
                         0x21,
                         0x45,
                         EVENT_MESSAGE,
                         POT_TOUCH,
                         potId,
                         (uint8_t)(controlId & 0x7f),
                         (uint8_t)((controlId >> 7) & 0x7f),
                         touched,
                         0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendUsbHostChanged(MidiInterface::Type interface, uint8_t port)
{
    uint8_t data[7] = { 0xF0, 0x00, 0x21, 0x45, EVENT_MESSAGE, USB_HOST_CHANGE,
                        0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

namespace
{
    /** The transaction the message being answered arrived with. */
    uint16_t currentTransactionId = 0;
} // namespace

void MidiOutput::setTransactionId(uint16_t newTransactionId)
{
    currentTransactionId = newTransactionId;
}

uint16_t MidiOutput::getTransactionId(void)
{
    return (currentTransactionId);
}

void MidiOutput::sendAck(MidiInterface::Type interface, uint8_t port)
{
    uint8_t data[9] = { 0xF0,
                        0x00,
                        0x21,
                        0x45,
                        EVENT_MESSAGE,
                        ACK,
                        (uint8_t)(currentTransactionId & 0x7F),
                        (uint8_t)((currentTransactionId >> 7) & 0x7F),
                        0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendNack(MidiInterface::Type interface, uint8_t port)
{
    uint8_t data[9] = { 0xF0,
                        0x00,
                        0x21,
                        0x45,
                        EVENT_MESSAGE,
                        NACK,
                        (uint8_t)(currentTransactionId & 0x7F),
                        (uint8_t)((currentTransactionId >> 7) & 0x7F),
                        0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendAvailable(MidiInterface::Type interface, uint8_t port)
{
    uint8_t data[9] = { 0xF0,      0x00, 0x21, 0x45, EVENT_MESSAGE,
                        AVAILABLE, 0x00, 0x00, 0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendStagedProgress(MidiInterface::Type interface,
                                    uint8_t port,
                                    uint32_t bytes)
{
    uint8_t data[11] = { 0xF0,
                         0x00,
                         0x21,
                         0x45,
                         EVENT_MESSAGE,
                         STAGED_PROGRESS,
                         (uint8_t)(bytes & 0x7f),
                         (uint8_t)((bytes >> 7) & 0x7f),
                         (uint8_t)((bytes >> 14) & 0x7f),
                         (uint8_t)((bytes >> 21) & 0x7f),
                         0xF7 };

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, sizeof(data));
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendUsbHostDevices(MidiInterface::Type interface, uint8_t port)
{
    uint8_t data[512];
    StaticJsonDocument<512> doc;

    data[0] = 0xF0;
    data[1] = 0x00;
    data[2] = 0x21;
    data[3] = 0x45;
    data[4] = FILE_UPLOAD;
    data[5] = USB_HOST_DEVICES;

    char *jsonStart = (char *)&data[6];

    doc["version"] = 1;
    JsonArray devices = doc.createNestedArray("devices");

    for (uint8_t i = 0; i < NR_OF_MIDI_INTERFACES; i++) {
        const USBDevice &device = USBDevices[i];

        /* An empty slot has no driver name, which is what reset() leaves
         * behind. A socket with nothing in it is left out of the list rather
         * than reported as a nameless device. */
        if (device.driver[0] == '\0') {
            continue;
        }

        JsonObject entry = devices.createNestedObject();
        entry["manufacturer"] = device.manufacturer;
        entry["product"] = device.product;
        entry["serialNumber"] = device.serial;
        entry["vid"] = device.manufacturerId;
        entry["pid"] = device.productId;

        JsonArray ports = entry.createNestedArray("ports");
        JsonObject devicePort = ports.createNestedObject();
        devicePort["devicePort"] = 1;
        devicePort["name"] = device.driver;
        devicePort["electraPort"] = (device.midiBus == 0) ? "port1" : "port2";
    }

    serializeJson(doc, jsonStart, sizeof(data) - 7);
    size_t jsonSize = strlen(jsonStart);
    data[jsonSize + 6] = SYSEX_END;

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, jsonSize + 7);
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendAppInfo(MidiInterface::Type interface, uint8_t port)
{
    uint8_t data[256];
    StaticJsonDocument<256> doc;

    data[0] = 0xF0;
    data[1] = 0x00;
    data[2] = 0x21;
    data[3] = 0x45;
    data[4] = FILE_UPLOAD;
    data[5] = APP_INFO;

    char *jsonStart = (char *)&data[6];
    doc["app"] = App::get()->getApplicationName();
    if (System::context.getPresetName()) {
        doc["preset"] = System::context.getPresetName();
    }
    serializeJson(doc, jsonStart, 250);
    size_t jsonSize = strlen(jsonStart);
    data[jsonSize + 6] = SYSEX_END;

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, jsonSize + 7);
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendRuntimeInfo(MidiInterface::Type interface, uint8_t port)
{
    uint8_t data[128];
    StaticJsonDocument<128> doc;

    data[0] = 0xF0;
    data[1] = 0x00;
    data[2] = 0x21;
    data[3] = 0x45;
    data[4] = FILE_UPLOAD;
    data[5] = MEMORY_INFO;

    char *jsonStart = (char *)&data[6];
    doc["freePercentage"] = Hardware::ram.getFreeRam();
    serializeJson(doc, jsonStart, 120);
    size_t jsonSize = strlen(jsonStart);
    data[jsonSize + 6] = SYSEX_END;

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, jsonSize + 7);
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendElectraInfo(MidiInterface::Type interface,
                                 uint8_t port,
                                 const char *electraInfoSerial,
                                 uint8_t electraInfoHwRevision)
{
    uint8_t data[128];
    char buf[16];
    StaticJsonDocument<128> doc;

    sprintf(buf,
            "%d.%02d",
            electraInfoHwRevision / 100,
            electraInfoHwRevision - 200);

    data[0] = 0xF0;
    data[1] = 0x00;
    data[2] = 0x21;
    data[3] = 0x45;
    data[4] = FILE_UPLOAD;
    data[5] = ELECTRA_INFO;

    char *jsonStart = (char *)&data[6];
    doc["versionText"] = FIRMWARE_VERSION;
    doc["versionSeq"] = FIRMWARE_SEQ;
    doc["serial"] = electraInfoSerial;
    doc["hwRevision"] = buf;
    serializeJson(doc, jsonStart, 120);
    size_t jsonSize = strlen(jsonStart);
    data[jsonSize + 6] = SYSEX_END;

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, jsonSize + 7);
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

void MidiOutput::sendMidiLearn(MidiInterface::Type interface,
                               uint8_t portToTransmitOn,
                               const char *msg,
                               uint8_t port,
                               uint8_t channel,
                               uint16_t parameterId,
                               uint16_t value)
{
    uint8_t data[512];
    StaticJsonDocument<512> doc;

#ifdef DEBUG
    System::logger.write(
        ERROR,
        "sendMidiLearn: msg=%s, port=%d, channel=%d, parameter=%d, value=%d",
        msg,
        port,
        channel,
        parameterId,
        value);
#endif

    data[0] = 0xF0;
    data[1] = 0x00;
    data[2] = 0x21;
    data[3] = 0x45;
    data[4] = MIDILEARN_SWITCH;

    char *jsonStart = (char *)&data[5];

    doc["msg"] = msg;
    doc["port"] = port;
    doc["channel"] = channel;
    doc["parameterId"] = parameterId;
    doc["value"] = value;
    serializeJson(doc, jsonStart, 504);
    size_t jsonSize = strlen(jsonStart);
    data[jsonSize + 5] = SYSEX_END;

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, jsonSize + 6);
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, portToTransmitOn, message);
}

void MidiOutput::sendMidiLearnSysex(MidiInterface::Type interface,
                                    uint8_t portToTransmitOn,
                                    uint8_t port,
                                    const SysexBlock &capturedSysexBlock)
{
    size_t sysExLength = capturedSysexBlock.getLength();
    char writeBuffer[64];

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());

    writeBuffer[0] = 0xF0;
    writeBuffer[1] = 0x00;
    writeBuffer[2] = 0x21;
    writeBuffer[3] = 0x45;
    writeBuffer[4] = MIDILEARN_SWITCH;
    sysexBlock.writeBytes((const uint8_t *)writeBuffer, 5);

    snprintf(writeBuffer,
             sizeof(writeBuffer),
             "{\"msg\":\"sysex\", \"port\":%d,\"data\":[",
             port);
    sysexBlock.writeBytes((const uint8_t *)writeBuffer, strlen(writeBuffer));

    for (size_t i = 0; i < sysExLength; i++) {
        snprintf(writeBuffer,
                 sizeof(writeBuffer),
                 "\"%02X\"%c",
                 capturedSysexBlock.peek(i),
                 (i == (sysExLength - 1)) ? ' ' : ',');
        sysexBlock.writeBytes((const uint8_t *)writeBuffer,
                              strlen(writeBuffer));
    }

    writeBuffer[0] = ']';
    writeBuffer[1] = '}';
    writeBuffer[2] = 0xF7;

    sysexBlock.writeBytes((const uint8_t *)writeBuffer, 3);
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, portToTransmitOn, message);
}

extern uint8_t stopFlush;

bool MidiOutput::sendSysExFile(uint8_t port,
                               const char *filename,
                               ElectraCommand::Object fileType)
{
    FileIoStream file;
    uint32_t i = 0;
    uint32_t packet = 0;
    uint8_t byteInPacket = 0;
    uint8_t data[4];

    if (!(file = Hardware::sdcard.createInputStream(filename))) {
        System::logger.write(
            LOG_ERROR,
            "sendSysexFile: the file does not exists: filename=%s",
            filename);
        return (false);
    }

    stopFlush = 1;
    MidiOutput::sendSysExHeader(port);

    data[0] = FILE_UPLOAD;
    data[1] = (uint8_t)fileType;
    packet = 2;

    while (file.available()) {
        byteInPacket = packet % 4;
        data[byteInPacket] = file.read();

        if (byteInPacket == 3) {
            MidiOutput::sendSysExData(data, 4, port);
        }

        i++;
        packet++;

        if ((i % 20) == 0) {
            delayMicroseconds(50);
            App::get()->yield();
        }
    }

    file.close();

    /*
     * send last packet correctly
     */
    byteInPacket = packet % 4;
    data[byteInPacket] = SYSEX_END;
    MidiOutput::sendSysExData(data, byteInPacket + 1, port);

    stopFlush = 0;

    return (true);
}

bool MidiOutput::sendSysExFileBase64(uint8_t port,
                                     const char *filename,
                                     ElectraCommand::Object fileType)
{
    /* Three bytes in, four out: the encoder works in threes, so the file is
     * read in threes and nothing has to be carried between chunks. 240 is
     * what the sysex block streams in elsewhere. */
    static constexpr uint16_t chunkBytes = 240;
    static uint8_t chunk[chunkBytes];
    /* Four characters for every three bytes, and chunkBytes is a
     * multiple of three so there is never a partial group to pad. */
    static char encoded[((chunkBytes / 3) * 4) + 1];
    static_assert((chunkBytes % 3) == 0,
                  "a chunk that is not a multiple of three would be "
                  "padded in the middle of the file");

    FileIoStream file;

    if (!(file = Hardware::sdcard.createInputStream(filename))) {
        System::logger.write(
            LOG_ERROR,
            "sendSysExFileBase64: the file does not exist: filename=%s",
            filename);
        return (false);
    }

    stopFlush = 1;
    MidiOutput::sendSysExHeader(port);

    uint8_t header[2] = { FILE_UPLOAD, (uint8_t)fileType };

    MidiOutput::sendSysExData(header, 2, port);

    uint32_t sent = 0;

    while (file.available()) {
        int read = file.read(chunk, chunkBytes);

        if (read <= 0) {
            break;
        }

        int length = base64_encode(encoded, (char *)chunk, read);

        MidiOutput::sendSysExData((const uint8_t *)encoded, length, port);

        /* The same breather sendSysExFile takes. The host reads as fast as
         * the link delivers, but the device's own USB buffers do not. */
        sent += read;

        if ((sent % (chunkBytes * 20)) == 0) {
            delayMicroseconds(50);
            App::get()->yield();
        }
    }

    file.close();

    uint8_t end = SYSEX_END;

    MidiOutput::sendSysExData(&end, 1, port);

    stopFlush = 0;

    System::logger.write(LOG_INFO,
                         "sendSysExFileBase64: sent: filename=%s, bytes=%d",
                         filename,
                         (int)sent);

    return (true);
}

void MidiOutput::sendFileListing(MidiInterface::Type interface,
                                 uint8_t port,
                                 const JsonDocument &listing)
{
    /* Below the SRAM bank boundary at 0x20000000, where a copy spanning
     * SRAM_L and SRAM_U hard faults. See the linker script's .bss.lowram. */
    __attribute__((section(".bss.lowram"))) static uint8_t data[2304];

    data[0] = 0xF0;
    data[1] = 0x00;
    data[2] = 0x21;
    data[3] = 0x45;
    data[4] = FILE_UPLOAD;
    data[5] = LOCATION;

    char *jsonStart = (char *)&data[6];
    serializeJson(listing, jsonStart, sizeof(data) - 8);
    size_t jsonSize = strlen(jsonStart);
    data[jsonSize + 6] = SYSEX_END;

    SysexBlock sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
    sysexBlock.writeBytes(data, jsonSize + 7);
    sysexBlock.close();

    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
}

/*
 * send USB MIDI packets
 */
void MidiOutput::sendSysExData(const uint8_t *data,
                               uint32_t length,
                               uint8_t port)
{
    uint8_t cable = (port & 0x0F) << 4;

    while (length > 3) {
        usb_midi_write_packed(0x04 | cable | (data[0] << 8) | (data[1] << 16)
                              | (data[2] << 24));
        data += 3;
        length -= 3;
    }
    if (length == 3) {
        usb_midi_write_packed(0x07 | cable | (data[0] << 8) | (data[1] << 16)
                              | (data[2] << 24));
    } else if (length == 2) {
        usb_midi_write_packed(0x06 | cable | (data[0] << 8) | (data[1] << 16));
    } else if (length == 1) {
        usb_midi_write_packed(0x05 | cable | (data[0] << 8));
    }
}

void MidiOutput::sendSysExHeader(uint8_t port)
{
    uint8_t data[4];
    uint32_t manufacturerSysexId = SYSEX_MANUFACTURER;

    data[0] = SYSEX_START;
    data[1] = (manufacturerSysexId >> 16) & 0xFF;
    data[2] = (manufacturerSysexId >> 8) & 0xFF;
    data[3] = manufacturerSysexId & 0xFF;

    MidiOutput::sendSysExData(data, 4, port);
}

void MidiOutput::setPort(uint8_t newPort)
{
    port = newPort;
}

uint8_t MidiOutput::getPort(void) const
{
    return (port);
}

void MidiOutput::setChannel(uint8_t newChannel)
{
    channel = newChannel;
}

uint8_t MidiOutput::getChannel(void) const
{
    return (channel);
}

void MidiOutput::setRate(uint16_t newRate)
{
    rate = newRate;
}

uint16_t MidiOutput::getRate(void) const
{
    return (rate);
}

void MidiOutput::setTsLastMessage(void) const
{
    tsLastMessage = millis();
}

bool MidiOutput::isReady(void)
{
    return (((millis() - tsLastMessage) > rate) ? true : false);
}

/** Send Electra message to Midi outputs.
 *
 */
bool MidiOutput::canSend(MidiInterface::Type interface,
                         uint8_t port,
                         size_t bytes)
{
    return (MidiInterface::get(interface)->canSend(port, bytes));
}

void MidiOutput::sendMessage(MidiInterface::Type interface,
                             uint8_t port,
                             MidiMessage &message)
{
    addToQueue(interface, port, message);
}

bool MidiOutput::isIdenticalChange(const MidiMessageTransport &m1,
                                   const MidiMessageTransport &m2)
{
    if (m1.getType() == MidiMessage::Type::ControlChange) {
        // This is to prevent issue of the rpn reset
        if ((m1.getInterfaceType() == m2.getInterfaceType())
            && (m1.getPort() == m2.getPort()) && (m1.getType() == m2.getType())
            && (m1.getChannel() == m2.getChannel())
            && ((m1.getData1() == 100) || (m1.getData1() == 101))) {
            return false;
        } else if ((m1.getInterfaceType() == m2.getInterfaceType())
                   && (m1.getPort() == m2.getPort())
                   && (m1.getType() == m2.getType())
                   && (m1.getChannel() == m2.getChannel())
                   && (m1.getData1() == m2.getData1())) {
            return true;
        }
    } else if (m1.getType() == MidiMessage::Type::ProgramChange) {
        if ((m1.getInterfaceType() == m2.getInterfaceType())
            && (m1.getPort() == m2.getPort()) && (m1.getType() == m2.getType())
            && (m1.getChannel() == m2.getChannel())) {
            return true;
        }
    } else if (m1.getType() == MidiMessage::Type::SystemExclusive) {
        if ((m1.getInterfaceType() == m2.getInterfaceType())
            && (m1.getPort() == m2.getPort())
            && (m1.getType() == m2.getType())) {
            return true;
        }
    }
    return (false);
}

/** Adds the message to the queue if it is not registered there already
 *
 */
void MidiOutput::addToQueue(MidiInterface::Type interface,
                            uint8_t port,
                            MidiMessage &message)
{
    TRACE_MIDI_OUT(MidiInterface::getName(interface),
                   port,
                   "queue",
                   message.getChannel(),
                   message.getData1(),
                   message.getData2());
        MidiMessageTransport mmt(interface, port, message);

    /* A full queue used to overwrite its oldest entry, so a script sending
     * a patch of more than sixty-four values in one go silently lost the
     * first of them. Draining a tick's worth here instead costs the caller
     * the time of ten sends, which is the price of asking. */
    if (outgoingQueue.isFull()) {
        System::tasks.flushMidi();
    }

    /*
    for (int i = 0; i < outgoingQueue.size(); i++) {
        if (MidiOutput::isIdenticalChange(outgoingQueue[i], mmt)) {
            outgoingQueue.getObjectPointer(i)->invalid = true;
        }
    }
*/
    outgoingQueue.push(mmt);
}

/** Send Electra message to Midi outputs.
 *
 */
void MidiOutput::send(MidiInterface::Type interface,
                      uint8_t port,
                      MidiMessage::Type type,
                      uint8_t channel,
                      uint8_t data1,
                      uint8_t data2)
{
    TRACE_MIDI_OUT(
        MidiInterface::getName(interface), port, "send", channel, data1, data2);
    PROFILE_BEGIN(UsbSend);
    MidiInterface::get(interface)->send(port, type, channel, data1, data2);
    PROFILE_END(UsbSend);
    indicate(interface, port, Direction::out, type);
}

void MidiOutput::send(MidiInterface::Type interface,
                      uint8_t port,
                      MidiMessage &message)
{
    TRACE_MIDI_OUT(MidiInterface::getName(interface),
                   port,
                   "sendMessage",
                   message.getChannel(),
                   message.getData1(),
                   message.getData2());

    if (message.getType() == MidiMessage::Type::SystemExclusive) {
        PROFILE_BEGIN(SysexSend);
        MidiInterface::get(interface)->send(port, message.getSysExBlock());
        PROFILE_END(SysexSend);
    } else {
        MidiInterface::get(interface)->send(port,
                                            message.getType(),
                                            message.getChannel(),
                                            message.getData1(),
                                            message.getData2());
    }
    indicate(interface, port, Direction::out, message.getType());
}

void MidiOutput::send(MidiInterface::Type interface,
                      uint8_t port,
                      SysexBlock &sysexBlock)
{
    TRACE_MIDI_OUT(
        MidiInterface::getName(interface), port, "sendSysExBlock", 0, 0, 0);
    MidiInterface::get(interface)->sendSysEx(port, sysexBlock);
    indicate(
        interface, port, Direction::out, MidiMessage::Type::SystemExclusive);
}

void MidiOutput::sendControlChange(MidiInterface::Type interface,
                                   uint8_t port,
                                   uint8_t channel,
                                   uint8_t parameterNumber,
                                   uint8_t value)
{
    MidiMessage message(
        channel, MidiMessage::Type::ControlChange, parameterNumber, value);
    sendMessage(interface, port, message);
    indicate(interface, port, Direction::out, MidiMessage::Type::ControlChange);
}

void MidiOutput::sendNoteOn(MidiInterface::Type interface,
                            uint8_t port,
                            uint8_t channel,
                            uint8_t noteNumber,
                            uint8_t velocity)
{
    MidiInterface::get(interface)->sendNoteOn(
        port, channel, noteNumber, velocity);
    indicate(interface, port, Direction::out, MidiMessage::Type::NoteOn);
}

void MidiOutput::sendNoteOff(MidiInterface::Type interface,
                             uint8_t port,
                             uint8_t channel,
                             uint8_t noteNumber,
                             uint8_t velocity)
{
    MidiInterface::get(interface)->sendNoteOff(
        port, channel, noteNumber, velocity);
    indicate(interface, port, Direction::out, MidiMessage::Type::NoteOff);
}

void MidiOutput::sendStart(MidiInterface::Type interface, uint8_t port)
{
    MidiInterface::get(interface)->sendStart(port);
    indicate(interface, port, Direction::out, MidiMessage::Type::Start);
}

void MidiOutput::sendStop(MidiInterface::Type interface, uint8_t port)
{
    MidiInterface::get(interface)->sendStop(port);
    indicate(interface, port, Direction::out, MidiMessage::Type::Stop);
}

void MidiOutput::sendTuneRequest(MidiInterface::Type interface, uint8_t port)
{
    MidiInterface::get(interface)->sendTuneRequest(port);
    indicate(interface, port, Direction::out, MidiMessage::Type::TuneRequest);
}

void MidiOutput::sendProgramChange(MidiInterface::Type interface,
                                   uint8_t port,
                                   uint8_t channel,
                                   uint8_t programNumber)
{
    MidiMessage message(
        channel, MidiMessage::Type::ProgramChange, programNumber, 0);
    sendMessage(interface, port, message);
    indicate(interface, port, Direction::out, MidiMessage::Type::ProgramChange);
}

void MidiOutput::sendSysEx(MidiInterface::Type interface,
                           uint8_t port,
                           SysexBlock &sysexBlock)
{
    MidiMessage message(sysexBlock);
    sendMessage(interface, port, message);
    indicate(
        interface, port, Direction::out, MidiMessage::Type::SystemExclusive);
}

void MidiOutput::sendSysEx(MidiInterface::Type interface,
                           uint8_t port,
                           uint8_t *data,
                           uint16_t dataLength)
{
    MidiInterface::get(interface)->sendSysEx(port, data, dataLength);
    indicate(
        interface, port, Direction::out, MidiMessage::Type::SystemExclusive);
}

void MidiOutput::sendSysExPartial(MidiInterface::Type interface,
                                  uint8_t port,
                                  const uint8_t *data,
                                  uint16_t dataLength,
                                  bool complete)
{
    MidiInterface::get(interface)->sendSysExPartial(
        port, data, dataLength, complete);
    indicate(
        interface, port, Direction::out, MidiMessage::Type::SystemExclusive);
}

void MidiOutput::sendPitchBend(MidiInterface::Type interface,
                               uint8_t port,
                               uint8_t channel,
                               uint16_t value)
{
    MidiInterface::get(interface)->sendPitchBend(port, channel, value);
    indicate(interface, port, Direction::out, MidiMessage::Type::PitchBend);
}

void MidiOutput::sendAfterTouchPoly(MidiInterface::Type interface,
                                    uint8_t port,
                                    uint8_t channel,
                                    uint8_t noteNumber,
                                    uint8_t pressure)
{
    MidiInterface::get(interface)->sendAfterTouchPoly(
        port, channel, noteNumber, pressure);
    indicate(
        interface, port, Direction::out, MidiMessage::Type::AfterTouchPoly);
}

void MidiOutput::sendAfterTouchChannel(MidiInterface::Type interface,
                                       uint8_t port,
                                       uint8_t channel,
                                       uint8_t pressure)
{
    MidiInterface::get(interface)->sendAfterTouchChannel(
        port, channel, pressure);
    indicate(
        interface, port, Direction::out, MidiMessage::Type::AfterTouchChannel);
}

void MidiOutput::sendClock(MidiInterface::Type interface, uint8_t port)
{
    MidiInterface::get(interface)->sendClock(port);
    indicate(interface, port, Direction::out, MidiMessage::Type::Clock);
}

void MidiOutput::sendContinue(MidiInterface::Type interface, uint8_t port)
{
    MidiInterface::get(interface)->sendContinue(port);
    indicate(interface, port, Direction::out, MidiMessage::Type::Continue);
}

void MidiOutput::sendActiveSensing(MidiInterface::Type interface, uint8_t port)
{
    MidiInterface::get(interface)->sendActiveSensing(port);
    indicate(interface, port, Direction::out, MidiMessage::Type::ActiveSensing);
}

void MidiOutput::sendSystemReset(MidiInterface::Type interface, uint8_t port)
{
    MidiInterface::get(interface)->sendSystemReset(port);
    indicate(interface, port, Direction::out, MidiMessage::Type::SystemReset);
}

void MidiOutput::sendSongSelect(MidiInterface::Type interface,
                                uint8_t port,
                                uint8_t song)
{
    MidiInterface::get(interface)->sendSongSelect(port, song);
    indicate(interface, port, Direction::out, MidiMessage::Type::SongSelect);
}

void MidiOutput::sendSongPosition(MidiInterface::Type interface,
                                  uint8_t port,
                                  uint16_t beats)
{
    MidiInterface::get(interface)->sendSongPosition(port, beats);
    indicate(interface, port, Direction::out, MidiMessage::Type::SongPosition);
}

void MidiOutput::sendSysEx(MidiInterface::Type interface,
                           uint8_t port,
                           std::vector<uint8_t> data)
{
    sendSysEx(interface, port, &data[0], data.size());
    indicate(
        interface, port, Direction::out, MidiMessage::Type::SystemExclusive);
}

void MidiOutput::sendNrpn(MidiInterface::Type interface,
                          uint8_t port,
                          uint8_t channel,
                          uint16_t parameterNumber,
                          uint16_t midiValue,
                          bool lsbFirst,
                          bool resetRpn)
{
    sendControlChange(interface, port, channel, 99, parameterNumber >> 7);
    sendControlChange(interface, port, channel, 98, parameterNumber & 0x7F);

    if (lsbFirst) {
        sendControlChange(interface, port, channel, 38, midiValue >> 7);
        sendControlChange(interface, port, channel, 6, midiValue & 0x7F);
    } else {
        sendControlChange(interface, port, channel, 6, midiValue >> 7);
        sendControlChange(interface, port, channel, 38, midiValue & 0x7F);
    }

    if (resetRpn) {
        sendControlChange(interface, port, channel, 101, 127);
        sendControlChange(interface, port, channel, 100, 127);
    }
}

void MidiOutput::sendRpn(MidiInterface::Type interface,
                         uint8_t port,
                         uint8_t channel,
                         uint16_t parameterNumber,
                         uint16_t midiValue)
{
    System::logger.write(LOG_ERROR, "sending RPN: %d", parameterNumber);
    sendControlChange(interface, port, channel, 101, parameterNumber >> 7);
    sendControlChange(interface, port, channel, 100, parameterNumber & 0x7F);
    sendControlChange(interface, port, channel, 6, midiValue >> 7);
    sendControlChange(interface, port, channel, 38, midiValue & 0x7F);
    sendControlChange(interface, port, channel, 101, 127);
    sendControlChange(interface, port, channel, 100, 127);
}

void MidiOutput::sendControlChange14Bit(MidiInterface::Type interface,
                                        uint8_t port,
                                        uint8_t channel,
                                        uint16_t parameterNumber,
                                        uint16_t midiValue,
                                        bool lsbFirst)
{
    if (lsbFirst) {
        sendControlChange(
            interface, port, channel, parameterNumber + 32, midiValue >> 7);
        sendControlChange(
            interface, port, channel, parameterNumber, midiValue & 0x7F);
    } else {
        sendControlChange(
            interface, port, channel, parameterNumber, midiValue >> 7);
        sendControlChange(
            interface, port, channel, parameterNumber + 32, midiValue & 0x7F);
    }
}

void MidiOutput::enableThru(MidiInterface::Type interface,
                            uint8_t port,
                            bool shouldBeEnabled)
{
    if (interface == MidiInterface::Type::MidiIo) {
        MidiInterface::get(interface)->enableThru(port, shouldBeEnabled);
    }
    // other interfaces are not implmented on purpose
}

void MidiOutput::indicate(MidiInterface::Type interface,
                          uint8_t port,
                          Direction direction,
                          MidiMessage::Type msgType)
{
    App::get()->statusBar.indicate(interface, port, direction, msgType);
}

/* Below the SRAM bank boundary at 0x20000000, as the incoming queue is: a
 * message copied across SRAM_L and SRAM_U hard faults. */
__attribute__((section(".bss.lowram")))
CircularBuffer<MidiMessageTransport, MidiOutput::queueSize>
    MidiOutput::outgoingQueue;
