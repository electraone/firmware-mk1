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

#include "DetailVerticalFader4.h"
#include "VerticalFader4Control.h"

namespace
{
    /* The knobs sit where an envelope detail's do, so the two windows read as
     * the same window with a different graph in it. */
    constexpr uint16_t knobX[] = { 63, 228, 394, 560 };
    constexpr uint16_t knobWidth = 80;
    constexpr uint16_t knobHeight = 100;
    constexpr uint16_t knobBottomInset = 130;
} // namespace

DetailVerticalFader4::DetailVerticalFader4(const Control &control,
                                           MainDelegate &newDelegate)
    : Detail(control, newDelegate), faderKnobs{}
{
    for (uint8_t handle = 0; handle < VerticalFader4Control::numHandles;
         handle++) {
        faderKnobs[handle] =
            setUpKnob(control.translateValueId(handle), handle, handle + 1);
    }

    Detail::setLocked();
}

void DetailVerticalFader4::paint(Graphics &g)
{
    g.setColour(Colours565::black);
    g.fillRect(0, 0, getWidth() - 120, getHeight());
    g.setColour(Colours565::darkgrey);
    g.drawRect(0, 0, getWidth() - 120, getHeight());
    g.dim(getWidth() - 120, 0, 120, getHeight(), Colours565::black);
}

void DetailVerticalFader4::resized(void)
{
    component->setBounds(50, 50, 608, 150);
    lock->setBounds(getWidth() - 100, 25, 75, 75);
    dimmedSection->setBounds(getWidth() - 120, 100, 120, getHeight() - 100);

    for (uint8_t handle = 0; handle < VerticalFader4Control::numHandles;
         handle++) {
        resizeKnob(faderKnobs[handle],
                   knobX[handle],
                   getHeight() - knobBottomInset,
                   knobWidth,
                   knobHeight);
    }
}
