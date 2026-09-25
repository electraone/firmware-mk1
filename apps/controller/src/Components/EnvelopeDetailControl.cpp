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

#include "EnvelopeDetailControl.h"

EnvelopeDetailControl::EnvelopeDetailControl(const Control &control,
                                             MainDelegate &newDelegate,
                                             const EnvelopeShape &newShape)
    : EnvelopeControl(control, newDelegate, newShape)
{
}

/*
 * Neither of these calls up to EnvelopeControl. Letting go of a detail closes
 * it, and the control is about to be destroyed - hiding its segment marker on
 * the way out would only queue a repaint of a window that is going away.
 */
void EnvelopeDetailControl::onTouchUp(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

void EnvelopeDetailControl::onPotTouchUp(
    [[maybe_unused]] const PotEvent &potEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

/* A long hold on a page control opens the detail. Inside the detail there is
 * nothing to open. */
void EnvelopeDetailControl::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
}

uint16_t EnvelopeDetailControl::getContourHeight(void) const
{
    return (getHeight());
}

/* A detail has no name and no underline to draw. */
void EnvelopeDetailControl::paintDecorations([[maybe_unused]] Graphics &g)
{
}
