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

namespace
{
    /* The strip down the right of the window that closes the detail when it
     * is touched. */
    constexpr uint16_t dimmedWidth = 120;
} // namespace

DetailXyPad::DetailXyPad(const Control &control, MainDelegate &newDelegate)
    : Detail(control, newDelegate), xKnob(nullptr), yKnob(nullptr)
{
    xKnob = setUpKnob("X", XyPadControl::xHandle, 1);
    yKnob = setUpKnob("Y", XyPadControl::yHandle, 2);

    Detail::setLocked();
}

void DetailXyPad::paint(Graphics &g)
{
    const uint16_t frameWidth = (getWidth() > dimmedWidth)
                                    ? (uint16_t)(getWidth() - dimmedWidth)
                                    : getWidth();

    g.setColour(Colours565::black);
    g.fillRect(0, 0, frameWidth, getHeight());
    g.setColour(Colours565::darkgrey);
    g.drawRect(0, 0, frameWidth, getHeight());
    g.dim(frameWidth, 0, dimmedWidth, getHeight(), Colours565::black);
}

void DetailXyPad::resized(void)
{
    /* A square surface, as tall as the room above the knobs allows: an XY pad
     * whose two axes are different lengths reads as a different gesture in
     * each direction. The frame is the window less the dimmed strip, and the
     * pad and the two dials are all centred on it - the dials used to sit at
     * two numbers that were not either side of anything. */
    constexpr uint16_t top = 30;
    constexpr uint16_t knobRoom = 130;
    constexpr uint16_t knobWidth = 80;
    constexpr uint16_t knobHeight = 100;
    constexpr uint16_t knobSpread = 150;

    const uint16_t frameWidth = (getWidth() > dimmedWidth)
                                    ? (uint16_t)(getWidth() - dimmedWidth)
                                    : getWidth();
    const uint16_t centreX = frameWidth / 2;
    const uint16_t side = (getHeight() > (top + knobRoom))
                              ? (uint16_t)(getHeight() - top - knobRoom)
                              : getHeight();

    component->setBounds(centreX - (side / 2), top, side, side);
    lock->setBounds(getWidth() - 100, 25, 75, 75);
    dimmedSection->setBounds(frameWidth, 100, dimmedWidth, getHeight() - 100);

    resizeKnob(xKnob,
               centreX - knobSpread - (knobWidth / 2),
               getHeight() - knobRoom,
               knobWidth,
               knobHeight);
    resizeKnob(yKnob,
               centreX + knobSpread - (knobWidth / 2),
               getHeight() - knobRoom,
               knobWidth,
               knobHeight);
}
