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

#include "TextBoxControl.h"
#include "LookAndFeel.h"

TextBoxControl::TextBoxControl(const Control &control,
                               MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate)
{
}

/*
 * A text box has a value like any other control, and a script may well point
 * one at a real parameter - a program number whose name is what the box shows.
 * Nothing about the number is drawn, so this only asks for a repaint.
 */
void TextBoxControl::onMidiValueChange(
    [[maybe_unused]] const ControlValue &value,
    [[maybe_unused]] int16_t midiValue,
    [[maybe_unused]] uint8_t handle)
{
    repaint();
}

/* There is no editor to open yet, and the default long hold would open an
 * empty detail window. */
void TextBoxControl::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
}

bool TextBoxControl::isValueOnly(void) const
{
    return (control.getVariant() == Control::Variant::ValueOnly);
}

bool TextBoxControl::isBaseline(void) const
{
    return (control.getVariant() == Control::Variant::Baseline);
}

/*
 * Where the strip under a value would start: the strip height above where the
 * graphics end. A list draws its dots there and a thin fader its track; a
 * baseline text box draws nothing there and only borrows the line.
 */
uint16_t TextBoxControl::valueStripTop(void) const
{
    const uint16_t graphicsEnd =
        (getHeight() > controlMetrics.graphicsBottomInset)
            ? (uint16_t)(getHeight() - controlMetrics.graphicsBottomInset)
            : 0;
    const uint16_t strip = controlMetrics.stripHeight;

    return ((graphicsEnd > strip) ? (uint16_t)(graphicsEnd - strip) : 0);
}

void TextBoxControl::paint(Graphics &g)
{
    const ControlValue &controlValue = control.getValue(0);
    const Rectangle bounds = getLocalBounds();
    const uint32_t colour = control.getColour565();

    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);

    if (colour == 0) {
        return;
    }

    /* valueOnly gives the whole height to the text: there is no name beneath
     * it to leave room for, and no outline to sit inside. baseline keeps the
     * name and drops the outline. */
    const bool valueOnly = isValueOnly();
    const bool baseline = isBaseline();
    const bool outlined = !valueOnly && !baseline;
    const uint16_t boxHeight =
        valueOnly ? bounds.getHeight()
                  : (uint16_t)((bounds.getHeight()
                                > controlMetrics.textBoxBottomInset)
                                   ? bounds.getHeight()
                                         - controlMetrics.textBoxBottomInset
                                   : bounds.getHeight());

    if (outlined) {
        LookAndFeel::paintValueBox(g, bounds, colour, boxHeight, false);
    }

    /* The value's label - the same override a fader prints instead of its
     * number - and a dash when there is none: an empty box says nothing about
     * whether it is empty or broken. */
    char text[ControlValue::MaxLabelLength + 1];

    if (controlValue.isLabelSet()) {
        copyString(text, controlValue.getLabel(), sizeof(text) - 1);
    } else {
        copyString(text, emptyPlaceholder, sizeof(text) - 1);
    }

    const Control::Font font = control.getFont();
    const uint16_t lineHeight = g.user().getLineHeight(font.size, font.face);

    /* Padding is what keeps text off an outline. Without one there is nothing
     * to keep it off, and a valueOnly box is often the narrow one. */
    const uint16_t margin = outlined ? (textMargin * 2) : 0;
    const uint16_t available =
        (bounds.getWidth() > margin) ? (bounds.getWidth() - margin) : 0;

    char fitted[ControlValue::MaxLabelLength + 2];
    fitValueText(g, fitted, sizeof(fitted), text, available);

    /* Centred in the box, and clamped so that a face taller than the box it
     * was given still starts inside it: a preset may ask for the huge face in
     * a one-slot box.
     *
     * baseline is the exception: its text stands where a list's value stands,
     * so a text box in a row of lists and thin faders shares their line
     * whatever face it was given. */
    uint16_t textY = 0;

    if (baseline) {
        textY = valueLabelY(g, valueStripTop());
    } else if (boxHeight > lineHeight) {
        textY = (uint16_t)((boxHeight - lineHeight) / 2);
    }

    g.user().printText(0,
                       textY,
                       fitted,
                       bounds.getWidth(),
                       TextAlign::center,
                       Colours565::white,
                       font.size,
                       font.face,
                       font.spacing);

    if (!valueOnly) {
        g.user().printText(0,
                           getHeight() - controlMetrics.nameInset,
                           getName(),
                           getWidth(),
                           TextAlign::center,
                           Colours565::white,
                           TextSize::medium);
    }

    ControlComponent::paint(g);
}
