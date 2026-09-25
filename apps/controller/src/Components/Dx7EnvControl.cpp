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

#include "Dx7EnvControl.h"
#include "LookAndFeel.h"

Dx7EnvControl::Dx7EnvControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate)
{
    setValueRanges();
    setActiveSegment(control.inputs[0].getValueId());
    updateValueFromParameterMap();
}

void Dx7EnvControl::syncComponentProperties(void)
{
    setValueRanges();
    ControlComponent::syncComponentProperties();
}

void Dx7EnvControl::onTouchDown(const TouchEvent &touchEvent)
{
    previousScreenX = touchEvent.getScreenX();
}

void Dx7EnvControl::onTouchMove(const TouchEvent &touchEvent)
{
    // Code for handling touch move goes here
    uint8_t activeHandle = getActiveSegment();
    int16_t max = values[activeHandle].getMax();
    int16_t min = values[activeHandle].getMin();
    float step = getWidth() / (float)(max - min);
    int16_t delta = touchEvent.getScreenX() - previousScreenX;
    previousScreenX = touchEvent.getScreenX();

    int16_t newDisplayValue = constrain(
        ceil(getValue(activeHandle) + ((float)delta / step)), min, max);

    emitValueChange(newDisplayValue, control.getValue(activeHandle));
}

void Dx7EnvControl::onPotTouchDown(const PotEvent &potEvent)
{
    showActiveSegment(true);
    ControlComponent::onPotTouchDown(potEvent);
}

void Dx7EnvControl::onPotChange(const PotEvent &potEvent)
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

void Dx7EnvControl::onPotTouchUp(const PotEvent &potEvent)
{
    showActiveSegment(false);
    ControlComponent::onPotTouchUp(potEvent);
}

void Dx7EnvControl::onMidiValueChange(const ControlValue &value,
                                      int16_t midiValue,
                                      uint8_t handle)
{
    int16_t newDisplayValue = value.translateMidiValue(midiValue);
    if (handle <= 7) {
        if (handle % 2) {
            newDisplayValue = value.getMax() - newDisplayValue;
        }
        setValue(handle, newDisplayValue);
    }
}

void Dx7EnvControl::paint(Graphics &g)
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

void Dx7EnvControl::setValueRanges(void)
{
    values[Env5Seg::level1].setMin(control.values[0].getMin());
    values[Env5Seg::level1].setMax(control.values[0].getMax());
    values[Env5Seg::rate1].setMin(control.values[1].getMin());
    values[Env5Seg::rate1].setMax(control.values[1].getMax());
    values[Env5Seg::level2].setMin(control.values[2].getMin());
    values[Env5Seg::level2].setMax(control.values[2].getMax());
    values[Env5Seg::rate2].setMin(control.values[3].getMin());
    values[Env5Seg::rate2].setMax(control.values[3].getMax());
    values[Env5Seg::level3].setMin(control.values[4].getMin());
    values[Env5Seg::level3].setMax(control.values[4].getMax());
    values[Env5Seg::rate3].setMin(control.values[5].getMin());
    values[Env5Seg::rate3].setMax(control.values[5].getMax());
    values[Env5Seg::level4].setMin(control.values[6].getMin());
    values[Env5Seg::level4].setMax(control.values[6].getMax());
    values[Env5Seg::rate4].setMin(control.values[7].getMin());
    values[Env5Seg::rate4].setMax(control.values[7].getMax());
}
