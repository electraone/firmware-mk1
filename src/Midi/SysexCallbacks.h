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
 * @file SysexCallbacks.h
 *
 * @brief A collection of low-level Sysex callbacks.
 */

#pragma once

#include <stdint.h>

void handleMidiIoSysExPort0(const uint8_t *sysExData,
                            uint16_t sysExSize,
                            bool complete);
void handleMidiIoSysExPort1(const uint8_t *sysExData,
                            uint16_t sysExSize,
                            bool complete);
void handleMidiUsbDevSysEx(uint8_t cable,
                           const uint8_t *sysExData,
                           uint16_t sysExSize,
                           bool complete);
void handleMidiUsbHostSysExPort0(uint8_t cable,
                                 const uint8_t *sysExData,
                                 uint16_t sysExSize,
                                 bool complete);
void handleMidiUsbHostSysExPort1(uint8_t cable,
                                 const uint8_t *sysExData,
                                 uint16_t sysExSize,
                                 bool complete);

/** Handle what the CTRL handlers kept back during a repaint. */
void flushDeferredCtrlSysex(void);
