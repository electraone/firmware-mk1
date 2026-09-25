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

#include "PresetButton.h"

PresetButton::PresetButton() : flagActive(false)
{
}

void PresetButton::setFlag(bool shouldBeSelected)
{
    flagActive = shouldBeSelected;
    repaint();
}

bool PresetButton::isFlagged(void) const
{
    return flagActive;
}

void PresetButton::paint(Graphics &g)
{
    Button::paint(g);

    if (flagActive == true) {
        g.setColour(0x838e);
        g.fillRect(5, 9, 2, 1);
        g.fillRect(9, 7, 2, 5);
        g.fillRect(13, 5, 2, 9);
    }
}
