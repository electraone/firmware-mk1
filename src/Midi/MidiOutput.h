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

/**
 * @file MidiOutput.cpp
 *
 * @brief An implementation of outgoing MIDI data.
 */

#pragma once

#include "ArduinoJson.h"

#include "MidiMessage.h"
#include "MidiMessageTransport.h"
#define CIRCULAR_BUFFER_INT_SAFE
#include "CircularBuffer.h"

#define SYSEX_START 0xF0
#define SYSEX_END 0xF7
#define SYSEX_MANUFACTURER 0x002145

/*
 * Commands
 */
#define FILE_UPLOAD 0x01
#define FILE_DOWNLOAD 0x02
#define MIDILEARN_SWITCH 0x03
#define UPDATE 0x04
#define REMOVE 0x05
#define SWAP 0x06
#define CREATE 0x07
#define EXECUTE 0x08
#define UPDATE_RUNTIME 0x14
#define DATA_CONTAINER 0x7D
#define EVENT_MESSAGE 0x7E
#define SYSTEM_CALL 0x7F

/*
 * Events
 */
#define NACK 0x00
#define ACK 0x01
#define PRESET_SWITCH 0x02
#define SNAPSHOT_CHANGE 0x03
#define SNAPSHOT_BANK_SWITCH 0x04
#define PRESET_LIST_CHANGE 0x05
#define PAGE_SWITCH 0x06
#define CONTROL_SET_SWITCH 0x07
#define PRESET_BANK_SWITCH 0x08
#define USB_HOST_CHANGE 0x09
#define POT_TOUCH 0x0A
#define STAGED_PROGRESS 0x2D
#define AVAILABLE 0x7F

/*
 * Object types
 */
#define FILE_PRESET_LEGACY 0x00
#define FILE_PRESET 0x01
#define FILE_CONFIG 0x02
#define FILE_SNAPSHOT 0x03
#define PRESET_LIST 0x04
#define SNAPSHOT_LIST 0x05
#define SNAPSHOT_INFO 0x06
#define CONTROL 0x07
#define FILE_LUA_SCRIPT 0x0C
#define FUNCTION 0x0D
#define USB_HOST_DEVICES 0x10
#define FILE_UITOOLKIT 0x70
#define LOCATION 0x34
#define FILE_DESCRIPTOR 0x35
#define CTRL_PORT 0x7B
#define APP_INFO 0x7C
#define LOGGER 0x7D
#define MEMORY_INFO 0x7E
#define ELECTRA_INFO 0x7F

#define USB_MIDI_PORT_CTRL 2

class MidiOutput : public MidiJack
{
public:
    MidiOutput(MidiInterface::Type newInterface, uint8_t newPort);
    MidiOutput(MidiInterface::Type newInterface,
               uint8_t newPort,
               uint8_t newChannel,
               uint16_t newRate);
    virtual ~MidiOutput() = default;

    /*
     * API commands
     */
    static void
        sendSetLogger(MidiInterface::Type interface, uint8_t port, bool status);
    static void sendPresetSlotChanged(MidiInterface::Type interface,
                                      uint8_t port);
    static void sendSnapshotBankChanged(MidiInterface::Type interface,
                                        uint8_t port,
                                        uint8_t bankNumber);
    static void sendSnapshotChanged(MidiInterface::Type interface,
                                    uint8_t port);
    static void sendPresetSwitched(MidiInterface::Type interface,
                                   uint8_t port,
                                   uint8_t bankNumber,
                                   uint8_t slotId);
    static void sendPageSwitched(MidiInterface::Type interface,
                                 uint8_t port,
                                 uint8_t pageNumber);
    static void sendPresetBankSwitched(MidiInterface::Type interface,
                                       uint8_t port,
                                       uint8_t bankNumber);
    static void sendControlSetSwitched(MidiInterface::Type interface,
                                       uint8_t port,
                                       uint8_t controlSetId);
    static void sendPotTouchEvent(MidiInterface::Type interface,
                                  uint8_t port,
                                  uint8_t potId,
                                  uint16_t controlId,
                                  bool touched);
    static void sendUsbHostChanged(MidiInterface::Type interface, uint8_t port);

