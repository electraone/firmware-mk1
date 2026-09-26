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
 * @file HardwareControl.h
 *
 * @brief What the six buttons on the panel do.
 */

#pragma once

#include "UiCommand.h"
#include "settings.h"

#include <cstdint>

/**
 * @brief One command a button runs, and whatever it needs to know.
 */
struct ButtonAction {
    UiCommand::Type command;
    int16_t parameter;

    bool isAssigned(void) const
    {
        return (command != UiCommand::Type::None);
    }
};

/**
 * @brief The panel's buttons, as the configuration has them.
 *
 * A button carries two commands, because a button means a different thing
 * while a finger is on a knob: the same three buttons that pick a control set
 * open the detail of whatever is being turned. What the configuration does not
 * name is left as None, and the window then does what it has always done - so
 * an instrument with no hardware section in its configuration behaves exactly
 * as it did before there was one.
 */
struct HardwareControl {
    ButtonAction button[NR_OF_HW_BUTTONS];
    ButtonAction knobTouch[NR_OF_HW_BUTTONS];

    void reset(void)
    {
        for (uint8_t i = 0; i < NR_OF_HW_BUTTONS; i++) {
            button[i] = { UiCommand::Type::None, 0 };
            knobTouch[i] = { UiCommand::Type::None, 0 };
        }
    }

    /**
     * @brief The button a name in the configuration stands for.
     *
     * The mk2's names, so that one configuration reads the same on both.
     *
     * @return the button id, or NR_OF_HW_BUTTONS when the name is not one
     */
    static uint8_t translateButtonName(const char *name);
};
