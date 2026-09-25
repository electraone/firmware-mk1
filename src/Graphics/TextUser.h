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
 * @file TextUser.h
 *
 * @brief A preset's text: sixteen faces, six colours.
 *
 * @section sizes The sixteen faces
 *
 * Each size is a separately rasterised face rather than one master scaled up
 * and down: a 12 px face and a 47 px face want different stem weights and
 * different spacing, and resampling gives neither. So is each weight, and so
 * is each spacing - monospaced is Menlo drawn for its own pitch, not the
 * proportional face advanced by the width of a W, which is what put nearly two
 * digit widths between characters. Four sizes, two weights and two spacings is
 * sixteen faces; tools/makefont builds them.
 *
 * @section colours Why six
 *
 * The chip has no per-pixel alpha - measured, not assumed - and its raster
 * operations are boolean, so a glyph can be masked on its way to the screen
 * but not tinted. The colour has to be in the pixels already, which means a
 * face is stored once per colour it can be drawn in.
 *
 * Six, then: the colours a preset can choose from. A request for anything else
 * is drawn in the nearest of them rather than refused, so a caller need not
 * know the list - but it will not get a seventh colour by asking. That is the
 * one place this differs from the mk2, where a glyph is stacked out of coverage
 * masks and can take any colour at all. Three blits a character was measured
 * here at three times the cost of one, over a link that is the thing text
 * actually costs.
 *
 * The anti-aliasing is blended towards black, because that is what the panel
 * is behind text nearly everywhere. Text on anything else is drawn from the
 * one bit masks instead - any colour, no fringe, no anti-aliasing - and
 * setTextBackgroundColour() is what chooses between the two. See TextAtlas.h.
 */

#pragma once

#include "TextAtlas.h"

class TextUser : public TextAtlas
{
public:
    TextUser(const uint8_t CSp, const uint8_t RSTp);
    virtual ~TextUser() = default;

    /** Composites the user bands into the atlas. */
    void uploadFonts(void) override;

    /**
     * Draws a string in the nearest of the six colours to the one asked for.
     *
     * Coordinates are relative to the active window, the same as every other
     * drawing call, and y is the top of the line box. width and align position
     * the string; as with the rest of the text API they do not clip it.
     */
    void printText(uint16_t x,
                   uint16_t y,
                   const char *text,
                   uint16_t width,
                   TextAlign align,
                   uint32_t colour,
                   TextSize size = TextSize::medium,
                   TextFace face = TextFace::regular,
                   TextSpacing spacing = TextSpacing::proportional);

    /**
     * @name Style based entry points
     *
     * The same shapes the components were already written against, taking a
     * TextStyle rather than a size and a face.
     * @{
     */
    void printText(uint16_t x,
                   uint16_t y,
                   const char *text,
                   TextStyle style,
                   uint16_t width,
                   TextAlign align = TextAlign::left,
                   uint32_t colour = 0xFFFF) override;
    void paintTextPlaceHolder(uint16_t x,
                              uint16_t y,
                              const char *text,
                              TextStyle style,
                              uint16_t width,
                              TextAlign align,
                              uint32_t backgroundColor = 0x0000) override;
    void printVerticalText(uint16_t x, uint16_t y, const char *text) override;
    uint16_t getTextWidth(const char *text, TextStyle style) override;
    uint16_t getLineHeight(TextStyle style) override;
    /** @} */

    /**
     * @name The colours the faces were built in
     *
     * mk1 only. There are six of them, and text asked for in anything else is
     * drawn in the nearest one - so a caller that cares what it will actually
     * get can ask, rather than finding out by looking at the panel.
     * @{
     */
    static uint8_t getNumColours(void);
    static uint32_t getColour(uint8_t index);
    /** @} */

    /** The width the string will occupy in the given face. */
    uint16_t getTextWidth(const char *text,
                          TextSize size,
                          TextFace face = TextFace::regular,
                          TextSpacing spacing = TextSpacing::proportional);

    /** The height of one line in the given size. */
    uint16_t getLineHeight(TextSize size, TextFace face = TextFace::regular);

    /**
     * @name Where the digits sit inside the line
     *
     * A line box holds ascender and descender room a number does not use - the
     * medium face is seventeen rows tall and its digits fill twelve of them -
     * so centring the box does not centre the number. These two say where the
     * ink of a digit lands inside the box, for the callers that are placing a
     * value in the middle of something rather than stacking lines.
     * @{
     */
    uint16_t getInkTop(TextSize size, TextFace face = TextFace::regular);
    uint16_t getInkHeight(TextSize size, TextFace face = TextFace::regular);
    /** @} */

    /** Lays a known background behind a string in an arbitrary face. */
    void paintTextPlaceHolder(uint16_t x,
                              uint16_t y,
                              const char *text,
                              uint16_t width,
                              TextAlign align,
                              uint32_t backgroundColor,
                              TextSize size,
                              TextFace face = TextFace::regular,
                              TextSpacing spacing = TextSpacing::proportional);

    /**
     * The colour the text is about to be drawn on.
     *
     * Black, the default, means the pre-coloured bands: they were blended
     * against black when they were composited, so that is the one background
     * their edges are right on. Anything else means the masks, which have no
     * edges to be wrong - the text is then the exact colour asked for rather
     * than the nearest of six, and flat rather than anti-aliased.
     *
     * It stays set until it is changed, so a caller that has moved off black
     * should put it back.
     */
    void setTextBackgroundColour(uint32_t colour);
    uint32_t getTextBackgroundColour(void) const;

private:
    static const FontFace &
        fontFor(TextSize size, TextFace face, TextSpacing spacing);
    static const FontBand &bandFor(const FontFace &face, uint32_t colour);

    uint32_t textBackgroundColour;
};
