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
    assignPot(control.inputs[0].getPotId(), control.values[0].getNumSteps());
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

    g.fillAll(Colours565::black);

    uint32_t colourTrack = Colours565::darker(colour, 0.3f);
    uint32_t backgroundColour = getUseAltBackground()
                                    ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour;

    uint16_t barHeight = bounds.getHeight();
    uint16_t padding = 0;
    uint16_t barX = 0;

    // For zero based faders, find the start position of the bar
    if (mode == Control::Mode::Bipolar) {
        barX = map(std::max((int16_t)0, min), min, max, 0, bounds.getWidth());
    }

    uint16_t barWidth = map(val, min, max, 0, bounds.getWidth()) - barX;

    // Clear the component area
    g.fillAll(backgroundColour);

    // Paint the track background
    g.setColour(colourTrack);
    g.fillRect(0, padding, bounds.getWidth(), barHeight);

    // Paint the active bar
    g.setColour(colour);
    g.fillRect(barX, padding, barWidth, barHeight);

    if (barWidth > 1) {
        g.setColour(Colours565::black);
        uint16_t separatorX =
            ((val > 0) ? barX + barWidth : barX + barWidth - 1);
        g.drawLine(separatorX, padding, separatorX, padding + barHeight - 1);
    }
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
