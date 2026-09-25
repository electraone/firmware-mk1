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
 * @file TextSystem.cpp
 *
 * @brief The system UI's text: two faces, three colour schemes.
 */

#include "TextSystem.h"

namespace
{
    /* The order the generator emits the system bands in. */
    constexpr uint8_t bandWhite = 0;
    constexpr uint8_t bandBlackOnWhite = 1;
    constexpr uint8_t bandWhiteOnDimmed = 2;
} // namespace

TextSystem::TextSystem(const uint8_t CSp, const uint8_t RSTp)
    : TextAtlas(CSp, RSTp), VideoRam(CSp, RSTp), FrameBuffer(CSp, RSTp)
{
}

const FontFace &TextSystem::fontFor(TextStyle style)
{
    return (fontSystem[(style == TextStyle::mediumTransparent) ? 1 : 0]);
}

const FontBand &TextSystem::bandFor(const FontFace &face, uint32_t colour)
{
    if (colour == 0x0000) {
        return (face.band[bandBlackOnWhite]);
    }

    if (colour == 0xFFFF) {
        return (face.band[bandWhite]);
    }

    return (face.band[bandWhiteOnDimmed]);
}

void TextSystem::uploadFonts(void)
{
    uploadFaces(fontSystem, (uint8_t)(sizeof(fontSystem) / sizeof(FontFace)));
}

void TextSystem::printText(uint16_t x,
                           uint16_t y,
                           const char *text,
                           TextStyle style,
                           uint16_t width,
                           TextAlign align,
                           uint32_t colour)
{
    const FontFace &face = fontFor(style);
    const FontBand &band = bandFor(face, colour);

    /* A scheme with a field of its own has it in the glyph boxes and nowhere
     * else, so the gaps between the characters would be whatever is behind
     * them. Laying the field first is not the caller's job: it is what having
     * one means. */
    if (band.field != 0x0000) {
        fillTextBox(x, y, text, face, width, align, band.field);
    }

    drawString(x, y, text, face, band, width, align, true);
}

void TextSystem::paintTextPlaceHolder(uint16_t x,
                                      uint16_t y,
                                      const char *text,
                                      TextStyle style,
                                      uint16_t width,
                                      TextAlign align,
                                      uint32_t backgroundColor)
{
    fillTextBox(x, y, text, fontFor(style), width, align, backgroundColor);
}

void TextSystem::printVerticalText(uint16_t x, uint16_t y, const char *text)
{
    const FontFace &face = fontFor(TextStyle::smallTransparent);
    drawVertical(x, y, text, face, face.band[bandWhite]);
}

uint16_t TextSystem::getTextWidth(const char *text, TextStyle style)
{
    return (measureString(text, fontFor(style)));
}

uint16_t TextSystem::getLineHeight(TextStyle style)
{
    return (fontFor(style).cell);
}
