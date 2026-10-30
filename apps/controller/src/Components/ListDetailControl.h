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
 * @file ListDetailControl.h
 *
 * @brief Implements an detail version of the List Control.
 */

#pragma once

#include "ListControl.h"

// \todo height of the item is fixed to 80. Changing it will break the layout
class ListDetailControl : public ListControl
{
public:
    ListDetailControl(const Control &control, MainDelegate &newDelegate);
    virtual ~ListDetailControl() = default;

    void paint(Graphics &g) override;
    void resized(void) override;

    void onTouchDown(const TouchEvent &touchEvent) override;
    void onTouchMove(const TouchEvent &touchEvent) override;
    void onTouchClick(const TouchEvent &touchEvent) override;
    void onTouchUp(const TouchEvent &touchEvent) override;
    void onTouchLongHold(const TouchEvent &touchEvent);
    void onPotTouchUp(const PotEvent &potEvent) override;
    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

private:
    void paintListItems(Graphics &g, int16_t offset);
    void determineOffsetY(void);

    std::function<void(void)> emit;

private:
    bool valueChosen;
    int16_t offsetY;
    int16_t prevTouchY;

    uint16_t listItemWidth;
    uint16_t listItemHeight;
    uint16_t maxItemsInViewport;

    uint16_t itemTop;
};
