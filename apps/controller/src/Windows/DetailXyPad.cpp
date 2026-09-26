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

#include "DetailXyPad.h"
#include "XyPadControl.h"

DetailXyPad::DetailXyPad(const Control &control, MainDelegate &newDelegate)
    : Detail(control, newDelegate), xKnob(nullptr), yKnob(nullptr)
{
    xKnob = setUpKnob("X", XyPadControl::xHandle, 1);
    yKnob = setUpKnob("Y", XyPadControl::yHandle, 2);

    Detail::setLocked();
}

void DetailXyPad::paint(Graphics &g)
{
    g.setColour(Colours565::black);
    g.fillRect(0, 0, getWidth() - 120, getHeight());
    g.setColour(Colours565::darkgrey);
    g.drawRect(0, 0, getWidth() - 120, getHeight());
    g.dim(getWidth() - 120, 0, 120, getHeight(), Colours565::black);
}

void DetailXyPad::resized(void)
{
    /* A square surface, as tall as the room above the knobs allows, centred
     * in the window: an XY pad whose two axes are different lengths reads as
     * a different gesture in each direction. */
    constexpr uint16_t top = 30;
    constexpr uint16_t knobRoom = 130;
    const uint16_t side = (getHeight() > (top + knobRoom))
                              ? (uint16_t)(getHeight() - top - knobRoom)
                              : getHeight();

    component->setBounds((getWidth() - 120 - side) / 2, top, side, side);
    lock->setBounds(getWidth() - 100, 25, 75, 75);
    dimmedSection->setBounds(getWidth() - 120, 100, 120, getHeight() - 100);

    resizeKnob(xKnob, 200, getHeight() - knobRoom, 80, 100);
    resizeKnob(yKnob, 500, getHeight() - knobRoom, 80, 100);
}
