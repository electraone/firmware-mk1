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
 * @file HardwareControl.cpp
 *
 * @brief What the six buttons on the panel do.
 */

#include "HardwareControl.h"
#include "Buttons.h"

#include <cstring>

uint8_t HardwareControl::translateButtonName(const char *name)
{
    if (name) {
        if (strcmp(name, "leftTop") == 0) {
            return (BUTTON_LEFT_TOP);
        } else if (strcmp(name, "leftMiddle") == 0) {
            return (BUTTON_LEFT_MIDDLE);
        } else if (strcmp(name, "leftBottom") == 0) {
            return (BUTTON_LEFT_BOTTOM);
        } else if (strcmp(name, "rightTop") == 0) {
            return (BUTTON_RIGHT_TOP);
        } else if (strcmp(name, "rightMiddle") == 0) {
            return (BUTTON_RIGHT_MIDDLE);
        } else if (strcmp(name, "rightBottom") == 0) {
            return (BUTTON_RIGHT_BOTTOM);
        }
    }

    return (NR_OF_HW_BUTTONS);
}
