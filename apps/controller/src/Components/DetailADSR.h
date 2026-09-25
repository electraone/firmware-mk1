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
 * @file DetailADSR.h
 *
 * @brief Implements a wrapper component for ADSR detail Control.
 */

#pragma once

#include "Detail.h"
#include "ControlComponent.h"
#include "KnobControl.h"

class DetailADSR : public Detail
{
public:
    DetailADSR(const Control &control, MainDelegate &newDelegate);
    virtual ~DetailADSR();

    void paint(Graphics &g) override;
    void resized() override;

private:
    ControlComponent *attackKnob;
    Control attack;
    ControlComponent *decayKnob;
    Control decay;
    ControlComponent *sustainKnob;
    Control sustain;
    ControlComponent *releaseKnob;
    Control release;

    std::vector<ControlValue> attackValues;
    std::vector<ControlValue> decayValues;
    std::vector<ControlValue> sustainValues;
    std::vector<ControlValue> releaseValues;
};
