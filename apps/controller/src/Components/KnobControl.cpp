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

#include "KnobControl.h"

KnobControl::KnobControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate)
{
    setMinimum(control.values[0].getMin());
    setMaximum(control.values[0].getMax());
}

void KnobControl::onTouchMove(const TouchEvent &touchEvent)
{
    int16_t max = value.getMax();
    int16_t min = value.getMin();

    float step = getWidth() / (float)(max - min);
    int16_t newDisplayValue =
        constrain(ceil(touchEvent.getX() / step + min), min, max);

    emitValueChange(newDisplayValue, control.getValue(0));
}

void KnobControl::onPotTouchDown(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchDown(potEvent);
}

void KnobControl::onPotChange(const PotEvent &potEvent)
{
    if (int16_t delta = potEvent.getAcceleratedChange()) {
        const auto &cv = control.getValue(0);
        int16_t newDisplayValue = cv.isRelative()
                                      ? cv.calculateRelativeDelta(delta)
                                      : getValue() + delta;
        emitValueChange(newDisplayValue, cv);
    }
}

void KnobControl::onPotTouchUp(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchUp(potEvent);
}

void KnobControl::onMidiValueChange(const ControlValue &value,
                                    int16_t midiValue,
                                    [[maybe_unused]] uint8_t handle)
{
    int16_t newDisplayValue = value.translateMidiValue(midiValue);
    setValue(newDisplayValue);
}

void KnobControl::paint(Graphics &g)
{
    Rectangle knobBounds = getBounds();
    colour = control.getColour565();
    Knob::paint(g);

    char stringValue[ControlValue::MaxLabelLength + 1];

    if (control.getValue(0).isLabelSet()) {
        copyString(stringValue,
                   control.getValue(0).getLabel(),
                   ControlValue::MaxLabelLength);
    } else {
        snprintf(stringValue,
                 ControlValue::MaxLabelLength,
                 formatString,
                 getValue());
    }

    g.printText(0,
                getWidth() / 2 - 7,
                stringValue,
                TextStyle::mediumTransparent,
                getWidth(),
                TextAlign::center);

    g.printText(0,
                getHeight() - 20,
                getName(),
                TextStyle::mediumWhiteOnBlack,
                getWidth(),
                TextAlign::center,
                2);
}
