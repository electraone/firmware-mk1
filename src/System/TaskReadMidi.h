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
 * @file TaskReadMidi.h
 *
 * @brief A task to read MIDI messages from the MIDI interface.
 */

#pragma once

#include "MidiMessage.h"
#include "MidiInterface.h"
#define CIRCULAR_BUFFER_INT_SAFE
#include "CircularBuffer.h"

void readCtrlMidi(void);
void readMidi(void);

/** Messages one tick may take from the USB device port; see the definition. */
extern uint8_t readMidiUsbDevBudget;
void processMidi(void);
bool runRouteMessageCallback(MidiInput &midiInput, MidiMessage &midiMessage);
void runOptionalCallbacks(MidiInput midiInput,
                          uint8_t channel,
                          MidiMessage::Type type,
                          uint8_t data1,
                          uint8_t data2);
