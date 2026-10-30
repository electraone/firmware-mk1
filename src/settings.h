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
 * @file settings.h
 *
 * @brief A base configuration of the controller device.
 * 
 * @todo this needs to be reworked.
 */

#pragma once

#define FIRMWARE_VERSION "v5.0.6"
#define FIRMWARE_SEQ 500000600
#define UI_FILE "ui-1.0.0.bmp"
#define JSON_VERSION 2

#define NR_OF_PAGES 12
#define NR_OF_DEVICES 2
#define NR_OF_MIDI_INTERFACES 2
#define NR_OF_MIDI_CHANNELS 16
#define NR_OF_HW_BUTTONS 6
#define NR_OF_USB_DEVICES 2
#define NR_OF_HW_POTS 12
#define MAX_POT_ID 12

/* Phase counts the pot reader reports for one whole turn of a knob. The two
 * gangs of a knob sit a quarter turn apart, so read against each other they
 * trace a diamond that is walked once per revolution; a quarter of that walk is
 * one converter range, and the converter is read at twelve bits, as on the
 * mk2. */
#define POT_COUNTS_PER_REVOLUTION (8192)

/* How far a knob travels to take a proportional control from one end of its
 * range to the other. Three hundred degrees is about as far as a wrist reaches
 * in one go, so minimum to maximum is a single gesture with no regripping,
 * whatever the control's resolution. */
#define POT_SWEEP_DEGREES (300)
#define POT_SWEEP_COUNTS ((POT_COUNTS_PER_REVOLUTION * POT_SWEEP_DEGREES) / 360)

/* How far a knob turns between two steps of a discrete control. Lists, pads and
 * anything else that counts rather than sweeps reads like an encoder instead -
 * one step per detent, regardless of how many entries there are - so that
 * stepping through a list feels the same whether it holds four entries or
 * forty. */
#define POT_ENCODER_DETENT_DEGREES (45)
#define POT_ENCODER_DETENT_COUNTS                                              \
    ((POT_COUNTS_PER_REVOLUTION * POT_ENCODER_DETENT_DEGREES) / 360)

/* Microseconds the multiplexer rests on one knob before that knob is read. One
 * knob is read per tick, so this is also the sampling rate: every knob is read
 * whether or not anything touches it, and one that has moved recently is read
 * on every other tick - see TaskReadPots. Switching the multiplexer dumps the
 * charge on its common node onto the selected wiper, which then needs tens of
 * microseconds per time constant to recover; a whole tick is far more than
 * that. The same figure as the mk2's. */
#define POT_SCAN_TICK (1000)

/* How long after its last movement a knob keeps its place in the fast rota. */
#define POT_ACTIVE_HOLD_MS (250)

#define MIDI_CTRL_PORT 2

#define MAX_DEVICE_NAME_LENGTH 20
#define TOUCH_LONG_HOLD_TIME 250
#define MAX_FILENAME_LENGTH 64
#define MAX_DIRNAME_LENGTH 64
