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

#include "MidiUsbHost.h"

/* Below the SRAM bank boundary at 0x20000000: the host controller transfers
 * into these devices' buffers, and an access spanning SRAM_L and SRAM_U hard
 * faults. See the linker script's .bss.lowram. */
#define LOWRAM __attribute__((section(".bss.lowram")))

// \todo needs to be moved to the MidiUsbHost class
LOWRAM USBHost usbHost;
LOWRAM USBHub hub1(usbHost);
LOWRAM MIDIDevice midiDevice1(usbHost);
LOWRAM MIDIDevice midiDevice2(usbHost);

USBDriver *drivers[] = { &hub1, &midiDevice1, &midiDevice2 };

const char *driverNames[3] = { "HUB1", "MIDI1", "MIDI2" };
bool driverActive[3] = {};
USBDevice USBDevices[2];
