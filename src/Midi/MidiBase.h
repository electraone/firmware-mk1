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
 * @file MidiBase.h
 *
 * @brief A base class for MIDI interfaces.
 */

#pragma once

#include "MidiMessage.h"

class MidiBase
{
public:
    virtual ~MidiBase() = default;

    virtual void send(uint8_t port,
                      MidiMessage::Type type,
                      uint8_t channel,
                      uint8_t data1,
                      uint8_t data2) const = 0;
        virtual void send(uint8_t port, MidiMessage &message) = 0;

    /** Whether a message of that many bytes can be sent now without
     *  waiting. Only a serial port ever says no: its line is slow and its
     *  buffer finite, and a caller on an interrupt must not wait for it. */
    virtual bool canSend(uint8_t port, size_t bytes) const
    {
        (void)port;
        (void)bytes;
        return (true);
    }
    virtual void send(uint8_t port, SysexBlock &sysexBlock) = 0;
    virtual void sendControlChange(uint8_t port,
                                   uint8_t parameterNumber,
                                   uint8_t value,
                                   uint8_t channel) const = 0;
    virtual void sendNoteOn(uint8_t port,
                            uint8_t channel,
                            uint8_t noteNumber,
                            uint8_t velocity) const = 0;
    virtual void sendNoteOff(uint8_t port,
                             uint8_t channel,
                             uint8_t noteNumber,
                             uint8_t velocity) const = 0;
    virtual void sendStart(uint8_t port) const = 0;
    virtual void sendStop(uint8_t port) const = 0;
    virtual void sendTuneRequest(uint8_t port) const = 0;
    virtual void sendProgramChange(uint8_t port,
                                   uint8_t channel,
                                   uint8_t programNumber) const = 0;
    virtual void sendSysEx(uint8_t port, SysexBlock &sysexBlock) = 0;
    virtual void sendSysEx(uint8_t port,
                           uint8_t *sysexData,
                           uint16_t sysexDataLength) const = 0;
    virtual void sendSysExPartial(uint8_t port,
                                  const uint8_t *sysexData,
                                  uint16_t sysexDataLength,
                                  bool complete) const = 0;
    virtual void
        sendPitchBend(uint8_t port, uint8_t channel, uint16_t value) const = 0;
    virtual void sendAfterTouchPoly(uint8_t port,
                                    uint8_t channel,
                                    uint8_t noteNumber,
                                    uint8_t pressure) const = 0;
    virtual void sendAfterTouchChannel(uint8_t port,
                                       uint8_t channel,
                                       uint8_t pressure) const = 0;
    virtual void sendClock(uint8_t port) const = 0;
    virtual void sendContinue(uint8_t port) const = 0;
    virtual void sendActiveSensing(uint8_t port) const = 0;
    virtual void sendSystemReset(uint8_t port) const = 0;
    virtual void sendSongSelect(uint8_t port, uint8_t song) const = 0;
    virtual void sendSongPosition(uint8_t port, uint16_t beats) const = 0;

    virtual void readNoAction(uint8_t port) const = 0;
    virtual bool read(uint8_t port) = 0;

    virtual uint8_t getChannel(uint8_t port) const = 0;
    virtual uint8_t getType(uint8_t port) const = 0;
    virtual uint8_t getData1(uint8_t port) const = 0;
    virtual uint8_t getData2(uint8_t port) const = 0;
    virtual uint8_t getActivePort(void) const = 0;

    virtual void initialise(void) const = 0;

    virtual void enableThru(uint8_t port, bool shouldBeEnabled) = 0;
};
