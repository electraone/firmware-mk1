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

#include "PadControl.h"

PadControl::PadControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate), state(false)
{
    updateValueFromParameterMap();
}

PadControl::~PadControl()
{
    if ((control.getMode() == Control::Mode::Momentary)) {
        const ControlValue cv = control.getValue(0);

        parameterMap.setValueSimple(cv.message.getDeviceId(),
                                    cv.message.getType(),
                                    cv.message.getParameterNumber(),
                                    cv.message.getOffValue());
    }
}

void PadControl::onTouchDown([[maybe_unused]] const TouchEvent &touchEvent)
{
    const ControlValue cv = control.getValue(0);

    if (cv.isRelative()) {
        int16_t delta = 1;

        if (control.getMode() == Control::Mode::Toggle) {
            delta = (state == false) ? 1 : -1;
        }

        ControlComponent::emitValueChange(delta, cv);
    } else {
        uint16_t midiValue = cv.message.getOnValue();

        if (control.getMode() == Control::Mode::Toggle) {
            midiValue = (state == false) ? cv.message.getOnValue()
                                         : cv.message.getOffValue();
            setState(!state);
        }

        parameterMap.setValue(cv.message.getDeviceId(),
                              cv.message.getType(),
                              cv.message.getParameterNumber(),
                              midiValue,
                              Origin::internal);
    }
}

void PadControl::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
    if (control.getMode() == Control::Mode::Momentary) {
        emitOffValue();
    }
}

void PadControl::onPotTouchDown(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchDown(potEvent);
}

void PadControl::onPotChange(const PotEvent &potEvent)
{
    // @todo absolute and relative controls should have their own classes
    int16_t delta = potEvent.getAcceleratedChange();
    const auto &cv = control.getValue(0);

    if (cv.isRelative()) {
        if (delta != 0) {
            ControlComponent::emitValueChange(delta, cv);
        }
    } else {
        int8_t step = ControlValue::invertedSign(delta);

        if (step < 0 && state != true) {
            parameterMap.setValue(cv.message.getDeviceId(),
                                  cv.message.getType(),
                                  cv.message.getParameterNumber(),
                                  cv.message.getOnValue(),
                                  Origin::internal);
        } else if (step > 0 && state != false) {
            parameterMap.setValue(cv.message.getDeviceId(),
                                  cv.message.getType(),
                                  cv.message.getParameterNumber(),
                                  cv.message.getOffValue(),
                                  Origin::internal);
        }
    }
}

void PadControl::onPotTouchUp(const PotEvent &potEvent)
{
    if (control.getMode() == Control::Mode::Momentary) {
        emitOffValue();
    }
    ControlComponent::onPotTouchUp(potEvent);
}

void PadControl::onMidiValueChange(const ControlValue &value,
                                   int16_t midiValue,
                                   [[maybe_unused]] uint8_t handle)
{
    if (value.translateMidiValue(midiValue)) {
        setState(true);
    } else {
        setState(false);
    }
}

void PadControl::paint(Graphics &g)
{
    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);
    auto bounds = getBounds();
    auto isMomentary = (control.getMode() == Control::Mode::Momentary);
    bounds.setX(0);
    bounds.setY(3);
    bounds.setWidth(getWidth());
    bounds.setHeight(getHeight() - 6);
    LookAndFeel::paintPad(
        g, bounds, control.getColour565(), isMomentary, getState());

    g.setColour(Colours565::white);
    char stringValue[ControlValue::MaxLabelLength + 1];
    if (control.getValue(0).isLabelSet()) {
        copyString(stringValue,
                   control.getValue(0).getLabel(),
                   ControlValue::MaxLabelLength);
    } else {
        snprintf(stringValue, ControlValue::MaxLabelLength, "%s", getName());
    }

    g.print(0,
            getHeight() / 2 - 10,
            stringValue,
            bounds.getWidth(),
            TextAlign::center);

    ControlComponent::paint(g);
}

void PadControl::emitValueChange([[maybe_unused]] int16_t newDisplayValue,
                                 [[maybe_unused]] const ControlValue &cv)
{
    // left empty on purpose. The logic is handled in the Pad component
}

void PadControl::onTouchLongHold([[maybe_unused]] const TouchEvent &touchEvent)
{
    // left empty on purpose. It overrides the long hold of ControlComponent
    // which defaults to displaying the detail.
}

bool PadControl::getState() const
{
    return state;
}
void PadControl::setState(bool newState)
{
    if (state != newState) {
        state = newState;
        repaint();
    }
}

void PadControl::emitOffValue(void)
{
    const ControlValue cv = control.getValue(0);

    parameterMap.setValue(cv.message.getDeviceId(),
                          cv.message.getType(),
                          cv.message.getParameterNumber(),
                          cv.message.getOffValue(),
                          Origin::internal);
}