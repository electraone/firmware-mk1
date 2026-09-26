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
 * @file MidiIo.h
 *
 * @brief A MIDI IO interface class.
 */

#pragma once

#include "MidiBase.h"
#include <MIDI.h>

// \todo Move to the class as static members.
extern midi::MidiInterface<Uart> MIDI1;
extern midi::MidiInterface<Uart> MIDI2;

void handleMidiIoSysExPort0(const uint8_t *sysExData,
                            uint16_t sysExSize,
                            bool complete);
void handleMidiIoSysExPort1(const uint8_t *sysExData,
                            uint16_t sysExSize,
                            bool complete);

/*
 * set up DIN5 MIDI interfaces
 */

class MidiIo : public MidiBase
{
public:
    MidiIo() : activePort(0)
    {
    }

    ~MidiIo() = default;

    void send(uint8_t port,
              MidiMessage::Type type,
              uint8_t channel,
              uint8_t data1,
              uint8_t data2) const override
    {
        midiDINInterfaces[port]->send(
            static_cast<midi::MidiType>(type), data1, data2, channel);
    }

        bool canSend(uint8_t port, size_t bytes) const override
    {
        const Uart &uart = (port == 0) ? Serial1 : Serial2;
        return (const_cast<Uart &>(uart).availableForWrite() >= (int)bytes);
    }

    void send(uint8_t port, MidiMessage &message) override
    {
        midiDINInterfaces[port]->send(
            static_cast<midi::MidiType>(message.getType()),
            message.getData1(),
            message.getData2(),
            message.getChannel());
    }

    void send(uint8_t port, SysexBlock &sysexBlock) override
    {
        sendSysEx(port, sysexBlock);
    }

    void sendControlChange(uint8_t port,
                           uint8_t parameterNumber,
                           uint8_t value,
                           uint8_t channel) const override
    {
        midiDINInterfaces[port]->sendControlChange(
            parameterNumber, value, channel);
    }

    void sendNoteOn(uint8_t port,
                    uint8_t channel,
                    uint8_t noteNumber,
                    uint8_t velocity) const override
    {
        midiDINInterfaces[port]->sendNoteOn(noteNumber, velocity, channel);
    }

    void sendNoteOff(uint8_t port,
                     uint8_t channel,
                     uint8_t noteNumber,
                     uint8_t velocity) const override
    {
        midiDINInterfaces[port]->sendNoteOff(noteNumber, velocity, channel);
    }

    void sendStart(uint8_t port) const override
    {
        midiDINInterfaces[port]->sendRealTime(midi::Start);
    }

    void sendStop(uint8_t port) const override
    {
        midiDINInterfaces[port]->sendRealTime(midi::Stop);
    }

    void sendTuneRequest(uint8_t port) const override
    {
        midiDINInterfaces[port]->sendTuneRequest();
    }

    void sendProgramChange(uint8_t port,
                           uint8_t channel,
                           uint8_t programNumber) const override
    {
        midiDINInterfaces[port]->sendProgramChange(programNumber, channel);
    }

    void sendSysEx(uint8_t port, SysexBlock &sysexBlock) override
    {
        uint8_t buffer[MemoryBlock::streamChunkSize];
        sysexBlock.seek(0);
        size_t readBytes = 0;
        while ((readBytes = sysexBlock.readBytes(buffer, sizeof(buffer)))
               != 0) {
            sendSysExPartial(port, buffer, readBytes, false);
        }
    }

    void sendSysEx(uint8_t port,
                   uint8_t *sysexData,
                   uint16_t sysexDataLength) const override
    {
        midiDINInterfaces[port]->sendSysEx(sysexDataLength, sysexData, false);
    }

