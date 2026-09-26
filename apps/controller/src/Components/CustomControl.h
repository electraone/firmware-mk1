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
 * @file CustomControl.h
 *
 * @brief Implements a generic Control with a paint function implemented
 *        in Lua.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"

/**
 * @brief A control the preset's script draws and drives.
 *
 * Nothing about it is decided here: what it looks like is the paint callback,
 * and what a finger or a knob does to it is whichever of the other four
 * callbacks the script subscribed to. A control with no script draws its
 * background and nothing else.
 */
class CustomControl : public ControlComponent
{
public:
    CustomControl(const Control &control, MainDelegate &newDelegate);
    virtual ~CustomControl() = default;

    void onTouchDown(const TouchEvent &touchEvent) override;
    void onTouchMove(const TouchEvent &touchEvent) override;
    void onTouchUp(const TouchEvent &touchEvent) override;
    void onTouchClick(const TouchEvent &touchEvent) override;
    void onTouchDoubleClick(const TouchEvent &touchEvent) override;
    void onTouchLongHold(const TouchEvent &touchEvent) override;
    void onPotTouchDown(const PotEvent &potEvent) override;
    void onPotChange(const PotEvent &potEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;

    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

    void paint(Graphics &g) override;

private:
    /**
     * @brief Hands a knob touch to whichever callback subscribed to it.
     *
     * @return true when the script has taken the gesture over, and the
     *         control's own handling of it must not run
     */
    bool handlePotTouch(const PotEvent &potEvent, LuaPotEvent event);
};
