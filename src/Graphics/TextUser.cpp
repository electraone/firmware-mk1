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
 * @file TextUser.cpp
 *
 * @brief A preset's text: sixteen faces, six colours.
 */

#include "TextUser.h"

namespace
{
    /**
 * @brief A style as a face.
 *
 * Both styles are the bold weight, as they are in TextSystem - bold is what
 * the system UI asked for. At Medium the two renderers draw different bolds of
 * the same height; see TextRenderer.h.
 */
    TextSize sizeForStyle(TextStyle style)
    {
        return ((style == TextStyle::mediumTransparent) ? TextSize::medium
                                                        : TextSize::small);
    }

    TextFace faceForStyle(TextStyle style)
    {
        (void)style;
        return (TextFace::bold);
    }

    /** How far apart two colours are, channel by channel. */
    uint32_t colourDistance(uint16_t one, uint16_t other)
    {
        int16_t red = (int16_t)(((one >> 11) & 0x1F) - ((other >> 11) & 0x1F));
        int16_t green = (int16_t)(((one >> 5) & 0x3F) - ((other >> 5) & 0x3F));
        int16_t blue = (int16_t)((one & 0x1F) - (other & 0x1F));

        /* Green counted at its own precision, which is twice the other two. */
        return ((uint32_t)(red * red) + (uint32_t)(green * green / 4)
                + (uint32_t)(blue * blue));
    }
} // namespace

TextUser::TextUser(const uint8_t CSp, const uint8_t RSTp)
    : TextAtlas(CSp, RSTp),
      VideoRam(CSp, RSTp),
      FrameBuffer(CSp, RSTp),
      textBackgroundColour(0x0000)
{
}

const FontFace &
    TextUser::fontFor(TextSize size, TextFace face, TextSpacing spacing)
{
    const uint8_t sizeIndex =
        isTextSize((int)size) ? (uint8_t)size : (uint8_t)TextSize::medium;
    const uint8_t faceIndex =
        isTextFace((int)face) ? (uint8_t)face : (uint8_t)TextFace::regular;
    const uint8_t spacingIndex = isTextSpacing((int)spacing)
                                     ? (uint8_t)spacing
                                     : (uint8_t)TextSpacing::proportional;

    return (fontUser[spacingIndex][faceIndex][sizeIndex]);
}

/**
 * @brief The band whose ink is nearest what the caller asked for.
 *
 * A request for a colour the faces do not carry is drawn in the nearest one
 * they do. Refusing would be no more honest - the caller still gets pixels of
 * some colour - and it would mean every call site knowing the list.
 */
const FontBand &TextUser::bandFor(const FontFace &face, uint32_t colour)
{
    uint8_t nearest = 0;
    uint32_t best = 0xFFFFFFFF;

    for (uint8_t band = 0; band < face.numBands; band++) {
        uint32_t distance =
            colourDistance(face.band[band].ink, (uint16_t)colour);

        if (distance < best) {
            best = distance;
            nearest = band;
        }
    }

    return (face.band[nearest]);
}

uint8_t TextUser::getNumColours(void)
{
    return (fontUser[0][0][0].numBands);
}

uint32_t TextUser::getColour(uint8_t index)
{
    const FontFace &face = fontUser[0][0][0];

    return (face.band[(index < face.numBands) ? index : 0].ink);
}

void TextUser::uploadFonts(void)
{
    uploadFaces(&fontUser[0][0][0],
                (uint8_t)(numTextSpacings * numTextFaces * numTextSizes));
}

void TextUser::printText(uint16_t x,
                         uint16_t y,
                         const char *text,
                         uint16_t width,
                         TextAlign align,
                         uint32_t colour,
                         TextSize size,
                         TextFace face,
                         TextSpacing spacing)
{
    const FontFace &font = fontFor(size, face, spacing);
    drawString(x, y, text, font, bandFor(font, colour), width, align, false);
}

void TextUser::printText(uint16_t x,
                         uint16_t y,
                         const char *text,
                         TextStyle style,
                         uint16_t width,
                         TextAlign align,
                         uint32_t colour)
{
    printText(x,
              y,
              text,
              width,
              align,
              colour,
              sizeForStyle(style),
              faceForStyle(style));
}

void TextUser::paintTextPlaceHolder(uint16_t x,
                                    uint16_t y,
                                    const char *text,
                                    TextStyle style,
                                    uint16_t width,
                                    TextAlign align,
                                    uint32_t backgroundColor)
{
    paintTextPlaceHolder(x,
                         y,
                         text,
                         width,
                         align,
                         backgroundColor,
                         sizeForStyle(style),
                         faceForStyle(style));
}

void TextUser::paintTextPlaceHolder(uint16_t x,
                                    uint16_t y,
                                    const char *text,
                                    uint16_t width,
                                    TextAlign align,
                                    uint32_t backgroundColor,
                                    TextSize size,
                                    TextFace face,
                                    TextSpacing spacing)
{
    fillTextBox(x,
                y,
                text,
                fontFor(size, face, spacing),
                width,
                align,
                backgroundColor);
}

void TextUser::printVerticalText(uint16_t x, uint16_t y, const char *text)
{
    const FontFace &font =
        fontFor(TextSize::small, TextFace::regular, TextSpacing::proportional);
    drawVertical(x, y, text, font, bandFor(font, 0xFFFF));
}

uint16_t TextUser::getTextWidth(const char *text, TextStyle style)
{
    return (getTextWidth(text, sizeForStyle(style), faceForStyle(style)));
}

uint16_t TextUser::getTextWidth(const char *text,
                                TextSize size,
                                TextFace face,
                                TextSpacing spacing)
{
    return (measureString(text, fontFor(size, face, spacing)));
}

uint16_t TextUser::getLineHeight(TextStyle style)
{
    return (getLineHeight(sizeForStyle(style), faceForStyle(style)));
}

uint16_t TextUser::getLineHeight(TextSize size, TextFace face)
{
    return (fontFor(size, face, TextSpacing::proportional).cell);
}

uint16_t TextUser::getInkTop(TextSize size, TextFace face)
{
    return (fontFor(size, face, TextSpacing::proportional).inkTop);
}

uint16_t TextUser::getInkHeight(TextSize size, TextFace face)
{
    return (fontFor(size, face, TextSpacing::proportional).inkHeight);
}

void TextUser::setTextBackgroundColour(uint32_t colour)
{
    textBackgroundColour = colour;
}

uint32_t TextUser::getTextBackgroundColour(void) const
{
    return (textBackgroundColour);
}
