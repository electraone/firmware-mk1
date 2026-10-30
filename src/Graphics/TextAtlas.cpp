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

static_assert(TextAtlas::maskAddress + ((uint32_t)fontMaskRows * fontMaskWidth)
                  <= (uint32_t)FRAME_GLYPHS_END,
              "the atlas has outgrown the region reserved for it - drop a "
              "face in tools/makefont, or move what is above it and take "
              "another page");

namespace
{
    /** One row of the atlas, built in RAM and sent over in a single transfer.
     *  Below the SRAM bank boundary, as everything in .bss.lowram is. */
    __attribute__((section(".bss.lowram"))) uint16_t atlasRow[fontAtlasWidth];

    /** The same for the mask atlas, whose row is bytes rather than pixels. */
    __attribute__((section(".bss.lowram"))) uint8_t maskRow[fontMaskWidth];

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

void (*TextAtlas::lineUploaded)(void) = nullptr;

void TextAtlas::setLineUploadedCallback(void (*callback)(void))
{
    lineUploaded = callback;
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

            if (lineUploaded) {
                lineUploaded();
            }
        }
    }
}

void TextAtlas::uploadMasks(const FontFace *faces, uint8_t count)
{
    for (uint16_t line = 0; line < fontMaskRows; line++) {
        memset(maskRow, 0, sizeof(maskRow));

        for (uint8_t f = 0; f < count; f++) {
            const FontFace &face = faces[f];

            if (face.mask == nullptr) {
                continue;
            }

            for (uint16_t index = 0; index < fontNumGlyphs; index++) {
                const FontGlyph &glyph = face.glyph[index];
                uint16_t top = face.maskY[index];

                if ((glyph.height == 0) || (line < top)
                    || (line >= (top + glyph.height))) {
                    continue;
                }

                uint16_t stride = (uint16_t)((glyph.width + 7) / 8);

                memcpy(&maskRow[face.maskX[index]],
                       face.mask + face.maskAt[index]
                           + (uint32_t)(line - top) * stride,
                       stride);
            }
        }

        writeRamData(maskAddress + ((uint32_t)line * fontMaskWidth),
                     maskRow,
                     sizeof(maskRow));

        if (lineUploaded) {
            lineUploaded();
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

void TextAtlas::drawStringExpanded(uint16_t x,
                                   uint16_t y,
                                   const char *text,
                                   const FontFace &face,
                                   uint32_t colour,
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

    setExpansionKey(colour, colour);

    uint16_t pen = 0;

    for (uint16_t i = 0; text[i] != 0; i++) {
        uint16_t index = glyphIndex(text[i]);
        const FontGlyph &glyph = face.glyph[index];

        if (clip && ((pen + glyph.advance) > width)) {
            break;
        }

        expandGlyph(address,
                    face,
                    index,
                    x + xPosition + pen + glyph.left,
                    y + glyph.top,
                    glyph.width,
                    colour);

        pen += glyph.advance;
    }
}

void TextAtlas::setExpansionKey(uint32_t ink, uint32_t other)
{
    /* Three to choose from and two inks to avoid, so one of them is always
     * free. None of the three is a colour a preset can ask for. */
    static const uint16_t candidates[] = { 0xF81F, 0x07E0, 0x001F };

    for (uint8_t i = 0; i < 3; i++) {
        if ((candidates[i] != (uint16_t)ink)
            && (candidates[i] != (uint16_t)other)) {
            setBteChromaColor(candidates[i]);
            return;
        }
    }
}

void TextAtlas::expandGlyph(uint32_t address,
                            const FontFace &face,
                            uint16_t index,
                            uint16_t x,
                            uint16_t y,
                            uint16_t width,
                            uint32_t colour)
{
    const FontGlyph &glyph = face.glyph[index];

    if ((width == 0) || (glyph.width == 0) || (glyph.height == 0)) {
        return;
    }

    if (width > glyph.width) {
        width = glyph.width;
    }

    /* The source x is a byte and not a pixel: the expansion reads eight
     * pixels at a time, which is why a mask is packed to byte columns in the
     * first place. */
    bteExpansion(maskAddress,
                 face.maskX[index],
                 face.maskY[index],
                 address,
                 x,
                 y,
                 width,
                 glyph.height,
                 colour);
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
