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
 * @file Dx7EnvDetailControl.h
 *
 * @brief Implements an detail version of the Dx7-style envelope Control.
 */

#pragma once

#include "Dx7EnvControl.h"

class Dx7EnvDetailControl : public Dx7EnvControl
{
public:
    Dx7EnvDetailControl(const Control &control, MainDelegate &newDelegate);
    virtual ~Dx7EnvDetailControl() = default;

    void onTouchUp(const TouchEvent &touchEvent) override;
    void onTouchLongHold(const TouchEvent &touchEvent);
    void onPotTouchUp(const PotEvent &potEvent) override;
    void paint(Graphics &g) override;
};
