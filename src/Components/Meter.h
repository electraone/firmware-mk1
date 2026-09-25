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
 * @file Meter.h
 *
 * @brief A base class for Meter style components.
 */

#pragma once

#include "Component.h"

class Meter : public Slider
{
public:
    Indicator() : value(0), max(100)
    {
    }

    virtual ~Indicator() = default;

    void setValue(float newValue)
    {
        if (strlen(newLabel) == 0) {
            *label = '\0';
        } else {
            copyString(label, newLabel, maxLabelLength);
        }
    }

    void setBackgroundColour(uint16_t newBackgroundColour)
    {
        backgroundColour = newBackgroundColour;
    }

    void setDisabled(bool shouldBeDisabled)
    {
        isDisabled = shouldBeDisabled;
    }

    void setActive(bool shouldBeActive)
    {
        active = shouldBeActive;
    }

    void paint(Graphics &g)
    {
        uint16_t labelWidth =
            g.system().getTextWidth(label, TextStyle::smallTransparent);

        g.system().printText(0,
                             0,
                             label,
                             TextStyle::smallTransparent,
                             getWidth(),
                             TextAlign::center);

        if (active == true) {
            g.setColour(Colours565::white);
            g.drawLine((getWidth() / 2) - (labelWidth / 2) - 1,
                       15,
                       (getWidth() / 2) - (labelWidth / 2) - 1 + labelWidth,
                       15);
        }
    }

    void onPotTouchDown(const PotEvent &potEvent) override
    {
        if (onClick) {
            onClick();
        }
    }

    void onPotTouchUp(const PotEvent &potEvent) override
    {
        if (onRelease) {
            onRelease();
        }
    }

private:
    static const uint8_t maxLabelLength = 20;
    char label[maxLabelLength + 1];
    uint16_t backgroundColour;
    bool active;

    struct {
        bool isDisabled : 1;
    };
};
