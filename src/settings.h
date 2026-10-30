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

#define FIRMWARE_VERSION "v3.5.5"
#define FIRMWARE_SEQ 300500500
#define UI_FILE "ui-0.9.6.bmp"
#define JSON_VERSION 2

#define NR_OF_PAGES 12
#define NR_OF_DEVICES 2
#define NR_OF_MIDI_INTERFACES 2
#define NR_OF_MIDI_CHANNELS 16
#define NR_OF_HW_BUTTONS 6
#define NR_OF_USB_DEVICES 2
#define NR_OF_HW_POTS 12
#define MAX_POT_ID 12
#define MIDI_CTRL_PORT 2

#define MAX_DEVICE_NAME_LENGTH 20
#define TOUCH_LONG_HOLD_TIME 250
#define MAX_FILENAME_LENGTH 64
#define MAX_DIRNAME_LENGTH 64
