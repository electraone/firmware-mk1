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

#include "MidiIo.h"

// \todo move to the MidiIo class.
/* Spelled out rather than through MIDI_CREATE_INSTANCE, to put them below the
 * SRAM bank boundary at 0x20000000: each carries a SysEx buffer that is copied
 * from, and a copy spanning SRAM_L and SRAM_U hard faults. See the linker
 * script's .bss.lowram. */
__attribute__((section(".bss.lowram")))
midi::MidiInterface<Uart> MIDI1((Uart &)Serial1);
__attribute__((section(".bss.lowram")))
midi::MidiInterface<Uart> MIDI2((Uart &)Serial2);

constexpr midi::MidiInterface<Uart> *MidiIo::midiDINInterfaces[2];
