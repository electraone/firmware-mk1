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
    const uint32_t colour = control.getColour565();

    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);

    if (colour == 0) {
        return;
    }

    if (control.getVariant() == Control::Variant::Checkbox) {
        paintCheckbox(g);
    } else if (control.getVariant() == Control::Variant::ValueOnly) {
        paintValueOnly(g);
    } else {
        paintPad(g);
    }

    ControlComponent::paint(g);
}

/*
 * The pad's box, laid out as paintPad() lays it out so that the two variants
 * line up in a row: from padBoxTopInset down, to the model's inset above the
 * bottom - or, because this model ends its boxes on the name baseline, to the
 * baseline of the names beside it.
 */
Rectangle PadControl::padBounds(Graphics &g) const
{
    const uint16_t inset = controlMetrics.padHeightInset;

    Rectangle bounds(0,
                     controlMetrics.padBoxTopInset,
                     getWidth(),
                     (getHeight() > inset) ? (uint16_t)(getHeight() - inset)
                                           : getHeight());

    if (controlMetrics.boxesEndOnNameBaseline) {
        const uint16_t baseline = nameBaseline(g);

        bounds.setHeight((baseline < getHeight()) ? baseline : getHeight());
    }

    return (bounds);
}

void PadControl::paintPad(Graphics &g)
{
    const uint32_t colour = control.getColour565();
    const Rectangle bounds = padBounds(g);
    const bool isMomentary = (control.getMode() == Control::Mode::Momentary);

    LookAndFeel::paintPad(g, bounds, colour, isMomentary, getState());

    /* What paintPad() really fills with - 0.5 on, 0.4 off. Judging the label
     * against anything else is judging it against a colour that is not on the
     * screen. */
    const uint32_t currentColour = getState()
                                       ? Colours565::darker(colour, 0.5f)
                                       : Colours565::darker(colour, 0.4f);
    const uint32_t textColour = Colours565::textColourOn(currentColour);

    /* The name, not the value: a pad is on or off, and printing 0 or 1 would
     * tell nobody anything. */
    char stringValue[ControlValue::MaxLabelLength + 1];
    resolveStateText(stringValue, sizeof(stringValue), nullptr);

    /* A short pad prints its value across the middle. A tall one prints its
     * name underneath instead. */
    if (controlMetrics.padValueCentred) {
        /* The pad is painted in the control's colour under this, so the text
         * is drawn from the masks and blended against the pad rather than
         * against the black the renderer assumes. See TextAtlas.h. */
        auto &text = g.user();
        const uint32_t previousBackground = text.getTextBackgroundColour();

        text.setTextBackgroundColour(currentColour);

        /* The control's own face, as a fader prints its value in. Centred on
         * the pad by the letters' ink - on the box paintPad() draws, which
         * starts at the top of the control whatever bounds' y says. */
        printValue(g,
                   0,
                   valueLabelYCentredOn(g, bounds.getHeight() / 2),
                   stringValue,
                   bounds.getWidth(),
                   TextAlign::center,
                   textColour);

        text.setTextBackgroundColour(previousBackground);
    } else {
        g.user().printText(0,
                           getHeight() - controlMetrics.nameInset,
                           getName(),
                           getWidth(),
                           TextAlign::center,
                           Colours565::white,
                           TextSize::medium);
    }
}

/*
 * A pad that shows its state as a word rather than its name: an outline in
 * the control's colour over a very dim fill while it is off, a solid fill
 * while it is on, and across the middle the overlay item whose value is the
 * MIDI value of that state - "Off" and "On", "Mono" and "Poly", whatever the
 * preset says. A pad with no overlay, or an overlay with no item for the
 * state, falls back on the name, as the default pad does.
 *
 * ValueOnly because that is what the word means on a list too: no name of its
 * own, the value across the control.
 */
