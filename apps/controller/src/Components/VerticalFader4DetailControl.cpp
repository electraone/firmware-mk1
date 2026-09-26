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

#include "VerticalFader4DetailControl.h"

VerticalFader4DetailControl::VerticalFader4DetailControl(
    const Control &control,
    MainDelegate &newDelegate)
    : VerticalFader4Control(control, newDelegate)
{
}

/*
 * Neither of these calls up to VerticalFader4Control. Letting go of a detail
 * closes it, and the control is about to be destroyed - hiding its outline on
 * the way out would only queue a repaint of a window that is going away.
 */
void VerticalFader4DetailControl::onTouchUp(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

void VerticalFader4DetailControl::onPotTouchUp(
    [[maybe_unused]] const PotEvent &potEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

/* A long hold on a page control opens the detail. Inside the detail there is
 * nothing to open. */
void VerticalFader4DetailControl::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
}

/* The whole height: a component fills its bounds, and the window decides how
 * tall the bars are. */
uint16_t VerticalFader4DetailControl::getBarsHeight(void) const
{
    return (getHeight());
}

/* A detail has no name and no underline to draw. */
void VerticalFader4DetailControl::paintDecorations([[maybe_unused]] Graphics &g)
{
}
