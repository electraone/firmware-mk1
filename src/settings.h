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

#define FIRMWARE_VERSION "v3.5.4"
#define FIRMWARE_SEQ 300500400
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
 * one converter range, and the converter here is ten bits. */
#define POT_COUNTS_PER_REVOLUTION (4096)

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

/* How often one knob is read, in microseconds. Only the knobs under a finger
 * are read, so this is the cadence the knob being turned gets: two hundred
 * readings a second, where it used to be sixty-six. One reading costs sixty
 * microseconds, measured, so a knob in a hand is a bit over one percent of the
 * processor and an untouched panel is none of it.
 *
 * It matters more than it used to. Rotation is no longer multiplied by how fast
 * the knob is moving, so what reaches a control is however much travel arrived
 * since the last reading: read too seldom and a quick sweep lands as a handful
 * of jumps instead of a slide. */
#define POT_SCAN_TICK (5000)

/* The same, for the special boot that does not use knob touch and so has to
 * sweep all twelve. Faster per reading, because each reading is a different
 * knob: twelve of these is one visit to each knob, which puts a knob's own
 * cadence at thirty a second rather than two hundred. That is what it has
 * always been, and this boot mode exists for a unit whose touch sensing does
 * not work rather than for turning knobs on. */
#define POT_SCAN_TICK_ALL (2500)

#define MIDI_CTRL_PORT 2

#define MAX_DEVICE_NAME_LENGTH 20
#define TOUCH_LONG_HOLD_TIME 250
#define MAX_FILENAME_LENGTH 64
#define MAX_DIRNAME_LENGTH 64
