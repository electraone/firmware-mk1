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

#include "BankButton.h"

BankButton::BankButton()
{
}

void BankButton::paint(Graphics &g)
{
    uint16_t backgroundColour = getColour565();

    g.fillAll(LookAndFeel::backgroundColour);

    if (isSelected()) {
        g.setColour(backgroundColour);
        g.fillRoundRect(0, 0, getWidth(), getHeight(), getRadius());
        g.dimAll();
    } else if (isHighlighted()) {
        backgroundColour = Colours565::lighter(backgroundColour, 0.1f);
        g.setColour(backgroundColour);
        g.fillRoundRect(0, 0, getWidth(), getHeight(), getRadius());
        g.dimAll();
    }

    g.setColour(getSelectedColour());
    g.drawRoundRect(0, 0, getWidth(), getHeight(), getRadius());
    g.printText(0,
                getHeight() / 2 - 7,
                getLabel(),
                TextStyle::smallTransparent,
                getWidth(),
                TextAlign::center);
}
