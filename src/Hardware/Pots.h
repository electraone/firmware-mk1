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
 * @file Pots.h
 *
 * @brief A collection of all hardware Potentiometers.
 */

#pragma once

#include "settings.h"
#include "Pot.h"
#include "PotTouch.h"

class Pots
{
public:
    Pots();
    ~Pots() = default;

    Pot &operator[](const uint8_t id);
    void rescanAllPots(void);
    void initialise(void);

private:
    Pot pots[NR_OF_HW_POTS] = {
        Pot(0), Pot(1), Pot(2), Pot(3), Pot(4),  Pot(5),
        Pot(6), Pot(7), Pot(8), Pot(9), Pot(10), Pot(11)
    };
};
