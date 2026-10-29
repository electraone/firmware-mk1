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
    copyString(label, newLabel ? newLabel : "", MaxSetLabelLength);
    setName(label);
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

/*
 * Black is what an unlit button is, and a group asking for it is how a
 * buttonlike group says so - the button is drawn as an outline for that colour
 * and solid for every other. The outline is drawn in the group's own colour
 * rather than in black, which would draw nothing.
 */
void GroupControl::paint(Graphics &g)
{
    const Rectangle bounds = getBounds();
    const Group::Font font = group.getFont();
    const uint32_t ground = getUseAltBackground()
                                ? LookAndFeel::altBackgroundColour
                                : LookAndFeel::backgroundColour;

    if (group.getVariant() == Group::Variant::Highlighted) {
        LookAndFeel::paintHighlightedGroup(
            g, bounds, colour, label, font, ground);
    } else if (group.getVariant() == Group::Variant::Buttonlike) {
        const bool filled = (colour != Colours565::black);

        LookAndFeel::paintButtonlikeGroup(g,
                                          bounds,
                                          filled ? colour
                                                 : group.getColour565(),
                                          label,
                                          filled,
                                          font,
                                          ground);
    } else {
        LookAndFeel::paintStandardGroup(g, bounds, colour, label, font, ground);
    }
}
