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

#include "RelativeControl.h"
#include "LookAndFeel.h"

RelativeControl::RelativeControl(const Control &control,
                                 MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate), delta(0)
{
}

void RelativeControl::onPotTouchDown(const PotEvent &potEvent)
{
    delta = 0;
    ControlComponent::onPotTouchDown(potEvent);
}

void RelativeControl::onPotChange(const PotEvent &potEvent)
{
    delta = potEvent.getAcceleratedChange();
    const auto &cv = control.getValue(0);
    emitValueChange(cv.isAccelerated() ? delta : ControlValue::sign(delta), cv);
    repaint();
}

void RelativeControl::onPotTouchUp(const PotEvent &potEvent)
{
    delta = 0;
    ControlComponent::onPotTouchUp(potEvent);
}

void RelativeControl::onMidiValueChange(
    [[maybe_unused]] const ControlValue &value,
    [[maybe_unused]] int16_t midiValue,
    [[maybe_unused]] uint8_t handle)
{
}

void RelativeControl::paint(Graphics &g)
{
    uint16_t width = getWidth();
    uint16_t centerX = width / 2;

    uint32_t colour = control.getColour565();
    uint32_t colourTrack = Colours565::darker(colour, 0.3f);
    uint32_t backgroundColour = getUseAltBackground()
                                    ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour;

    // Clear the component area
    g.fillAll(backgroundColour);

    char stringValue[ControlValue::MaxLabelLength + 1];
    if (control.getValue(0).isLabelSet()) {
        copyString(stringValue,
                   control.getValue(0).getLabel(),
                   ControlValue::MaxLabelLength);
    } else {
        snprintf(stringValue, ControlValue::MaxLabelLength, "%d", delta);
    }

    g.printText(0,
                0,
                stringValue,
                TextStyle::mediumTransparent,
                width,
                TextAlign::center);

    for (uint8_t i = 0; i < 3; i++) {
        leftArrow(g,
                  (centerX - 42) + i * 10,
                  (delta < 0) && (abs(delta) > (2 - i)),
                  colour,
                  colourTrack);
    }
    for (uint8_t i = 0; i < 3; i++) {
        rightArrow(g,
                   (centerX + 42) - i * 10,
                   (delta > 0) && (delta > (2 - i)),
                   colour,
                   colourTrack);
    }

    g.setColour(getActive() ? colour : colourTrack);
    g.fillRect(centerX - padding, yPosition, padding * 2, 7);
    g.setColour(backgroundColour);
    g.drawPixel(centerX - padding, yPosition);
    g.drawPixel(centerX - padding, yPosition + 6);
    g.drawPixel(centerX + padding - 1, yPosition);
    g.drawPixel(centerX + padding - 1, yPosition + 6);

    g.printText(0,
                getHeight() - 20,
                getName(),
                TextStyle::mediumWhiteOnBlack,
                getWidth(),
                TextAlign::center,
                2);

    ControlComponent::paint(g);
}

void RelativeControl::leftArrow(Graphics &g,
                                uint16_t x,
                                bool active,
                                uint32_t colour,
                                uint32_t colourTrack)
{
    g.setColour(active ? colour : colourTrack);
    g.fillTriangle(
        x + 0, yPosition + 3, x + 3, yPosition + 0, x + 3, yPosition + 6);
    g.fillRect(x + 4, yPosition + 0, 4, 7);
}

void RelativeControl::rightArrow(Graphics &g,
                                 uint16_t x,
                                 bool active,
                                 uint32_t colour,
                                 uint32_t colourTrack)
{
    g.setColour(active ? colour : colourTrack);
    g.fillTriangle(
        x - 1, yPosition + 3, x - 4, yPosition + 0, x - 4, yPosition + 6);
    g.fillRect(x - 8, yPosition + 0, 4, 7);
}
