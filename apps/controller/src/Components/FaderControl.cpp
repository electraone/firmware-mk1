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
#include "FaderControl.h"

FaderControl::FaderControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate), thresholdCrossed(false)
{
    const auto &controlValue = control.getValue(0);

    setMinimum(controlValue.getMin());
    setMaximum(controlValue.getMax());
    list.assignListData(controlValue.getOverlay());
    updateValueFromParameterMap();
}

void FaderControl::syncComponentProperties(void)
{
    const auto &controlValue = control.getValue(0);
    setMinimum(controlValue.getMin());
    setMaximum(controlValue.getMax());
    list.assignListData(controlValue.getOverlay());
    setNumValues(controlValue.getNumSteps());
    ControlComponent::syncComponentProperties();
}

void FaderControl::onTouchDown(const TouchEvent &touchEvent)
{
    previousScreenX = touchEvent.getScreenX();
    thresholdCrossed = false;
}

void FaderControl::onTouchMove(const TouchEvent &touchEvent)
{
    int16_t delta = touchEvent.getScreenX() - previousScreenX;
    const auto &cv = control.getValue(0);

    if (cv.isRelative()) {
        // @todo accelleration ignored for now
        delta = ControlValue::sign(delta);
        emitValueChange(delta, cv);
    } else {
        int16_t max = value.getMax();
        int16_t min = value.getMin();
        float step = getWidth() / (float)(max - min);
        if (!thresholdCrossed) {
            if (delta < 10) {
                return;
            } else {
                thresholdCrossed = true;
                delta = 0;
            }
        }
        previousScreenX = touchEvent.getScreenX();

        int16_t newDisplayValue =
            constrain(ceil(getValue() + ((float)delta / step)), min, max);
        emitValueChange(newDisplayValue, cv);
    }
}

void FaderControl::onPotTouchDown(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchDown(potEvent);
}

void FaderControl::onPotChange(const PotEvent &potEvent)
{
    if (int16_t delta = potEvent.getAcceleratedChange()) {
        const auto &cv = control.getValue(0);
        int16_t newDisplayValue = cv.isRelative()
                                      ? cv.calculateRelativeDelta(delta)
                                      : getValue() + delta;
        emitValueChange(newDisplayValue, cv);
    }
}

void FaderControl::onPotTouchUp(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchUp(potEvent);
}

void FaderControl::onMidiValueChange(const ControlValue &value,
                                     int16_t midiValue,
                                     [[maybe_unused]] uint8_t handle)
{
    int16_t newDisplayValue = value.translateMidiValue(midiValue);
    setValue(newDisplayValue);
}

void FaderControl::paint(Graphics &g)
{
    Rectangle sliderBounds = getBounds();

    paintFader(g,
               sliderBounds,
               control.getColour565(),
               value.getMin(),
               value.getMax(),
               value.get(),
               list.getList(),
               control.getMode(),
               control.getVariant());

    g.printText(0,
                getHeight() - 20,
                getName(),
                TextStyle::mediumWhiteOnBlack,
                getWidth(),
                TextAlign::center,
                2);
    ControlComponent::paint(g);
}

void FaderControl::paintFader(Graphics &g,
                              const Rectangle &bounds,
                              uint32_t colour,
                              int16_t min,
                              int16_t max,
                              int16_t val,
                              const ListData *items,
                              Control::Mode mode,
                              Control::Variant variant)
{
    uint32_t colourTrack = Colours565::darker(colour, 0.3f);
    uint32_t backgroundColour = getUseAltBackground()
                                    ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour;

    if (items) {
        variant = Control::Variant::Thin;
    }

    uint16_t barHeight = 0;
    uint16_t padding = 0;

    if (variant == Control::Variant::Thin) {
        barHeight = 9;
        padding = bounds.getHeight() - 36;
    } else {
        barHeight = 24;
        padding = bounds.getHeight() - 51;
    }

    uint16_t barX = 0;

    // For zero based faders, fing the start position of the bar
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

    /*
    if (barWidth > 1) {
        g.setColour(Colours565::black);
        uint16_t separatorX = 0;
        separatorX = ((val > 0) ? barX + barWidth : barX + barWidth - 1);
        g.drawLine(
            separatorX, padding, separatorX, padding + barHeight - 1);
    }
*/
    // Paint the outline
    g.setColour(backgroundColour);
    g.drawPixel(0, padding);
    g.drawPixel(0, padding + barHeight - 1);
    g.drawPixel(getWidth(), padding);
    g.drawPixel(getWidth(), padding + barHeight - 1);

    /*
    g.setColour(Colours565::darker(colourTrack, 0.9f));
    g.drawPixel(1, padding);
    g.drawPixel(0, padding + 1);
    g.drawPixel(1, padding + barHeight - 1);
    g.drawPixel(0, padding + barHeight - 2);
    g.drawPixel(getWidth() - 2, padding);
    g.drawPixel(getWidth(), padding + 1);
    g.drawPixel(getWidth() - 2, padding + barHeight - 1);
    g.drawPixel(getWidth(), padding + barHeight - 2);
*/
    if (variant == Control::Variant::Thin) {
        paintValueFixed(g, bounds, colour, val, items, mode, variant);
    } else {
        paintValueFloating(
            g, bounds, colour, barX + barWidth, val, items, mode, variant);
    }
}

