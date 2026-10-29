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

#include "FaderDetailControl.h"

FaderDetailControl::FaderDetailControl(const Control &control,
                                       MainDelegate &newDelegate)
    : FaderControl(control, newDelegate), previousScreenX(0)
{
    assignPot(control.inputs[0].getPotId(), control.values[0].getPotSteps());
    setColour(control.getColour565());
}

void FaderDetailControl::onTouchMove(const TouchEvent &touchEvent)
{
    const auto &cv = control.getValue(0);

    if (cv.isRelative()) {
        int16_t delta = touchEvent.getScreenX() - previousScreenX;
        previousScreenX = touchEvent.getScreenX();
        emitValueChange(ControlValue::sign(delta), cv);
    } else {
        int16_t max = value.getMax();
        int16_t min = value.getMin();
        float step = getWidth() / (float)(max - min);
        int16_t newDisplayValue =
            constrain(ceil(touchEvent.getX() / step + min), min, max);

        emitValueChange(newDisplayValue, cv);
    }
}

void FaderDetailControl::paint(Graphics &g)
{
    const Rectangle &bounds = getBounds();
    int16_t val = value.get();
    int16_t min = value.getMin();
    int16_t max = value.getMax();
    Control::Mode mode = control.getMode();

    uint32_t backgroundColour = getUseAltBackground()
                                    ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour;

    int16_t barX = 0;

    // For zero based faders, find the start position of the bar
    if (mode == Control::Mode::Bipolar) {
        barX = map(std::max((int16_t)0, min), min, max, 0, bounds.getWidth());
    }

    /* Signed, as on a page: a bipolar fader below its zero has moved the end
     * of the bar to the left of where it starts. */
    const int16_t barWidth =
        (int16_t)(map(val, min, max, 0, bounds.getWidth()) - barX);

    g.fillAll(backgroundColour);

    /* The same track a fader on a page draws, at the height of the detail:
     * the same rounded ends, the same mark where a bipolar fader sits on its
     * zero, and the same bar running leftwards below it. The black line that
     * used to be drawn at the end of the bar is gone with them - it was there
     * because the bar had no end of its own to see. */
    LookAndFeel::paintFaderTrack(
        g, bounds, colour, 0, bounds.getHeight(), barX, barWidth);
}

void FaderDetailControl::onTouchUp(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

void FaderDetailControl::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    // No action in this case, function provided for potential future use.
}

void FaderDetailControl::onPotTouchUp([[maybe_unused]] const PotEvent &potEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}
