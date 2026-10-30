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
 * @file DetailDx7Env.h
 *
 * @brief Implements a wrapper component for Dx7-style envelope detail Control.
 */

#pragma once

#include "Detail.h"
#include "ControlComponent.h"
#include "KnobControl.h"

class DetailDx7Env : public Detail
{
public:
    DetailDx7Env(const Control &control, MainDelegate &newDelegate);
    virtual ~DetailDx7Env();

    void paint(Graphics &g) override;
    void resized() override;

private:
    ControlComponent *l1Knob;
    Control l1;
    ControlComponent *l2Knob;
    Control l2;
    ControlComponent *l3Knob;
    Control l3;
    ControlComponent *l4Knob;
    Control l4;
    ControlComponent *r1Knob;
    Control r1;
    ControlComponent *r2Knob;
    Control r2;
    ControlComponent *r3Knob;
    Control r3;
    ControlComponent *r4Knob;
    Control r4;

    std::vector<ControlValue> l1Values;
    std::vector<ControlValue> l2Values;
    std::vector<ControlValue> l3Values;
    std::vector<ControlValue> l4Values;
    std::vector<ControlValue> r1Values;
    std::vector<ControlValue> r2Values;
    std::vector<ControlValue> r3Values;
    std::vector<ControlValue> r4Values;
};
