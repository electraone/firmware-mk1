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
 * @file DX7EnvControl.h
 *
 * @brief Implements an on-screen DX7-style envelope Control.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"
#include "Env5Seg.h"

class Dx7EnvControl : public ControlComponent, public Env5Seg
{
public:
    Dx7EnvControl(const Control &control, MainDelegate &newDelegate);
    virtual ~Dx7EnvControl() = default;

    void syncComponentProperties(void) override;

    virtual void onTouchDown(const TouchEvent &touchEvent) override;
    void onTouchMove(const TouchEvent &touchEvent) override;
    void onPotTouchDown(const PotEvent &potEvent) override;
    void onPotChange(const PotEvent &potEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;
    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

    void paint(Graphics &g) override;

private:
    void setValueRanges(void);

    uint16_t previousScreenX;
};
