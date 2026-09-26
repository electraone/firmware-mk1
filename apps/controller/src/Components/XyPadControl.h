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
 * @file XyPadControl.h
 *
 * @brief Implements an on-screen XY pad Control.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"
#include "MultiValue.h"

/**
 * @brief A ground with two crosshairs on it, one per value.
 *
 * The bottom left corner is (0, 0): Y counts up from the bottom edge, so
 * turning its knob clockwise moves the line up.
 */
class XyPadControl : public ControlComponent, public MultiValue
{
public:
    XyPadControl(const Control &control, MainDelegate &newDelegate);
    virtual ~XyPadControl() = default;

    void syncComponentProperties(void) override;

    void onTouchDown(const TouchEvent &touchEvent) override;
    void onTouchMove(const TouchEvent &touchEvent) override;
    void onTouchUp(const TouchEvent &touchEvent) override;
    void onPotTouchDown(const PotEvent &potEvent) override;
    void onPotChange(const PotEvent &potEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;
    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

    uint8_t getNumHandles(void) const override;
    void paint(Graphics &g) override;

    static constexpr uint8_t xHandle = 0;
    static constexpr uint8_t yHandle = 1;
    static constexpr uint8_t numHandles = 2;

protected:
    void setValueRanges(void);

    /** Where the handle sits inside a pad of that size, in pixels. */
    uint16_t handleX(uint16_t width) const;
    uint16_t handleY(uint16_t height) const;
};
