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
 * @file TextAtlas.cpp
 *
 * @brief The atlas both renderers draw out of, and how a string is put on it.
 */

#include "TextAtlas.h"

#include <cstring>

static_assert((uint32_t)FRAME_GLYPHS
                      + ((uint32_t)fontAtlasRows * fontAtlasWidth * 2)
                  <= (uint32_t)SDRAM_CGRAM,
              "the atlas has outgrown the video memory left for it - drop a "
              "face in tools/makefont, or move the character generator");

namespace
{
    /** One row of the atlas, built in RAM and sent over in a single transfer. */
    uint16_t atlasRow[fontAtlasWidth];

    /**
 * @brief Blends ink towards field by the coverage, in 16 bit colour.
 *
 * Per channel on the unpacked values rather than on the packed word, which is
 * the only way to get it right: the channels are five, six and five bits wide
 * and a blend of the packed number mixes them into each other.
 */
    uint16_t blend(uint16_t ink, uint16_t field, uint8_t covered)
    {
        if (covered == 0) {
            return (field);
        }

        if (covered == 0xFF) {
            return (ink);
        }

        uint16_t inkRed = (ink >> 11) & 0x1F;
        uint16_t inkGreen = (ink >> 5) & 0x3F;
        uint16_t inkBlue = ink & 0x1F;

        uint16_t fieldRed = (field >> 11) & 0x1F;
        uint16_t fieldGreen = (field >> 5) & 0x3F;
        uint16_t fieldBlue = field & 0x1F;

        uint16_t red =
            (uint16_t)(fieldRed
                       + (((int16_t)inkRed - fieldRed) * covered) / 255);
        uint16_t green =
            (uint16_t)(fieldGreen
                       + (((int16_t)inkGreen - fieldGreen) * covered) / 255);
        uint16_t blue =
            (uint16_t)(fieldBlue
                       + (((int16_t)inkBlue - fieldBlue) * covered) / 255);

        return ((uint16_t)((red << 11) | (green << 5) | blue));
    }
} // namespace

TextAtlas::TextAtlas(const uint8_t CSp, const uint8_t RSTp)
    : VideoRam(CSp, RSTp), FrameBuffer(CSp, RSTp)
{
}

uint16_t TextAtlas::glyphIndex(char character)
{
    uint8_t code = (uint8_t)character;

    if ((code < fontFirstGlyph) || (code > fontLastGlyph)) {
        return (0); /* the space, which is the first glyph of every face */
    }

    return ((uint16_t)(code - fontFirstGlyph));
}

void TextAtlas::uploadFaces(const FontFace *faces, uint8_t count)
{
    for (uint8_t f = 0; f < count; f++) {
        const FontFace &face = faces[f];

        /* A band has one ink on one field, so a coverage value can only ever
         * come out as one of sixteen colours. Mixing them once a band rather
         * than once a pixel takes the blending out of the inner loop.
         *
         * Eight rows, where the generator emits six - the preset colours -
         * and three. */
        uint16_t ramp[8][16];

        for (uint8_t b = 0; b < face.numBands; b++) {
            for (uint8_t level = 0; level < 16; level++) {
                ramp[b][level] = blend(face.band[b].ink,
                                       face.band[b].field,
                                       (uint8_t)(level * 17));
            }
        }

        for (uint16_t line = face.row; line < (face.row + face.rows); line++) {
            /* The gaps between the boxes are never read - a blit takes a
             * glyph's own box and nothing around it - so what is in them only
             * has to be something. */
            memset(atlasRow, 0, sizeof(atlasRow));

            for (uint8_t b = 0; b < face.numBands; b++) {
                const FontBand &band = face.band[b];

                for (uint16_t index = 0; index < fontNumGlyphs; index++) {
                    const FontGlyph &glyph = face.glyph[index];
                    uint16_t top = band.y[index];

                    if ((glyph.height == 0) || (line < top)
                        || (line >= (top + glyph.height))) {
                        continue;
                    }

                    uint16_t stride = (uint16_t)((glyph.width + 1) / 2);
                    const uint8_t *source = face.coverage
                                            + face.coverageAt[index]
                                            + (uint32_t)(line - top) * stride;
                    uint16_t *target = &atlasRow[band.x[index]];

                    for (uint8_t i = 0; i < glyph.width; i += 2) {
                        uint8_t packed = *source++;

                        target[i] = ramp[b][packed >> 4];

                        if ((i + 1) < glyph.width) {
                            target[i + 1] = ramp[b][packed & 0x0F];
                        }
                    }
                }
            }

            writeRamData(FRAME_GLYPHS + ((uint32_t)line * fontAtlasWidth * 2),
                         (const uint8_t *)atlasRow,
                         sizeof(atlasRow));
        }
    }
}

uint16_t TextAtlas::measureString(const char *text, const FontFace &face)
{
    uint16_t width = 0;

    for (uint16_t i = 0; text[i] != 0; i++) {
        width += face.glyph[glyphIndex(text[i])].advance;
    }

    return (width);
}

void TextAtlas::drawString(uint16_t x,
                           uint16_t y,
                           const char *text,
                           const FontFace &face,
                           const FontBand &band,
                           uint16_t width,
                           TextAlign align,
                           bool clip)
{
    uint32_t address = getCanvasAddress();
    uint16_t textWidth = measureString(text, face);
    uint16_t xPosition = 0;

    if (textWidth > width) {
        textWidth = width;
    }

    if (align == TextAlign::center) {
        xPosition = (width - textWidth) / 2;
    } else if (align == TextAlign::right) {
        xPosition = width - textWidth;
    }

    /* A band drawn on a background of its own has nothing to key away, and
     * says so with a colour that is not in it. */
    setBteChromaColor(band.key);

    uint16_t pen = 0;

    for (uint16_t i = 0; text[i] != 0; i++) {
        uint16_t index = glyphIndex(text[i]);
        const FontGlyph &glyph = face.glyph[index];

        if (clip && ((pen + glyph.advance) > width)) {
            break;
        }

        /* Only the ink is stored, so the blit is the size of the ink and the
         * bearings put it back where the character cell wants it. */
        if ((glyph.width != 0) && (glyph.height != 0)) {
            bteCopyChroma(FRAME_GLYPHS,
                          band.x[index],
                          band.y[index],
                          address,
                          x + xPosition + pen + glyph.left,
                          y + glyph.top,
                          glyph.width,
                          glyph.height);
        }

        pen += glyph.advance;
    }
}

void TextAtlas::drawVertical(uint16_t x,
                             uint16_t y,
                             const char *text,
                             const FontFace &face,
                             const FontBand &band)
{
    for (uint16_t i = 0; text[i] != 0; i++) {
        char character[2] = { text[i], 0 };
        drawString(
            x, y, character, face, band, face.cell, TextAlign::left, false);
        y += face.cell;
    }
}

void TextAtlas::fillTextBox(uint16_t x,
                            uint16_t y,
                            const char *text,
                            const FontFace &face,
                            uint16_t width,
                            TextAlign align,
                            uint32_t backgroundColor)
{
    uint16_t textWidth = measureString(text, face);
    uint16_t xPosition;

    if (align == TextAlign::center) {
        xPosition = (width - textWidth) / 2;
    } else if (align == TextAlign::right) {
        xPosition = width - textWidth;
    } else {
        xPosition = 0;
    }

    setForegroundColor(backgroundColor);
    fillRect(x + xPosition - 2, y - 1, textWidth + 2, face.cell + 1);
}
