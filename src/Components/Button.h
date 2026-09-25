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
 * @file Button.h
 *
 * @brief A Button component.
 */

#pragma once

#include "Component.h"
#include "LookAndFeel.h"

class Button : virtual public Component
{
public:
    Button()
        : highlighted(false),
          disabled(false),
          selected(false),
          longTouched(false),
          colour(0x0001),
          selectedColour(0x0004),
          radius(5)
    {
        label[0] = '\0';
    }

    Button(const char *newLabel,
           uint32_t newColour,
           uint32_t newSelectedColour,
           uint8_t newRadius)
        : highlighted(false),
          disabled(false),
          selected(false),
          colour(newColour),
          selectedColour(newSelectedColour),
          radius(newRadius)
    {
        setLabel(newLabel);
    }

    virtual ~Button() = default;

    std::function<void(void)> onClick;
    std::function<void(void)> onHold;
    std::function<bool(void)> onRelease;

    void setLabel(const char *newLabel)
    {
        if (strlen(newLabel) == 0) {
            *label = '\0';
        } else {
            copyString(label, newLabel, maxLabelLength);
        }
        repaint();
    }

    const char *getLabel(void)
    {
        return (label);
    }

    void setColour(uint32_t newColour)
    {
        colour = newColour;
        repaint();
    }

    uint32_t getColour(void) const
    {
        return (colour);
    }

    uint32_t getColour565(void) const
    {
        return (Colours888::toRGB565(colour));
    }

    void setSelectedColour(uint32_t newColour)
    {
        selectedColour = newColour;
        repaint();
    }

    uint32_t getSelectedColour(void) const
    {
        return (selectedColour);
    }

    uint32_t getSelectedColour565(void) const
    {
        return (Colours888::toRGB565(selectedColour));
    }

    void setColours(uint32_t newColour, uint32_t newSelectedColour)
    {
        colour = newColour;
        selectedColour = newSelectedColour;
        repaint();
    }

    void setRadius(uint8_t newRadius)
    {
        radius = newRadius;
        repaint();
    }

    uint8_t getRadius(void) const
    {
        return (radius);
    }

    void setDisabled(bool shouldBeDisabled)
    {
        disabled = shouldBeDisabled;
        repaint();
    }

    bool isDisabled(void) const
    {
        return (disabled);
    }

    void setSelected(bool shouldBeSelected)
    {
        selected = shouldBeSelected;
        repaint();
    }

    bool isSelected(void) const
    {
        return (selected);
    }

    void setHighlighted(bool shouldBeHighligthed)
    {
        highlighted = shouldBeHighligthed;
        repaint();
    }

    bool isHighlighted(void) const
    {
        return (highlighted);
    }

    void onTouchDown([[maybe_unused]] const TouchEvent &touchEvent) override
    {
        if (onClick) {
            setHighlighted(true);
        }
    }

    void onTouchLongHold([[maybe_unused]] const TouchEvent &touchEvent) override
    {
        if (onHold) {
            onHold();
        }
        longTouched = true;
    }

    void onTouchUp([[maybe_unused]] const TouchEvent &touchEvent) override
    {
        if (highlighted) {
            setHighlighted(false);
        }

        if (!longTouched && onClick) {
            onClick();
        }

        if (onRelease) {
            onRelease();
        }

        longTouched = false;
    }

    void onPotTouchDown([[maybe_unused]] const PotEvent &potEvent) override
    {
        if (onClick) {
            setHighlighted(true);
        }
    }

    void onPotTouchUp([[maybe_unused]] const PotEvent &potEvent) override
    {
        if (highlighted) {
            setHighlighted(false);
        }

        if (onClick) {
            onClick();
        }

        if (onRelease) {
            onRelease();
        }
    }

    void paint(Graphics &g) override
    {
        uint16_t backgroundColour = Colours888::toRGB565(colour);

        if (selected) {
            backgroundColour = Colours888::toRGB565(selectedColour);
        }

        if (highlighted) {
            backgroundColour = Colours565::lighter(backgroundColour, 0.1f);
        }

        g.setColour(backgroundColour);
        g.fillRoundRect(0, 0, getWidth(), getHeight(), radius);
        g.system().printText(0,
                             getHeight() / 2 - 7,
                             label,
                             TextStyle::smallTransparent,
                             getWidth(),
                             TextAlign::center);
        if (dimmed == true) {
            g.dimAll();
        }
    }

private:
    static const uint8_t maxLabelLength = 20;
    char label[maxLabelLength + 1];

    uint8_t radius;

    struct {
        bool highlighted : 1;
        bool disabled : 1;
        bool selected : 1;
        bool longTouched : 1;
        uint32_t colour : 24;
        uint32_t selectedColour : 24;
    };
};
