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
 * @file Hardware.h
 *
 * @brief A wrapper class for all hardware related functionality.
 */

#pragma once

#include "pins_electra.h"
#include "Pots.h"
#include "PotTouch.h"
#include "Buttons.h"
#include "Touch.h"
#include "Graphics.h"
#include "PersistentStorage.h"
#include "Memory.h"
#include "RamMonitor.h"

// A list of all hardware features of Electra One platform.
// Members are made public on purpose.

/* This is to be able to initiate USB stack in here. To give more
 * control over the USB device configuration
 */
extern "C" void usb_init(uint8_t keyboardEnabled,
                         uint8_t mouseEnabled,
                         uint8_t keymediaEnabled);
extern "C" void usb_stop(void);

class Hardware
{
public:
    Hardware() = delete;

    static void initialise(void);

    /**
     * @brief Mount the card and hand the pools their storage.
     *
     * Apart from initialise() because the mount is the long part of a boot
     * on this model - seconds, on a card that only answers in 1-bit mode -
     * and the display is up by then: the caller puts something on it first.
     */
    static void mountStorage(void);

    static void reset(void);

    static Pots pots;
    static PotTouch potTouch;
    static Buttons buttons;
    static ADC adc;
    static Touch touch;
    static Graphics screen;
    static PersistentStorage sdcard;
    static Memory memory;
    static RamMonitor ram;
};
