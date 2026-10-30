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

#include "Dx7EnvDetailControl.h"
#include "LookAndFeel.h"

Dx7EnvDetailControl::Dx7EnvDetailControl(const Control &control,
                                         MainDelegate &newDelegate)
    : Dx7EnvControl(control, newDelegate)
{
}

void Dx7EnvDetailControl::onTouchUp(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

void Dx7EnvDetailControl::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    // No action for long hold
}

void Dx7EnvDetailControl::onPotTouchUp(
    [[maybe_unused]] const PotEvent &potEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

void Dx7EnvDetailControl::paint(Graphics &g)
{
    Rectangle envBounds = getBounds();
    envBounds.setHeight(envBounds.getHeight());
    computePoints(envBounds);
    g.fillAll(LookAndFeel::backgroundColour);
    LookAndFeel::paintEnvelope(
        g, envBounds, control.getColour565(), baselineY, points, 0, false);
}
