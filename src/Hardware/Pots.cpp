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

#include "Hardware.h"
#include "System.h"
#include "pins_electra.h"

Pots::Pots()
{
}

Pot &Pots::operator[](const uint8_t id)
{
    return (pots[id]);
}

/**
 * @brief Take every knob's first reading, so each has a reference to move from.
 *
 * Each knob is given the same settling time the scan gives it before it is
 * read - see POT_SCAN_TICK - since a reading taken straight after the
 * multiplexer moves carries some of the knob read before it.
 */
void Pots::rescanAllPots(void)
{
    for (uint8_t address = 0; address < NR_OF_HW_POTS; address++) {
        MUX_SET_ADDRESS(address);
        MUX_ENABLE;
        delayMicroseconds(POT_SCAN_TICK);
        pots[address].initialise();
    }

    MUX_DISABLE;
}

void Pots::initialise(void)
{
    // Initialise Pot Touch
    Hardware::potTouch.begin();
    Hardware::potTouch.reset();

    // Configure Pot Touch
    if (!Hardware::potTouch.init()) {
        System::logger.write(
            LOG_ERROR,
            "capacitive sensing: cannot calibrate. Check input USB voltage");
    } else {
        System::logger.write(LOG_ERROR, "capacitive sensing: Initialized");
    }

    // Run the first scan of Pots to get them to known state
    rescanAllPots();
}
