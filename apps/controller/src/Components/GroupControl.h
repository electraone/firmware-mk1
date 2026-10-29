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
 * @file GroupControl.h
 *
 * @brief Implements an on-screen group visual.
 */

#pragma once

#include "Group.h"
#include "Component.h"

class GroupControl : public Component
{
public:
    explicit GroupControl(const Group &groupToAssign);
    virtual ~GroupControl() = default;

    void setColour(uint32_t newColour);
    void setLabel(const char *newLabel);
    void setUseAltBackground(bool shouldUseAltBackground);
    bool getUseAltBackground(void) const;

    void paint(Graphics &g) override;

    static constexpr uint8_t MaxSetLabelLength = 40;
    uint32_t colour;
    /* A copy, not a pointer: the group fetches its label from the display's
     * memory into a buffer that the next fetch overwrites. */
    char label[MaxSetLabelLength + 1];
    const Group &group;
    bool useAltBackground;
};
