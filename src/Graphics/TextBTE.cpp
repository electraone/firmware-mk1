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
 * @file TextBTE.cpp
 *
 * @brief A BTE based anti-aliased texts.
 */

#include "TextBTE.h"

#include "Fonts/UiAsset.inc"

/**
 * @brief One face of the strip, and where each of its colours sits in it.
 *
 * The tables are generated beside the pixels they describe, so this is only
 * the gathering up: a style names a face and a variant, and the renderer then
 * has everything it needs to find a glyph, size the blit and place it.
 */
struct AssetFace {
    uint8_t cell;
    const uint8_t *advance;
    const uint8_t *width;
    const uint8_t *height;
    const uint8_t *left;
    const uint8_t *top;
    const uint16_t *x;
    const uint16_t *y;
};

#define ASSET_FACE(face, variant)                                              \
    {                                                                          \
        face##Cell, face##Advance, face##Width, face##Height, face##Left,      \
            face##Top, face##variant##X, face##variant##Y                      \
    }

/** Indexed by TextStyle. The user faces appear once per preset colour. */
static const AssetFace systemFaces[] = {
    ASSET_FACE(SystemSmall, White),
    ASSET_FACE(SystemMedium, White),
    ASSET_FACE(SystemMedium, BlackOnWhite),
    ASSET_FACE(SystemMedium, WhiteOnDimmed),
};

static const AssetFace userSmallFaces[] = {
    ASSET_FACE(UserSmall, White),  ASSET_FACE(UserSmall, Red),
    ASSET_FACE(UserSmall, Orange), ASSET_FACE(UserSmall, Blue),
    ASSET_FACE(UserSmall, Green),  ASSET_FACE(UserSmall, Purple),
};

static const AssetFace userMediumFaces[] = {
    ASSET_FACE(UserMedium, White),  ASSET_FACE(UserMedium, Red),
    ASSET_FACE(UserMedium, Orange), ASSET_FACE(UserMedium, Blue),
    ASSET_FACE(UserMedium, Green),  ASSET_FACE(UserMedium, Purple),
};

static const AssetFace userLargeFaces[] = {
    ASSET_FACE(UserLarge, White),  ASSET_FACE(UserLarge, Red),
    ASSET_FACE(UserLarge, Orange), ASSET_FACE(UserLarge, Blue),
    ASSET_FACE(UserLarge, Green),  ASSET_FACE(UserLarge, Purple),
};

/** How wide a space is, per face. It has no ink so it is not in the strip. */
static uint8_t spaceWidth(const AssetFace &face)
{
    return ((uint8_t)(face.cell / 2));
}

/**
 * @brief The face a style asks for, in the colour the caller asked for.
 *
 * A system style ignores the colour: the system UI draws on backgrounds it
 * chose itself, and the variant is part of the style. A user style takes one
 * of the six colours a preset can use, which is what the six bands are for.
 */
static const AssetFace &faceFor(TextStyle style, uint8_t colour)
{
    if (colour >= 6) {
        colour = 0;
    }

    switch (style) {
        case TextStyle::userSmall:
            return (userSmallFaces[colour]);
        case TextStyle::userMedium:
            return (userMediumFaces[colour]);
        case TextStyle::userLarge:
            return (userLargeFaces[colour]);
        default:
            break;
    }

    return (systemFaces[(uint8_t)style]);
}

/** Where a character sits in the tables, or -1 if the strip has no ink for
 *  it. Space and anything outside the printable range come back as -1. */
static int16_t glyphIndex(char character)
{
    uint8_t code = (uint8_t)character;

    if ((code < uiAssetFirstGlyph) || (code > uiAssetLastGlyph)) {
        return (-1);
    }

    return ((int16_t)(code - uiAssetFirstGlyph));
}

TextBTE::TextBTE(const uint8_t CSp, const uint8_t RSTp) : FrameBuffer(CSp, RSTp)
{
}

void TextBTE::printText(uint16_t x,
                        uint16_t y,
                        const char *text,
                        TextStyle textStyle,
                        uint16_t width,
                        TextAlign align,
                        uint8_t color)
{
    const AssetFace &face = faceFor(textStyle, color);
    uint32_t address = getCanvasAddress();
    uint16_t textWidth = getTextWidth(text, textStyle);
    uint16_t xPosition = 0;

    if (textWidth > width) {
        textWidth = width;
    }

    if (align == TextAlign::center) {
        xPosition = (width - textWidth) / 2;
    } else if (align == TextAlign::right) {
        xPosition = width - textWidth;
    }

    /* Every glyph is keyed against the same colour, whichever variant it came
     * from. A variant that carries its own background has none of that colour
     * in its box, so the same blit copies it whole. */
    setBteChromaColor(uiAssetChromaKey);

    uint16_t pen = 0;

    for (uint16_t i = 0; text[i] != 0; i++) {
        int16_t index = glyphIndex(text[i]);

        if (index < 0) {
            pen += spaceWidth(face);
            continue;
        }

        uint16_t advance = (uint16_t)face.advance[index] + 1;

        // Do not print outside of the bounding box
        if ((pen + advance) > width) {
            break;
        }

        /* Only the ink is stored, so the blit is the size of the ink and the
         * bearing puts it back where the character cell wants it. */
        bteCopyChroma(FRAME_UI_TOOLKIT,
                      face.x[index],
                      face.y[index],
                      address,
                      x + xPosition + pen + face.left[index],
                      y + face.top[index],
                      face.width[index],
                      face.height[index]);

        pen += advance;
    }
}

uint16_t TextBTE::getTextWidth(const char *text, TextStyle textStyle)
{
    const AssetFace &face = faceFor(textStyle, 0);
    uint16_t width = 0;

    for (uint16_t i = 0; text[i] != 0; i++) {
        int16_t index = glyphIndex(text[i]);

        width += (index < 0) ? spaceWidth(face)
                             : (uint16_t)face.advance[index] + 1;
    }

    return (width);
}

void TextBTE::paintTextPlaceHolder(uint16_t x,
                                   uint16_t y,
                                   const char *text,
                                   TextStyle style,
                                   uint16_t width,
                                   TextAlign align,
                                   uint32_t backgroundColor)
{
    uint16_t xPosition;
    uint16_t textWidth = getTextWidth(text, style);
    uint8_t charHeight = faceFor(style, 0).cell;

    if (align == TextAlign::center) {
        xPosition = (width - textWidth) / 2;
    } else if (align == TextAlign::right) {
        xPosition = width - textWidth;
    } else {
        xPosition = 0;
    }

    setForegroundColor(backgroundColor);
    fillRect(x + xPosition - 2, y - 1, textWidth + 2, charHeight + 1);
}

// Private