void FaderControl::paintValueFixed(Graphics &g,
                                   const Rectangle &bounds,
                                   uint32_t colour,
                                   int16_t val,
                                   const ListData *items,
                                   [[maybe_unused]] Control::Mode mode,
                                   [[maybe_unused]] Control::Variant variant)
{
    uint16_t labelYPosition = 0;

    // Print the label text / Bitmap if exists
    if (items && !items->getByValue(val).isBitmapEmpty()) {
        uint16_t paddingBitmap = ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;
        items->getByValue(val).paintBitmap(paddingBitmap, 0, colour);
    } else if (items && !items->getByValue(val).isLabelEmpty()) {
        g.setColour(Colours565::white);
        g.printText(0,
                    0,
                    items->getByValue(val).getLabel(),
                    TextStyle::mediumTransparent,
                    bounds.getWidth(),
                    TextAlign::center);
    } else {
        char stringValue[ControlValue::MaxLabelLength + 1];
        if (control.getValue(0).isLabelSet()) {
            copyString(stringValue,
                       control.getValue(0).getLabel(),
                       ControlValue::MaxLabelLength);
        } else {
            snprintf(
                stringValue, ControlValue::MaxLabelLength, formatString, val);
        }

        g.printText(0,
                    labelYPosition,
                    stringValue,
                    TextStyle::mediumTransparent,
                    getWidth(),
                    TextAlign::center);
    }
}

void FaderControl::paintValueFloating(Graphics &g,
                                      [[maybe_unused]] const Rectangle &bounds,
                                      [[maybe_unused]] uint32_t colour,
                                      uint16_t x,
                                      int16_t val,
                                      [[maybe_unused]] const ListData *items,
                                      [[maybe_unused]] Control::Mode mode,
                                      [[maybe_unused]] Control::Variant variant)
{
    uint16_t labelYPosition = 13;

    char stringValue[ControlValue::MaxLabelLength + 1];

    if (control.getValue(0).isLabelSet()) {
        copyString(stringValue,
                   control.getValue(0).getLabel(),
                   ControlValue::MaxLabelLength);
    } else {
        snprintf(stringValue, ControlValue::MaxLabelLength, formatString, val);
    }

    uint16_t textWidth =
        TextBTE::getTextWidth(stringValue, TextStyle::mediumTransparent);
    g.setColour(x > (bounds.getWidth() - textWidth)
                        && isColorTooBright(colour, 0.7f)
                    ? Colours565::black
                    : Colours565::white);
    g.print(0,
            labelYPosition,
            stringValue,
            bounds.getWidth() - textPositionFromRight,
            TextAlign::right);
}

bool FaderControl::isColorTooBright(uint16_t color, float brightnessThreshold)
{
    uint16_t r = (color >> 11) & 0x1F;
    uint16_t g = (color >> 5) & 0x3F;
    uint16_t b = color & 0x1F;

    // Normalize the components to the [0, 1] range
    float R = static_cast<float>(r) / 31.0f;
    float G = static_cast<float>(g) / 63.0f;
    float B = static_cast<float>(b) / 31.0f;

    float maxColor = std::max({ R, G, B });
    float minColor = std::min({ R, G, B });

    // Calculate lightness in the HSL color space using float
    float lightness = (maxColor + minColor) / 2.0f;

    return (lightness > brightnessThreshold);
}