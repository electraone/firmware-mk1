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
 * @file DetailVerticalFader4.h
 *
 * @brief The window a four-fader control opens: the bars, and a dial under
 *        each of them.
 */

#pragma once

#include "Detail.h"
#include "ControlComponent.h"
#include "VerticalFader4Control.h"

class DetailVerticalFader4 : public Detail
{
public:
    DetailVerticalFader4(const Control &control, MainDelegate &newDelegate);
    virtual ~DetailVerticalFader4() = default;

    void paint(Graphics &g) override;
    void resized(void) override;

private:
    ControlComponent *faderKnobs[VerticalFader4Control::numHandles];
};
