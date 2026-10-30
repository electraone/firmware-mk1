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

#include "GroupControl.h"
#include "LookAndFeel.h"

GroupControl::GroupControl(const Group &groupToAssign)
    : group(groupToAssign), useAltBackground(false)
{
    auto bounds = group.getBounds();
    setBounds(bounds);

    setLabel(group.getLabel());
    setName(group.getLabel());
    setHighlighted(group.getVariant() == Group::Variant::Highlighted);
    setColour(group.getColour565());
    setVisible(group.isVisible());
}

void GroupControl::setColour(uint32_t newColour)
{
    colour = newColour;
    repaint();
}

void GroupControl::setLabel(const char *newLabel)
{
    label = newLabel;
    repaint();
}

void GroupControl::setHighlighted(bool shouldBeHighlighted)
{
    isHighligted = shouldBeHighlighted;
    repaint();
}

void GroupControl::setUseAltBackground(bool shouldUseAltBackground)
{
    useAltBackground = shouldUseAltBackground;
}

bool GroupControl::getUseAltBackground(void) const
{
    return (useAltBackground);
}

void GroupControl::paint(Graphics &g)
{
    Rectangle bounds = getBounds();
    uint16_t width = bounds.getWidth();
    uint16_t height = bounds.getHeight();

    if (isHighligted) {
        paintHighligted(g, width, height, colour);
    } else {
        paintDefault(g, width, height, colour);
        g.paintTextPlaceHolder(0,
                               2,
                               label,
                               TextStyle::smallTransparent,
                               width,
                               TextAlign::center,
                               getUseAltBackground()
                                   ? LookAndFeel::altBackgroundColour
                                   : LookAndFeel::backgroundColour);
    }

    if (strlen(label) > 0) {
        g.printText(
            0, 2, label, TextStyle::smallTransparent, width, TextAlign::center);
    }
}

void GroupControl::paintHighligted(Graphics &g,
                                   uint16_t width,
                                   uint16_t height,
                                   uint32_t colour)
{
    if (height > 20) {
        g.setColour(Colours565::darker(colour, 0.2));
        g.drawRoundRect(0, 7, width, height - 7, 2);
    }
    g.setColour(Colours565::darker(colour, 0.25));
    g.fillRoundRect(0, 0, width, 15, 2);
}

void GroupControl::paintDefault(Graphics &g,
                                uint16_t width,
                                uint16_t height,
                                uint32_t colour)
{
    g.setColour(Colours565::darker(colour, 0.2));
    if (height < 20) {
        g.drawLine(0, 7, width, 7);
    } else {
        g.drawRoundRect(0, 7, width, height - 7, 2);
    }
}