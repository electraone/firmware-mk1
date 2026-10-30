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
 * @file FaderDetailControl.h
 *
 * @brief Implements an detail version of the Fader Control.
 */

#pragma once

#include "FaderControl.h"

class FaderDetailControl : public FaderControl
{
public:
    FaderDetailControl(const Control &control, MainDelegate &newDelegate);
    virtual ~FaderDetailControl() = default;

    void onTouchMove(const TouchEvent &touchEvent) override;
    void paint(Graphics &g) override;
    void onTouchUp(const TouchEvent &touchEvent) override;
    void onTouchLongHold(const TouchEvent &touchEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;

private:
    uint16_t previousScreenX;
};
