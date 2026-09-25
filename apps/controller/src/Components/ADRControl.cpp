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

#include "ADRControl.h"
#include "LookAndFeel.h"

ADRControl::ADRControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate)
{
    setValueRanges();
    setActiveSegment(control.inputs[0].getValueId());
    updateValueFromParameterMap();
}

void ADRControl::syncComponentProperties(void)
{
    setValueRanges();
    ControlComponent::syncComponentProperties();
}

void ADRControl::onTouchDown(const TouchEvent &touchEvent)
{
    previousScreenX = touchEvent.getScreenX();
}

void ADRControl::onTouchMove([[maybe_unused]] const TouchEvent &touchEvent)
{
    // commented out on purpose
    /*
    uint8_t activeHandle = getActiveSegment();
    int16_t max = values[activeHandle].getMax();
    int16_t min = values[activeHandle].getMin();
    float step = getWidth() / (float)(max - min);
    int16_t delta = touchEvent.getScreenX() - previousScreenX;
    previousScreenX = touchEvent.getScreenX();

    int16_t newDisplayValue = constrain(
        ceil(getValue(activeHandle) + ((float)delta / step)), min, max);

    emitValueChange(newDisplayValue, control.getValue(activeHandle));
    */
}

void ADRControl::onPotTouchDown(const PotEvent &potEvent)
{
    showActiveSegment(true);
    ControlComponent::onPotTouchDown(potEvent);
}

void ADRControl::onPotChange(const PotEvent &potEvent)
{
    if (int16_t delta = potEvent.getAcceleratedChange()) {
        uint8_t activeHandle = getActiveSegment();
        const auto &cv = control.getValue(activeHandle);
        int16_t newDisplayValue = cv.isRelative()
                                      ? cv.calculateRelativeDelta(delta)
                                      : getValue(activeHandle) + delta;
        emitValueChange(newDisplayValue, cv);
    }
}

void ADRControl::onPotTouchUp(const PotEvent &potEvent)
{
    showActiveSegment(false);
    ControlComponent::onPotTouchUp(potEvent);
}

void ADRControl::onMidiValueChange(const ControlValue &value,
                                   int16_t midiValue,
                                   uint8_t handle)
{
    int16_t newDisplayValue = value.translateMidiValue(midiValue);
    if (handle <= 2) {
        setValue(handle, newDisplayValue);
    }
}

void ADRControl::paint(Graphics &g)
{
    Rectangle envBounds = getBounds();
    envBounds.setHeight(envBounds.getHeight() / 2);
    computePoints(envBounds);
    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);
    LookAndFeel::paintEnvelope(g,
                               envBounds,
                               control.getColour565(),
                               baselineY,
                               points,
                               activeSegment,
                               activeSegmentIsShown);

    g.printText(0,
                getHeight() - 20,
                getName(),
                TextStyle::mediumWhiteOnBlack,
                getWidth(),
                TextAlign::center,
                2);
    ControlComponent::paint(g);
}

void ADRControl::setValueRanges(void)
{
    setMin(ADR::attack, control.values[0].getMin());
    setMax(ADR::attack, control.values[0].getMax());
    setMin(ADR::decay, control.values[1].getMin());
    setMax(ADR::decay, control.values[1].getMax());
    setMin(ADR::release, control.values[2].getMin());
    setMax(ADR::release, control.values[2].getMax());
}
