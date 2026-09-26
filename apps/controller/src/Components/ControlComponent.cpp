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
#include "ControlMetrics.h"
#include "FaderControl.h"
#include "ListControl.h"
#include "ListButtonControl.h"
#include "PadControl.h"
#include "EnvelopeControl.h"
#include "VerticalFader4Control.h"
#include "XyPadControl.h"
#include "TextBoxControl.h"
#include "CustomControl.h"
#include "ADSR.h"
#include "ADR.h"
#include "Env5Seg.h"
#include "AHDSR.h"
#include "ADSSR.h"
#include "AR.h"
#include "FaderDetailControl.h"
#include "ListDetailControl.h"
#include "EnvelopeDetailControl.h"
#include "VerticalFader4DetailControl.h"
#include "XyPadDetailControl.h"
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

    if (MultiValue *en = dynamic_cast<MultiValue *>(this)) {
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

uint16_t ControlComponent::valueLabelY(Graphics &g, uint16_t graphicsTop) const
{
    const Control::Font font = getValueFont();
    const uint16_t aboveBaseline = g.user().getInkTop(font.size, font.face)
                                   + g.user().getInkHeight(font.size, font.face)
                                   + controlMetrics.valueGap;

    return ((graphicsTop > aboveBaseline)
                ? (uint16_t)(graphicsTop - aboveBaseline)
                : 0);
}

uint16_t ControlComponent::valueLabelYCentredOn(Graphics &g,
                                                uint16_t centreY) const
{
    const Control::Font font = getValueFont();
    const uint16_t above =
        (uint16_t)(g.user().getInkTop(font.size, font.face)
                   + g.user().getInkHeight(font.size, font.face) / 2);

    return ((centreY > above) ? (uint16_t)(centreY - above) : 0);
}

void ControlComponent::printValue(Graphics &g,
                                  uint16_t x,
                                  uint16_t y,
                                  const char *text,
                                  uint16_t width,
                                  TextAlign align,
                                  uint32_t colour)
{
    const Control::Font font = getValueFont();

    g.user().printText(
        x, y, text, width, align, colour, font.size, font.face, font.spacing);
}

uint16_t ControlComponent::valueTextWidth(Graphics &g, const char *text) const
{
    const Control::Font font = getValueFont();

    return (g.user().getTextWidth(text, font.size, font.face, font.spacing));
}

void ControlComponent::fitValueText(Graphics &g,
                                    char *out,
                                    size_t size,
                                    const char *text,
                                    uint16_t maxWidth) const
{
    if (size == 0) {
        return;
    }

    copyString(out, text, (uint16_t)(size - 1));

    if (valueTextWidth(g, out) <= maxWidth) {
        return;
    }

    /* Dropped a character at a time and measured again, because the face is
     * proportional - there is no width per character to divide by. The
     * strings are at most MaxLabelLength long, so this is bounded at twenty
     * passes over a very short buffer. */
    size_t length = strlen(out);

    while (length > 0) {
        length--;
        out[length] = '\0';

        /* A cut that lands after a space would put the marker a word's gap
         * away from the text it marks. */
        if ((length > 0) && (out[length - 1] == ' ')) {
            continue;
        }

        char candidate[ControlValue::MaxLabelLength + 2];

        snprintf(candidate, sizeof(candidate), "%s.", out);

        if (valueTextWidth(g, candidate) <= maxWidth) {
            copyString(out, candidate, (uint16_t)(size - 1));
            return;
        }
    }

    /* Narrower than a single character and its marker. Nothing legible fits,
     * and the marker alone would say less than nothing does. */
    out[0] = '\0';
}

uint16_t ControlComponent::nameBaseline(Graphics &g) const
{
    const uint16_t y = (getHeight() > controlMetrics.nameInset)
                           ? (uint16_t)(getHeight() - controlMetrics.nameInset)
                           : 0;

    return ((uint16_t)(y
                       + g.user().getInkTop(TextSize::medium, TextFace::regular)
                       + g.user().getInkHeight(TextSize::medium,
                                               TextFace::regular)));
}

uint16_t ControlComponent::colourOfIndex(const ListData *items,
                                         uint16_t index) const
{
    /* getByIndex indexes the vector without checking, so the bounds are
     * checked here rather than at every call site. */
    if ((items == nullptr) || (index >= items->getNumItems())) {
        return (control.getColour565());
    }

    return (colourOf(items->getByIndex(index)));
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
    } else if (control.getType() == Control::Type::Vfader) {
        c = new VerticalFader4Control(control, newDelegate);
    } else if (control.getType() == Control::Type::XyPad) {
        c = new XyPadControl(control, newDelegate);
    } else if (control.getType() == Control::Type::TextBox) {
        c = new TextBoxControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Custom) {
        c = new CustomControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Adsr) {
        c = new ShapedEnvelope<ADSR, EnvelopeControl>(control, newDelegate);
    } else if (control.getType() == Control::Type::Ahdsr) {
        c = new ShapedEnvelope<AHDSR, EnvelopeControl>(control, newDelegate);
    } else if (control.getType() == Control::Type::Adssr) {
        c = new ShapedEnvelope<ADSSR, EnvelopeControl>(control, newDelegate);
    } else if (control.getType() == Control::Type::Adr) {
        c = new ShapedEnvelope<ADR, EnvelopeControl>(control, newDelegate);
    } else if (control.getType() == Control::Type::Ar) {
        c = new ShapedEnvelope<AR, EnvelopeControl>(control, newDelegate);
    } else if (control.getType() == Control::Type::Dx7envelope) {
        c = new ShapedEnvelope<Env5Seg, EnvelopeControl>(control, newDelegate);
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
    } else if (control.getType() == Control::Type::Vfader) {
        c = new VerticalFader4DetailControl(control, newDelegate);
    } else if (control.getType() == Control::Type::XyPad) {
        c = new XyPadDetailControl(control, newDelegate);
    } else if (control.getType() == Control::Type::Adsr) {
        c = new ShapedEnvelope<ADSR, EnvelopeDetailControl>(control,
                                                            newDelegate);
    } else if (control.getType() == Control::Type::Ahdsr) {
        c = new ShapedEnvelope<AHDSR, EnvelopeDetailControl>(control,
                                                             newDelegate);
    } else if (control.getType() == Control::Type::Adssr) {
        c = new ShapedEnvelope<ADSSR, EnvelopeDetailControl>(control,
                                                             newDelegate);
    } else if (control.getType() == Control::Type::Adr) {
        c = new ShapedEnvelope<ADR, EnvelopeDetailControl>(control,
                                                           newDelegate);
    } else if (control.getType() == Control::Type::Ar) {
        c = new ShapedEnvelope<AR, EnvelopeDetailControl>(control, newDelegate);
    } else if (control.getType() == Control::Type::Dx7envelope) {
        c = new ShapedEnvelope<Env5Seg, EnvelopeDetailControl>(control,
                                                               newDelegate);
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