    /**
     * @brief Sends one chunk of a system exclusive message on a DIN port.
     *
     * @attention `complete` is deliberately not used, and the parameter is
     * only here because the interface declares it. A stored message carries
     * its own 0xF0 and 0xF7, and the `true` below is what tells the library
     * not to add its own, so the chunks concatenate into the right thing on
     * the wire whether or not this one is the last. Passing `complete`
     * through would wrap every chunk but the final one in a 0xF0 and 0xF7 of
     * its own.
     */
    void sendSysExPartial(uint8_t port,
                          const uint8_t *sysexData,
                          uint16_t sysexDataLength,
                          [[maybe_unused]] bool complete) const override
    {
        midiDINInterfaces[port]->sendSysEx(sysexDataLength, sysexData, true);
    }

    void sendPitchBend(uint8_t port,
                       uint8_t channel,
                       uint16_t value) const override
    {
        midiDINInterfaces[port]->sendPitchBend(value - 8191, channel);
    }

    void sendAfterTouchPoly(uint8_t port,
                            uint8_t channel,
                            uint8_t noteNumber,
                            uint8_t pressure) const override
    {
        midiDINInterfaces[port]->sendPolyPressure(
            noteNumber, pressure, channel);
    }

    void sendAfterTouchChannel(uint8_t port,
                               uint8_t channel,
                               uint8_t pressure) const override
    {
        midiDINInterfaces[port]->sendAfterTouch(pressure, channel);
    }

    void sendClock(uint8_t port) const override
    {
        midiDINInterfaces[port]->sendRealTime(midi::Clock);
    }

    void sendContinue(uint8_t port) const override
    {
        midiDINInterfaces[port]->sendRealTime(midi::Continue);
    }

    void sendActiveSensing(uint8_t port) const override
    {
        midiDINInterfaces[port]->sendRealTime(midi::ActiveSensing);
    }

    void sendSystemReset(uint8_t port) const override
    {
        midiDINInterfaces[port]->sendRealTime(midi::SystemReset);
    }

    void sendSongSelect(uint8_t port, uint8_t song) const override
    {
        midiDINInterfaces[port]->sendSongSelect(song);
    }

    void sendSongPosition(uint8_t port, uint16_t beats) const override
    {
        midiDINInterfaces[port]->sendSongPosition(beats);
    }

    void readNoAction(uint8_t port) const override
    {
        midiDINInterfaces[port]->readNoAction();
    }

    bool read(uint8_t port) override
    {
        activePort = port;
        return (midiDINInterfaces[port]->read());
    }

    uint8_t getChannel(uint8_t port) const override
    {
        return (midiDINInterfaces[port]->getChannel());
    }

    uint8_t getType(uint8_t port) const override
    {
        return (midiDINInterfaces[port]->getType());
    }

    uint8_t getData1(uint8_t port) const override
    {
        return (midiDINInterfaces[port]->getData1());
    }

    uint8_t getData2(uint8_t port) const override
    {
        return (midiDINInterfaces[port]->getData2());
    }

    uint8_t getActivePort(void) const override
    {
        return (activePort);
    }

    void initialise(void) const override
    {
        midiDINInterfaces[0]->begin(MIDI_CHANNEL_OMNI);
        midiDINInterfaces[1]->begin(MIDI_CHANNEL_OMNI);

        midiDINInterfaces[0]->turnThruOff();
        midiDINInterfaces[1]->turnThruOff();

        midiDINInterfaces[0]->setHandleSystemExclusivePartial(
            handleMidiIoSysExPort0);
        midiDINInterfaces[1]->setHandleSystemExclusivePartial(
            handleMidiIoSysExPort1);
    }

    void enableThru(uint8_t port, bool shouldBeEnabled) override
    {
        if (shouldBeEnabled) {
            midiDINInterfaces[port]->turnThruOn();
        } else {
            midiDINInterfaces[port]->turnThruOff();
        }
    }

private:
    uint8_t activePort;
    static constexpr midi::MidiInterface<Uart> *midiDINInterfaces[2] = {
        &MIDI1,
        &MIDI2
    };
};
