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

#include "DetailEnvelope.h"

namespace
{
    /* The strip down the right of the window that closes the detail when it
     * is touched. */
    constexpr uint16_t dimmedWidth = 120;

    constexpr uint16_t contourTop = 50;
    constexpr uint16_t contourSideMargin = 50;
    constexpr uint16_t knobWidth = 80;
    constexpr uint16_t knobHeight = 100;
    constexpr uint16_t knobRowHeight = 115;
    constexpr uint16_t knobRoom = 130;

    /** Between the contour's baseline and the top of the dials. Without it
     *  the two touch, and the envelope looks like it is standing on them. */
    constexpr uint16_t contourBottomGap = 18;

    /** More than this many dials and the row is split in two. */
    constexpr uint8_t maxInOneRow = 6;
} // namespace

DetailEnvelope::DetailEnvelope(const Control &control,
                               MainDelegate &newDelegate)
    : Detail(control, newDelegate)
{
    for (uint8_t handle = 0; handle < control.values.size(); handle++) {
        /* One knob per declared value, on the knob of the same number. A
         * preset declaring fewer values than its type implies gets fewer
         * dials rather than dials on values that are not there. */
        knobs.push_back(
            setUpKnob(control.translateValueId(handle), handle, handle + 1));
    }

    Detail::setLocked();
}

uint8_t DetailEnvelope::numColumns(void) const
{
    const uint8_t count = (uint8_t)knobs.size();

    if (count == 0) {
        return (1);
    }

    return ((count <= maxInOneRow) ? count : (uint8_t)((count + 1) / 2));
}

uint8_t DetailEnvelope::numRows(void) const
{
    return ((knobs.size() <= maxInOneRow) ? 1 : 2);
}

void DetailEnvelope::paint(Graphics &g)
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

/*
 * The contour across the top of the frame, and under it the dials, spread
 * evenly across the same width so the two read as one drawing. A two row
 * layout pairs them by column - value 2c above value 2c + 1 - which is what
 * puts a DX7 envelope's four levels over its four rates.
 */
void DetailEnvelope::resized(void)
{
    const uint16_t frameWidth = (getWidth() > dimmedWidth)
                                    ? (uint16_t)(getWidth() - dimmedWidth)
                                    : getWidth();
    const uint16_t contourWidth = (frameWidth > (2 * contourSideMargin))
                                      ? frameWidth - (2 * contourSideMargin)
                                      : frameWidth;
    const uint16_t rows = numRows();
    const uint16_t knobsRoom = knobRoom + ((rows - 1) * knobRowHeight);
    const uint16_t contourRoom =
        (getHeight() > (contourTop + knobsRoom))
            ? (uint16_t)(getHeight() - contourTop - knobsRoom)
            : getHeight();
    const uint16_t contourHeight =
        (contourRoom > contourBottomGap)
            ? (uint16_t)(contourRoom - contourBottomGap)
            : contourRoom;

    component->setBounds((frameWidth - contourWidth) / 2,
                         contourTop,
                         contourWidth,
                         contourHeight);
    lock->setBounds(getWidth() - 100, 25, 75, 75);
    dimmedSection->setBounds(frameWidth, 100, dimmedWidth, getHeight() - 100);

    const uint16_t columns = numColumns();
    const uint16_t slot = contourWidth / columns;
    const uint16_t left = (frameWidth - contourWidth) / 2;

    for (uint8_t index = 0; index < knobs.size(); index++) {
        const uint8_t column = (rows == 1) ? index : (uint8_t)(index / 2);
        const uint8_t row = (rows == 1) ? 0 : (uint8_t)(index % 2);
        const uint16_t centreX = left + (column * slot) + (slot / 2);

        resizeKnob(knobs[index],
                   centreX - (knobWidth / 2),
                   getHeight() - knobRoom - ((rows - 1 - row) * knobRowHeight),
                   knobWidth,
                   knobHeight);
    }
}
