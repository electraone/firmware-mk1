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

#include "ControlComponent.h"
#include "FaderControl.h"
#include "ListControl.h"
#include "ListButtonControl.h"
#include "PadControl.h"
#include "ADSRControl.h"
#include "ADRControl.h"
#include "Dx7EnvControl.h"
#include "FaderDetailControl.h"
#include "ListDetailControl.h"
#include "ADSRDetailControl.h"
#include "ADRDetailControl.h"
#include "Dx7EnvDetailControl.h"
#include "KnobControl.h"
#include "RelativeControl.h"
#include "MainWindow.h"

ControlComponent::ControlComponent(const Control &controlToAssign,
                                   MainDelegate &newDelegate)
    : control(controlToAssign),
      delegate(newDelegate),
      useAltBackground(false),
      active(false)
{
}

void ControlComponent::paint(Graphics &g)
{
    if (active) {
        g.setColour(0x4228);
        g.fillRect(0, getHeight() - 2, getWidth(), 2);
    }
}

void ControlComponent::syncComponentProperties(void)
{
    auto bounds = control.getBounds();
    setBounds(bounds);

    setName(control.getName());
    setId(control.getId());
    setVisible(control.isVisible());
}

void ControlComponent::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    if (MainWindow *window = dynamic_cast<MainWindow *>(getWindow())) {
        window->openDetail(getId());
    }
    setActive(false);
}

void ControlComponent::onTouchDoubleClick(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    uint8_t handle = 0;

    if (Envelope *en = dynamic_cast<Envelope *>(this)) {
        handle = en->getActiveSegment();
    }

    delegate.setDefaultValue(getId(), handle);
}

void ControlComponent::onPotTouchDown(const PotEvent &potEvent)
{
    delegate.setActivePotTouch(potEvent.getPotId(), this);
    setActive(true);
    delegate.sendPotTouchEvent(potEvent.getPotId(), this->getId(), true);
}

void ControlComponent::onPotTouchUp(const PotEvent &potEvent)
{
    delegate.resetActivePotTouch(potEvent.getPotId());
    setActive(false);
    delegate.sendPotTouchEvent(potEvent.getPotId(), this->getId(), false);
}

void ControlComponent::updateValueFromParameterMap(void)
{
    for (const auto &value : control.values) {
        const auto &message = value.message;
        uint16_t midiValue =
            parameterMap.getValue(message.getDeviceId(),
                                  message.getType(),
                                  message.getParameterNumber());
        onMidiValueChange(value, midiValue, value.getHandle());
    }
}

void ControlComponent::emitValueChange(int16_t newDisplayValue,
                                       const ControlValue &cv)
{
    uint16_t midiValue = 0;

    Message::Type type = cv.message.getType();

    if ((type == Message::Type::relcc) || cv.isRelative()) {
        midiValue = calculateRelativeMidiValue(newDisplayValue, cv);
        parameterMap.setRelative(cv.message.getDeviceId(),
                                 type,
                                 cv.message.getParameterNumber(),
                                 midiValue,
                                 Origin::internal);
    } else {
        midiValue = calculateAbsoluteMidiValue(newDisplayValue, cv);
        parameterMap.setValue(cv.message.getDeviceId(),
                              type,
                              cv.message.getParameterNumber(),
                              midiValue,
                              Origin::internal);
    }

    System::logger.write(LOG_TRACE,
                         "emitValueChange: display=%d, midi=%d",
                         newDisplayValue,
                         midiValue);
}

void ControlComponent::setUseAltBackground(bool shouldUseAltBackground)
{
    useAltBackground = shouldUseAltBackground;
}

bool ControlComponent::getUseAltBackground(void) const
{
    return (useAltBackground);
}

void ControlComponent::setActive(bool shouldBeActive)
{
    active = shouldBeActive;
    repaint();
}

bool ControlComponent::getActive(void) const
{
    return (active);
}

ControlComponent *
    ControlComponent::createControlComponent(const Control &control,
                                             MainDelegate &newDelegate)
{
    // \todo refactor this to a map function instead of if-else
    ControlComponent *c = nullptr;

    if (control.getType() == Control::Type::Fader) {
        c = new FaderControl(control, newDelegate);
    } else if (control.getType() == Control::Type::List) {
        if (control.getVariant() == Control::Variant::ValueOnly) {
            c = new ListButtonControl(control, newDelegate);
        } else {
            c = new ListControl(control, newDelegate);
        }
    } else if (control.getType() == Control::Type::Pad) {
        c = new PadControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Adsr) {
        c = new ADSRControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Adr) {
        c = new ADRControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Dx7envelope) {
        c = new Dx7EnvControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Knob) {
        c = new KnobControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Relative) {
        c = new RelativeControl(control, newDelegate);
    }

    if (c) {
        c->syncComponentProperties();
    }

    return (c);
}

ControlComponent *
    ControlComponent::createDetailControlComponent(const Control &control,
                                                   MainDelegate &newDelegate)
{
    ControlComponent *c = nullptr;

    if (control.getType() == Control::Type::Fader) {
        c = new FaderDetailControl(control, newDelegate);
    } else if (control.getType() == Control::Type::List) {
        c = new ListDetailControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Adsr) {
        c = new ADSRDetailControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Adr) {
        c = new ADRDetailControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Dx7envelope) {
        c = new Dx7EnvDetailControl(control, newDelegate);
    }

    if (c) {
        c->setName(control.getName());
        c->setId(control.getId());
    }

    return (c);
}

uint16_t ControlComponent::calculateRelativeMidiValue(int16_t delta,
                                                      const ControlValue &cv)
{
    uint16_t midiValue = 0;
    RelativeMode relativeMode = cv.message.getRelativeMode();

    if (relativeMode == RelativeMode::signBit2) {
        // inc [01 - 63] dec [65 - 127]
        midiValue = (delta > 0) ? delta : 64 + abs(delta);
    } else if (relativeMode == RelativeMode::binOffset) {
        // inc [65 - 127] dec [63 - 0]
        midiValue = (delta > 0) ? 64 + delta : 64 + delta;
    } else if (relativeMode == RelativeMode::twosComplement) {
        // inc [1 - 64] dec [127 - 65]
        midiValue = (delta > 0) ? delta : 128 + delta;
    } else {
        // RelativeMode::signBit (and anything else)
        // inc [65 - 127] dec [1 - 63]
        midiValue = (delta > 0) ? 64 + delta : abs(delta);
    }
    return (midiValue);
}

uint16_t ControlComponent::calculateAbsoluteMidiValue(int16_t newDisplayValue,
                                                      const ControlValue &cv)
{
    uint16_t midiValue = 0;
    newDisplayValue = constrain(newDisplayValue, cv.getMin(), cv.getMax());
    midiValue = translateValueToMidiValue(cv.message.getSignMode(),
                                          cv.message.getBitWidth(),
                                          newDisplayValue,
                                          cv.getMin(),
                                          cv.getMax(),
                                          cv.message.getMidiMin(),
                                          cv.message.getMidiMax());
    return (midiValue);
}