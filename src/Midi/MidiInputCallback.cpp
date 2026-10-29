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
 * @file MidiInputCallback.cpp
 *
 * @brief A base class for MIDI input processing objects.
 */

#include "MidiInputCallback.h"

MidiInputCallback::MidiInputCallback()
{
    deviceManager.addMidiInputDeviceCallback(this);
}

bool (*MidiInputCallback::routeMessageCallback)(MidiInput &midiInput,
                                                MidiMessage &midiMessage) =
    nullptr;
void (*MidiInputCallback::routePartialSysexCallback)(MidiInput &midiInput,
                                                     const uint8_t *sysExData,
                                                     uint16_t sysExSize,
                                                     bool complete) = nullptr;
void (*MidiInputCallback::onMidiClockCallback)(MidiInput midiInput) = nullptr;
void (*MidiInputCallback::onMidiStartCallback)(MidiInput midiInput) = nullptr;
void (*MidiInputCallback::onMidiStopCallback)(MidiInput midiInput) = nullptr;
void (*MidiInputCallback::onMidiContinueCallback)(MidiInput midiInput) =
    nullptr;
void (*MidiInputCallback::onMidiActiveSensingCallback)(MidiInput midiInput) =
    nullptr;
void (*MidiInputCallback::onMidiSystemResetCallback)(MidiInput midiInput) =
    nullptr;
void (*MidiInputCallback::onMidiTuneRequestCallback)(MidiInput midiInput) =
    nullptr;
void (*MidiInputCallback::onAnyMessageCallback)(MidiInput midiInput,
                                                uint8_t channel,
                                                MidiMessage::Type type,
                                                uint8_t data1,
                                                uint8_t data2) = nullptr;
void (*MidiInputCallback::onMidiProgramChangeCallback)(MidiInput midiInput,
                                                       uint8_t channel,
                                                       uint8_t programNumber) =
    nullptr;
void (*MidiInputCallback::onMidiAfterTouchChannelCallback)(MidiInput midiInput,
                                                           uint8_t channel,
                                                           uint8_t pressure) =
    nullptr;
void (*MidiInputCallback::onMidiSongSelectCallback)(MidiInput midiInput,
                                                    uint8_t songNumber) =
    nullptr;
void (*MidiInputCallback::onMidiPitchBendCallback)(MidiInput midiInput,
                                                   uint8_t channel,
                                                   int value) = nullptr;
void (*MidiInputCallback::onMidiSongPositionCallback)(MidiInput midiInput,
                                                      int position) = nullptr;
void (*MidiInputCallback::onMidiControlChangeCallback)(MidiInput midiInput,
                                                       uint8_t channel,
                                                       uint8_t controllerNumber,
                                                       uint8_t value) = nullptr;
void (*MidiInputCallback::onMidiNoteOnCallback)(MidiInput midiInput,
                                                uint8_t channel,
                                                uint8_t noteNumber,
                                                uint8_t velocity) = nullptr;
void (*MidiInputCallback::onMidiNoteOffCallback)(MidiInput midiInput,
                                                 uint8_t channel,
                                                 uint8_t noteNumber,
                                                 uint8_t velocity) = nullptr;
void (*MidiInputCallback::onMidiAfterTouchPolyCallback)(MidiInput midiInput,
                                                        uint8_t channel,
                                                        uint8_t noteNumber,
                                                        uint8_t pressure) =
    nullptr;
void (*MidiInputCallback::onMidiSysexCallback)(MidiInput &midiInput,
                                               MidiMessage &midiMessage) =
    nullptr;
void (*MidiInputCallback::onMidiMessageCallback)(MidiInput &midiInput,
                                                 MidiMessage &midiMessage) =
    nullptr;

DeviceManager MidiInputCallback::deviceManager;
