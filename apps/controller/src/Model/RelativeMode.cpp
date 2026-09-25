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

#include "RelativeMode.h"
#include <cstring>

RelativeMode translateRelativeMode(const char *relativeModeText)
{
    if (relativeModeText) {
        if (strcmp(relativeModeText, "twosComplement") == 0) {
            return (RelativeMode::twosComplement);
        } else if (strcmp(relativeModeText, "signBit2") == 0) {
            return (RelativeMode::signBit2);
        } else if (strcmp(relativeModeText, "binOffset") == 0) {
            return (RelativeMode::binOffset);
        }
    }
    return (RelativeMode::signBit);
}