    /**
     * @brief What is plugged into the USB host sockets, as the API spells it.
     *
     * One device per socket, each with the one port it carries. The shape is
     * the published one - a device list, and a port list inside each device -
     * even though this model only ever has one port per device, so that a host
     * reads it with the same code on either.
     */
    static void sendUsbHostDevices(MidiInterface::Type interface, uint8_t port);

    /**
     * @brief How much of a staged upload has arrived, in bytes.
     *
     * Sent as each chunk is taken, so a host can draw a progress bar over a
     * transfer that takes tens of seconds. Twenty-eight bits, seven to a byte,
     * low byte first - the same shape the size is registered in.
     */
    static void sendStagedProgress(MidiInterface::Type interface,
                                   uint8_t port,
                                   uint32_t bytes);
    static void sendAck(MidiInterface::Type interface, uint8_t port);
    static void sendNack(MidiInterface::Type interface, uint8_t port);
    static void sendAvailable(MidiInterface::Type interface, uint8_t port);
    static void sendAppInfo(MidiInterface::Type interface, uint8_t port);
    static void sendRuntimeInfo(MidiInterface::Type interface, uint8_t port);
    static void sendElectraInfo(MidiInterface::Type interface,
                                uint8_t port,
                                const char *electraInfoSerial,
                                uint8_t electraInfoHwRevision);
    static void sendMidiLearn(MidiInterface::Type interface,
                              uint8_t portToTransmitOn,
                              const char *msg,
                              uint8_t port,
                              uint8_t channel,
                              uint16_t parameterId,
                              uint16_t value);
    static void sendMidiLearnSysex(MidiInterface::Type interface,
                                   uint8_t portToTransmitOn,
                                   uint8_t port,
                                   const SysexBlock &sysexBlock);

    static bool sendSysExFile(uint8_t port,
                              const char *filename,
                              ElectraCommand::Object fileType);

    /**
     * @brief The same, base64 encoded on the way out.
     *
     * SysEx carries seven bit bytes, so anything that is not text has to be
     * encoded: the first byte above 0x7F in a picture would be read as a
     * status byte and end the message where it stands.
     */
    static bool sendSysExFileBase64(uint8_t port,
                                    const char *filename,
                                    ElectraCommand::Object fileType);

    /** A directory listing, as the file transfer API spells one. */
    static void sendFileListing(MidiInterface::Type interface,
                                uint8_t port,
                                const JsonDocument &listing);

    /**
     * @name The transaction a reply belongs to
     *
     * A host may put a 14 bit transaction id in front of a request, and it
     * matches the reply to the request by that id rather than by arrival
     * order. An acknowledgement that always says nought is one the editor
     * cannot match, so its queue fills with messages it thinks are still in
     * flight - which is what it did.
     *
     * Held here rather than passed down because every acknowledgement in the
     * firmware would otherwise need the id threaded to it through code that
     * has no other reason to know about transactions. One message is parsed
     * at a time, so there is one current transaction.
     * @{
     */
    static void setTransactionId(uint16_t newTransactionId);
    static uint16_t getTransactionId(void);
    /** @} */

    static void
        sendSysExData(const uint8_t *data, uint32_t length, uint8_t port);
    static void sendSysExHeader(uint8_t port);
    // END of API commands

    void setPort(uint8_t newPort);
    uint8_t getPort(void) const;
    void setChannel(uint8_t newChannel);
    uint8_t getChannel(void) const;
    void setRate(uint16_t newRate);
    uint16_t getRate(void) const;
    void setTsLastMessage(void);
    bool isReady(void);

    // General MIDI messaging (outgoing)
    static void sendMessage(MidiInterface::Type interface,
                            uint8_t port,
                            MidiMessage &message);

    static bool isIdenticalChange(const MidiMessageTransport &m1,
                                  const MidiMessageTransport &m2);

    static void addToQueue(MidiInterface::Type interface,
                           uint8_t port,
                           MidiMessage &message);

