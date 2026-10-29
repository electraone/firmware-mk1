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
 * @file MidiInputCallback.h
 *
 * @brief A base class for MIDI input processing objects.
 */

#pragma once

#include "MidiMessage.h"
#include "DeviceManager.h"

/**
 * @brief Receives incoming MIDI messages from Electra's MIDI interfaces.
 */
class MidiInputCallback
{
public:
    MidiInputCallback();
    virtual ~MidiInputCallback() = default;

    /**
     * Gets called for all incoming MIDI messages besides Active sensing,
     * Clock, and SysEx.
     *
     * @param midiInput information about the MIDI message origin.
     * @param midiMessage received MIDI message.
     */
    virtual void handleIncomingMidiMessage(const MidiInput &midiInput,
                                           const MidiMessage &midiMessage) = 0;

    /** Optional callbacks handlers
     *
     */
    static bool (*routeMessageCallback)(MidiInput &midiInput,
                                        MidiMessage &midiMessage);
    static void (*routePartialSysexCallback)(MidiInput &midiInput,
                                             const uint8_t *sysExData,
                                             uint16_t sysExSize,
                                             bool complete);
    static void (*onMidiClockCallback)(MidiInput midiInput);
    static void (*onMidiStartCallback)(MidiInput midiInput);
    static void (*onMidiStopCallback)(MidiInput midiInput);
    static void (*onMidiContinueCallback)(MidiInput midiInput);
    static void (*onMidiActiveSensingCallback)(MidiInput midiInput);
    static void (*onMidiSystemResetCallback)(MidiInput midiInput);

    /**
     * Every message processMidi() hands over, after everything else has seen
     * it. Set by the app once and left alone across presets - unlike the Lua
     * callbacks around it, which a preset load clears.
     */
    static void (*onAnyMessageCallback)(MidiInput midiInput,
                                        uint8_t channel,
                                        MidiMessage::Type type,
                                        uint8_t data1,
                                        uint8_t data2);
    static void (*onMidiTuneRequestCallback)(MidiInput midiInput);
    static void (*onMidiProgramChangeCallback)(MidiInput midiInput,
                                               uint8_t channel,
                                               uint8_t programNumber);
    static void (*onMidiAfterTouchChannelCallback)(MidiInput midiInput,
                                                   uint8_t channel,
                                                   uint8_t pressure);
    static void (*onMidiSongSelectCallback)(MidiInput midiInput,
                                            uint8_t songNumber);
    static void (*onMidiPitchBendCallback)(MidiInput midiInput,
                                           uint8_t channel,
                                           int value);
    static void (*onMidiSongPositionCallback)(MidiInput midiInput,
                                              int position);
    static void (*onMidiControlChangeCallback)(MidiInput midiInput,
                                               uint8_t channel,
                                               uint8_t controllerNumber,
                                               uint8_t value);
    static void (*onMidiNoteOnCallback)(MidiInput midiInput,
                                        uint8_t channel,
                                        uint8_t noteNumber,
                                        uint8_t velocity);
    static void (*onMidiNoteOffCallback)(MidiInput midiInput,
                                         uint8_t channel,
                                         uint8_t noteNumber,
                                         uint8_t velocity);
    static void (*onMidiAfterTouchPolyCallback)(MidiInput midiInput,
                                                uint8_t channel,
                                                uint8_t noteNumber,
                                                uint8_t pressure);
    static void (*onMidiSysexCallback)(MidiInput &midiInput,
                                       MidiMessage &midiMessage);
    static void (*onMidiMessageCallback)(MidiInput &midiInput,
                                         MidiMessage &midiMessage);

    static DeviceManager deviceManager;
};
