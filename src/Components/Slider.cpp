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

/**
 * @file Slider.cpp
 *
 * @brief A base class for slider style components.
 */

#include "Slider.h"
#include <cstdio>

Slider::Slider()
    : colour(Colours565::red), formatString("%d"), valueBoxPosition(NoValueBox)
{
}

void Slider::setValueBoxPosition(Slider::ValueBoxPosition newValueBoxPosition)
{
    valueBoxPosition = newValueBoxPosition;
    repaint();
}

Slider::ValueBoxPosition Slider::getValueBoxPosition(void) const
{
    return (valueBoxPosition);
}

void Slider::setMinimum(int16_t newMin)
{
    value.setMin(newMin);
    if (getValue() < newMin) {
        setValue(newMin);
    }
    repaint();
}

int16_t Slider::getMinimum(void) const
{
    return (value.getMin());
}

void Slider::setMaximum(int16_t newMax)
{
    value.setMax(newMax);
    if (getValue() > newMax) {
        setValue(newMax);
    }
    repaint();
}

int16_t Slider::getMaximum(void) const
{
    return (value.getMax());
}

void Slider::setRange(int16_t newMin, int16_t newMax)
{
    value.setRange(newMin, newMax);
    repaint();
}

void Slider::setValue(int16_t newValue)
{
    value.set(newValue);
    repaint();
}

void Slider::applyValue(int16_t delta)
{
    value.apply(delta);
    repaint();
}

int16_t Slider::getValue(void) const
{
    return (value.get());
}

void Slider::setValueFormat(const char *newValueFormat)
{
    formatString = newValueFormat;
    repaint();
}

void Slider::setColour(uint32_t newColour)
{
    colour = newColour;
    repaint();
}

void Slider::resized(void)
{
    repaint();
}

void Slider::onTouchMove([[maybe_unused]] const TouchEvent &touchEvent)
{
}

void Slider::onTouchDown([[maybe_unused]] const TouchEvent &touchEvent)
{
}

void Slider::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
}

void Slider::onPotChange(const PotEvent &potEvent)
{
    if (potEvent.getRelativeChange()) {
        applyValue(potEvent.getRelativeChange());
    }
}

void Slider::onPotTouchDown([[maybe_unused]] const PotEvent &potEvent)
{
}

void Slider::onPotTouchUp([[maybe_unused]] const PotEvent &potEvent)
{
}

void Slider::paintValueBox(Graphics &g)
{
    char stringValue[10];
    snprintf(stringValue, sizeof(stringValue), formatString, value.get());
    g.user().printText(0,
                       getHeight() - 13,
                       stringValue,
                       getWidth(),
                       TextAlign::center,
                       Colours565::white,
                       TextSize::medium);
}
