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

#include "XyPadDetailControl.h"
#include "LookAndFeel.h"

XyPadDetailControl::XyPadDetailControl(const Control &control,
                                       MainDelegate &newDelegate)
    : XyPadControl(control, newDelegate)
{
}

/* A finger on the detail's pad puts the handle where it lands, rather than
 * stepping the value: the whole point of the big pad is that it is an
 * absolute surface. */
void XyPadDetailControl::onTouchDown(const TouchEvent &touchEvent)
{
    onTouchMove(touchEvent);
}

void XyPadDetailControl::onTouchMove(const TouchEvent &touchEvent)
{
    if ((getWidth() == 0) || (getHeight() == 0)) {
        return;
    }

    const ControlValue &xValue = control.getValue(xHandle);
    const int16_t xMin = xValue.getMin();
    const int16_t xMax = xValue.getMax();

    emitValueChange((int16_t)constrain(map((float)touchEvent.getX(),
                                           0.0f,
                                           (float)getWidth(),
                                           (float)xMin,
                                           (float)xMax),
                                       (float)xMin,
                                       (float)xMax),
                    xValue);

    /* Y grows upwards, from the bottom edge: a finger moving up raises it,
     * and the top of the pad is the maximum. */
    if (control.values.size() <= yHandle) {
        return;
    }

    const ControlValue &yValue = control.getValue(yHandle);
    const int16_t yMin = yValue.getMin();
    const int16_t yMax = yValue.getMax();
    const int16_t fromBottom = getHeight() - touchEvent.getY();

    emitValueChange((int16_t)constrain(map((float)fromBottom,
                                           0.0f,
                                           (float)getHeight(),
                                           (float)yMin,
                                           (float)yMax),
                                       (float)yMin,
                                       (float)yMax),
                    yValue);
}

void XyPadDetailControl::onTouchUp(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
}

/* A long hold on a page control opens the detail. Inside the detail there is
 * nothing to open. */
void XyPadDetailControl::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
}

void XyPadDetailControl::onPotTouchUp(const PotEvent &potEvent)
{
    /* The pad stays put: a knob is how the detail is meant to be used, and
     * letting go of one would close the window under the finger. */
    ControlComponent::onPotTouchUp(potEvent);
}

void XyPadDetailControl::paint(Graphics &g)
{
    const uint32_t highlightedColour = control.getColour565();
    const uint32_t colour = Colours565::darker(highlightedColour, 0.6f);
    const Rectangle padBounds = getLocalBounds();

    g.fillAll(LookAndFeel::backgroundColour);

    LookAndFeel::paintXyPadDetailGround(
        g, padBounds, Colours565::darker(colour, 0.3f));
    LookAndFeel::paintXyPadDetailCrosshairs(g,
                                            padBounds,
                                            colour,
                                            highlightedColour,
                                            handleX(getWidth()),
                                            handleY(getHeight()));
}