void PadControl::paintValueOnly(Graphics &g)
{
    const ControlValue &controlValue = control.getValue(0);
    const uint32_t colour = control.getColour565();
    const bool on = getState();
    const Rectangle bounds = padBounds(g);

    /* The dim fill of a pad at rest is the same fraction of its colour on
     * every pad; the lit one is just short of the colour itself, so it still
     * reads as a surface rather than a light. */
    const uint32_t fill = on ? Colours565::darker(colour, 0.85f)
                             : Colours565::darker(colour, 0.12f);

    LookAndFeel::fillSmoothRoundRect(g,
                                     bounds.getX(),
                                     bounds.getY(),
                                     bounds.getWidth(),
                                     bounds.getHeight(),
                                     fill,
                                     LookAndFeel::backgroundColour,
                                     LookAndFeel::padRadius);

    if (!on) {
        LookAndFeel::drawSmoothRoundRect(g,
                                         bounds.getX(),
                                         bounds.getY(),
                                         bounds.getWidth(),
                                         bounds.getHeight(),
                                         colour,
                                         fill,
                                         LookAndFeel::backgroundColour,
                                         LookAndFeel::padRadius);
    }

    /* The overlay item for this state, found by the MIDI value the state
     * sends - the two items a pad's overlay carries are keyed by its off and
     * on values. Held in a local because getByValue() answers with a copy. */
    const Overlay *overlay = controlValue.getOverlay();
    const int16_t stateValue = on ? controlValue.message.getOnValue()
                                  : controlValue.message.getOffValue();
    ListDataItem item =
        overlay ? overlay->getByValue(stateValue) : ListDataItem();

    char stringValue[ControlValue::MaxLabelLength + 1];
    const bool isText = resolveStateText(
        stringValue, sizeof(stringValue), overlay ? &item : nullptr);

    const uint32_t textColour = Colours565::textColourOn(fill);
    auto &text = g.user();
    const uint32_t previousBackground = text.getTextBackgroundColour();

    text.setTextBackgroundColour(fill);

    if (!isText) {
        item.paintBitmap((bounds.getWidth() - BITMAP_WIDTH) / 2 - 1,
                         bounds.getY()
                             + (bounds.getHeight() - BITMAP_HEIGHT) / 2,
                         textColour);
    } else {
        printValue(
            g,
            0,
            valueLabelYCentredOn(g, bounds.getY() + (bounds.getHeight() / 2)),
            stringValue,
            bounds.getWidth(),
            TextAlign::center,
            textColour);
    }

    text.setTextBackgroundColour(previousBackground);
}

/*
 * A toggle switch rather than a box, standing on the row a fader's track ends
 * on so that it lines up with the fader beside it, with the control's name
 * underneath as every other named control has.
 */
void PadControl::paintCheckbox(Graphics &g)
{
    const bool named = (strlen(getName()) > 0);
    Rectangle bounds(
        36,
        named ? 0 : 4,
        (getWidth() > 72) ? (uint16_t)(getWidth() - 72) : getWidth(),
        named ? (uint16_t)(getHeight() - 25) : (uint16_t)(getHeight() - 18));

    if (named) {
        /* paintCheckbox() draws no taller than its maximum, so that is taken
         * off before the top is worked out from the bottom. */
        const uint16_t graphicsEnd =
            (getHeight() > controlMetrics.graphicsBottomInset)
                ? (uint16_t)(getHeight() - controlMetrics.graphicsBottomInset)
                : getHeight();
        const uint16_t height = (graphicsEnd < LookAndFeel::checkboxMaxHeight)
                                    ? graphicsEnd
                                    : LookAndFeel::checkboxMaxHeight;

        bounds.setY(graphicsEnd - height);
        bounds.setHeight(height);
    } else if ((controlMetrics.padCheckboxBottomInset > 0)
               && (getHeight() > controlMetrics.padCheckboxBottomInset)) {
        bounds.setY(getHeight() - controlMetrics.padCheckboxBottomInset);
    }

    LookAndFeel::paintCheckbox(g,
                               bounds,
                               Colours565::darker(control.getColour565(), 0.3f),
                               getState(),
                               false);

    g.user().printText(0,
                       getHeight() - controlMetrics.nameInset,
                       getName(),
                       getWidth(),
                       TextAlign::center,
                       Colours565::white,
                       TextSize::medium);
}

bool PadControl::resolveStateText(char *text,
                                  size_t size,
                                  const ListDataItem *item) const
{
    if (item) {
        /* Above the label, because an overlay item that carries a bitmap
         * carries no useful text to fall back on. */
        if (!item->isBitmapEmpty()) {
            return (false);
        }
        if (!item->isLabelEmpty()) {
            copyString(text, item->getLabel(), size - 1);
            return (true);
        }
    }

    const ControlValue &cv = control.getValue(0);

    if (cv.isLabelSet()) {
        copyString(text, cv.getLabel(), size - 1);
    } else {
        copyString(text, getName(), size - 1);
    }

    return (true);
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