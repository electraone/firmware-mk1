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
 * @file RelativeMode.h
 *
 * @brief Collection of functions to work with relative modes.
 */

#pragma once

enum class RelativeMode {
    twosComplement = 0,
    signBit = 1,
    signBit2 = 2,
    binOffset = 3,
    noSign = 4
};

RelativeMode translateRelativeMode(const char *relativeModeText);