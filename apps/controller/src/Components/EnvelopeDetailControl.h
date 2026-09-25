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
 * @file EnvelopeDetailControl.h
 *
 * @brief An envelope Control drawn inside a detail window.
 */

#pragma once

#include "EnvelopeControl.h"

/**
 * @brief The detail window's version of an EnvelopeControl.
 *
 * There were three of these too, one per shape, and the diff between any two
 * was their names. What actually distinguishes a detail from a control on a
 * page is three things: it takes the whole window rather than leaving room for
 * a name, it draws no name and no active underline, and letting go closes it.
 *
 * The mk2's can also show other envelopes over its own, in their own colours.
 * That is not here.
 */
class EnvelopeDetailControl : public EnvelopeControl
{
public:
    EnvelopeDetailControl(const Control &control,
                          MainDelegate &newDelegate,
                          const EnvelopeShape &newShape);
    virtual ~EnvelopeDetailControl() = default;

    void onTouchUp(const TouchEvent &touchEvent) override;
    void onTouchLongHold(const TouchEvent &touchEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;

protected:
    uint16_t getContourHeight(void) const override;
    void paintDecorations(Graphics &g) override;
};
