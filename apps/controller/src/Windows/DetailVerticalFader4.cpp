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
    /* The strip down the right of the window that closes the detail when it
     * is touched. */
    constexpr uint16_t dimmedWidth = 120;

    constexpr uint16_t barsTop = 40;
    constexpr uint16_t knobRoom = 130;
    constexpr uint16_t barsBottomGap = 20;
    constexpr uint16_t knobWidth = 80;
    constexpr uint16_t knobHeight = 100;
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
 * The bars fill the room above the knobs, and each knob is placed under the
 * bar it turns - asked of the component rather than worked out again here,
 * which is how the two came to disagree: the knobs sat at four numbers that
 * looked about right and were forty pixels left of their bars.
 */
void DetailVerticalFader4::resized(void)
{
    const uint16_t frameWidth = (getWidth() > dimmedWidth)
                                    ? (uint16_t)(getWidth() - dimmedWidth)
                                    : getWidth();
    const uint16_t barsRoom = (getHeight() > (barsTop + knobRoom))
                                  ? (uint16_t)(getHeight() - barsTop - knobRoom)
                                  : getHeight();
    const uint16_t barsHeight = (barsRoom > barsBottomGap)
                                    ? (uint16_t)(barsRoom - barsBottomGap)
                                    : barsRoom;
    const uint16_t barsWidth =
        (frameWidth > 80) ? (frameWidth - 80) : frameWidth;

    component->setBounds(
        (frameWidth - barsWidth) / 2, barsTop, barsWidth, barsHeight);
    lock->setBounds(getWidth() - 100, 25, 75, 75);
    dimmedSection->setBounds(frameWidth, 100, dimmedWidth, getHeight() - 100);

    const auto *bars = dynamic_cast<const VerticalFader4Control *>(component);

    if (!bars) {
        return;
    }

    for (uint8_t handle = 0; handle < VerticalFader4Control::numHandles;
         handle++) {
        resizeKnob(faderKnobs[handle],
                   component->getX() + bars->barCentreX(handle)
                       - (knobWidth / 2),
                   getHeight() - knobRoom,
                   knobWidth,
                   knobHeight);
    }
}
