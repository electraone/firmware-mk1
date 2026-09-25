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
 * @file FontSpecimen.h
 *
 * @brief Every face the unit can draw, in every colour it can draw it in.
 *
 * One screen: the system's two faces in their three schemes, and then the
 * user's four sizes in both weights and both spacings, each of those drawn
 * six times across the panel - once per colour a preset can ask for. That is
 * the whole matrix, which is the point: a face that is packed wrong, keyed
 * wrong or a row out of place is visible here and nowhere else.
 *
 * The sample is the same string everywhere, letters and digits both, because
 * what is being compared is the faces and not the words. The digits are what
 * make the two spacings tell themselves apart.
 */

#pragma once

#include "Component.h"

class FontSpecimen final : public Component
{
public:
    FontSpecimen()
    {
        setName("fontSpecimen");
        setBounds(0, 0, 1024, 600);
    }

    void paint(Graphics &g) override
    {
        g.fillAll(Colours565::black);

        uint16_t y = 4;

        y = paintSystem(g, y);
        y += 6;

        for (uint8_t spacing = 0; spacing < numTextSpacings; spacing++) {
            for (uint8_t face = 0; face < numTextFaces; face++) {
                y = paintBlock(g, y, (TextSpacing)spacing, (TextFace)face);
            }
        }
    }

private:
    /** The string every face is shown with. */
    static constexpr const char *sample = "Ag0123";

    /** How wide a column of the six colours is. */
    static constexpr uint16_t column = 1024 / 6;

    /** The system's two faces in each of the three schemes it carries. */
    uint16_t paintSystem(Graphics &g, uint16_t y)
    {
        g.system().printText(4, y, "SYSTEM", TextStyle::smallTransparent, 200);
        y += 14;

        const uint32_t schemes[] = { Colours565::white,
                                     Colours565::black,
                                     Colours565::dimgrey };
        const char *names[] = { "white", "black on white", "on dimmed" };

        for (uint8_t scheme = 0; scheme < 3; scheme++) {
            uint16_t x = 4 + scheme * 220;

            g.system().printText(x,
                                 y,
                                 names[scheme],
                                 TextStyle::mediumTransparent,
                                 200,
                                 TextAlign::left,
                                 schemes[scheme]);
            g.system().printText(x,
                                 y + 19,
                                 sample,
                                 TextStyle::smallTransparent,
                                 200,
                                 TextAlign::left,
                                 schemes[scheme]);
        }

        return (y + 34);
    }

    /** One weight and one spacing: four sizes, six colours of each. */
    uint16_t
        paintBlock(Graphics &g, uint16_t y, TextSpacing spacing, TextFace face)
    {
        char heading[40];

        snprintf(heading,
                 sizeof(heading),
                 "%s %s",
                 (spacing == TextSpacing::proportional) ? "PROPORTIONAL"
                                                        : "MONOSPACED",
                 (face == TextFace::regular) ? "REGULAR" : "BOLD");

        g.setColour(Colours565::dimgrey);
        g.fillRect(0, y, 1024, 1);
        y += 4;

        g.system().printText(4, y, heading, TextStyle::smallTransparent, 400);
        y += 15;

        for (uint8_t size = 0; size < numTextSizes; size++) {
            for (uint8_t colour = 0; colour < TextUser::getNumColours();
                 colour++) {
                g.user().printText(colour * column + 4,
                                   y,
                                   sample,
                                   column - 8,
                                   TextAlign::left,
                                   TextUser::getColour(colour),
                                   (TextSize)size,
                                   face,
                                   spacing);
            }

            y += g.user().getLineHeight((TextSize)size, face) + 1;
        }

        return (y + 3);
    }
};
