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
 * @file TextGraphics.h
 *
 * @brief A simple Text component.
 */

#pragma once

#include "Component.h"

class TextGraphics : virtual public Component
{
public:
    TextGraphics()
        : text(nullptr),
          style(TextStyle::smallTransparent),
          aligment(TextAlign::left)
    {
    }

    TextGraphics(Rectangle bounds,
                 const char *newText,
                 TextStyle newStyle,
                 TextAlign newAligment,
                 uint16_t newColour = Colours565::white)
        : text(newText),
          style(newStyle),
          aligment(newAligment),
          colour(newColour)
    {
        setBounds(bounds);
    }

    ~TextGraphics()
    {
    }

    void paint(Graphics &g)
    {
        if ((text != nullptr) && (strlen(text) > 0)) {
            g.user().printText(
                0, getHeight() - 12, text, style, getWidth(), aligment, colour);
        }
    }

private:
    uint16_t colour;
    const char *text;
    TextStyle style;
    TextAlign aligment;
};
