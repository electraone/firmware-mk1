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
 * @file ListControl.h
 *
 * @brief Implements a List-style Control for lists of discrete values.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"
#include "List.h"

class ListControl : public ControlComponent, public List
{
public:
    static constexpr uint16_t thresholdSwipeDistance = 40;

    ListControl(const Control &control, MainDelegate &newDelegate);
    virtual ~ListControl() = default;

    virtual void syncComponentProperties(void) override;
    virtual void onTouchUp(const TouchEvent &touchEvent) override;
    virtual void onPotTouchDown(const PotEvent &potEvent) override;
    virtual void onPotChange(const PotEvent &potEvent) override;
    virtual void onPotTouchUp(const PotEvent &potEvent) override;
    virtual void onMidiValueChange(const ControlValue &value,
                                   int16_t midiValue,
                                   uint8_t handle = 0) override;

    void paint(Graphics &g) override;

    void emitValueChange(int16_t newIndex, const ControlValue &cv);

private:
    void paintList(Graphics &g,
                   const Rectangle &bounds,
                   uint32_t colour,
                   const ListData *items,
                   uint16_t activeIndex);

    static Overlay empty;
};
