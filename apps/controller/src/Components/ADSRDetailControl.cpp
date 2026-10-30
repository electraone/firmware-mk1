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

#include "ADSRDetailControl.h"
#include "LookAndFeel.h"

ADSRDetailControl::ADSRDetailControl(const Control &control,
                                     MainDelegate &newDelegate)
    : ADSRControl(control, newDelegate)
{
}

void ADSRDetailControl::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

void ADSRDetailControl::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
}

void ADSRDetailControl::onPotTouchUp([[maybe_unused]] const PotEvent &potEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

void ADSRDetailControl::paint(Graphics &g)
{
    Rectangle envBounds = getBounds();
    envBounds.setHeight(envBounds.getHeight());
    computePoints(envBounds);
    g.fillAll(LookAndFeel::backgroundColour);
    LookAndFeel::paintEnvelope(g,
                               envBounds,
                               control.getColour565(),
                               baselineY,
                               points,
                               activeSegment,
                               activeSegmentIsShown);
}