    static void send(MidiInterface::Type interface,
                     uint8_t port,
                     MidiMessage::Type type,
                     uint8_t channel,
                     uint8_t data1,
                     uint8_t data2);
    static void
        send(MidiInterface::Type interface, uint8_t port, MidiMessage &message);
    static void send(MidiInterface::Type interface,
                     uint8_t port,
                     SysexBlock &sysexBlock);
    static void sendControlChange(MidiInterface::Type interface,
                                  uint8_t port,
                                  uint8_t channel,
                                  uint8_t parameterNumber,
                                  uint8_t value);
    static void sendNoteOn(MidiInterface::Type interface,
                           uint8_t port,
                           uint8_t channel,
                           uint8_t noteNumber,
                           uint8_t velocity);
    static void sendNoteOff(MidiInterface::Type interface,
                            uint8_t port,
                            uint8_t channel,
                            uint8_t noteNumber,
                            uint8_t velocity);
    static void sendStart(MidiInterface::Type interface, uint8_t port);
    static void sendStop(MidiInterface::Type interface, uint8_t port);
    static void sendTuneRequest(MidiInterface::Type interface, uint8_t port);
    static void sendProgramChange(MidiInterface::Type interface,
                                  uint8_t port,
                                  uint8_t channel,
                                  uint8_t programNumber);
    static void sendSysEx(MidiInterface::Type interface,
                          uint8_t port,
                          SysexBlock &sysexBlock);
    static void sendSysEx(MidiInterface::Type interface,
                          uint8_t port,
                          uint8_t *data,
                          uint16_t dataLength);
    static void sendSysExPartial(MidiInterface::Type interface,
                                 uint8_t port,
                                 const uint8_t *data,
                                 uint16_t dataLength,
                                 bool complete);
    static void sendPitchBend(MidiInterface::Type interface,
                              uint8_t port,
                              uint8_t channel,
                              uint16_t value);
    static void sendAfterTouchPoly(MidiInterface::Type interface,
                                   uint8_t port,
                                   uint8_t channel,
                                   uint8_t noteNumber,
                                   uint8_t pressure);
    static void sendAfterTouchChannel(MidiInterface::Type interface,
                                      uint8_t port,
                                      uint8_t channel,
                                      uint8_t pressure);
    static void sendClock(MidiInterface::Type interface, uint8_t port);
    static void sendContinue(MidiInterface::Type interface, uint8_t port);
    static void sendActiveSensing(MidiInterface::Type interface, uint8_t port);
    static void sendSystemReset(MidiInterface::Type interface, uint8_t port);
    static void sendSongSelect(MidiInterface::Type interface,
                               uint8_t port,
                               uint8_t song);
    static void sendSongPosition(MidiInterface::Type interface,
                                 uint8_t port,
                                 uint16_t beats);
    static void sendSysEx(MidiInterface::Type interface,
                          uint8_t port,
                          std::vector<uint8_t> data);
    static void sendNrpn(MidiInterface::Type interface,
                         uint8_t port,
                         uint8_t channel,
                         uint16_t parameterNumber,
                         uint16_t midiValue,
                         bool lsbFirst,
                         bool resetRpn);
    static void sendRpn(MidiInterface::Type interface,
                        uint8_t port,
                        uint8_t channel,
                        uint16_t parameterNumber,
                        uint16_t midiValue);
    static void sendControlChange14Bit(MidiInterface::Type interface,
                                       uint8_t port,
                                       uint8_t channel,
                                       uint16_t parameterNumber,
                                       uint16_t midiValue,
                                       bool lsbFirst);

    static void enableThru(MidiInterface::Type interface,
                           uint8_t port,
                           bool shouldBeEnabled);

    static constexpr uint16_t queueSize = 64;
    static CircularBuffer<MidiMessageTransport, queueSize> outgoingQueue;

private:
    static void indicate(MidiInterface::Type interface,
                         uint8_t port,
                         Direction direction,
                         MidiMessage::Type msgType);
    uint8_t channel;
    uint16_t rate;
    uint32_t tsLastMessage;
};
