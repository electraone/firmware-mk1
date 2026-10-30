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
 * @file RelativeControl.h
 *
 * @brief Implements a visual for managing relative Controls.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"

class RelativeControl : public ControlComponent
{
public:
    RelativeControl(const Control &control, MainDelegate &newDelegate);
    virtual ~RelativeControl() = default;

    virtual void onPotTouchDown(const PotEvent &potEvent) override;
    virtual void onPotChange(const PotEvent &potEvent) override;
    virtual void onPotTouchUp(const PotEvent &potEvent) override;

    virtual void onMidiValueChange(const ControlValue &value,
                                   int16_t midiValue,
                                   uint8_t handle = 0) override;

    void paint(Graphics &g) override;

private:
    int16_t delta;

    void leftArrow(Graphics &g,
                   uint16_t x,
                   bool active,
                   uint32_t colour,
                   uint32_t colourTrack);
    void rightArrow(Graphics &g,
                    uint16_t x,
                    bool active,
                    uint32_t colour,
                    uint32_t colourTrack);

    static constexpr uint16_t padding = 8;
    static constexpr uint16_t yPosition = 21;
};
