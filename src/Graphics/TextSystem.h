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
 * @file TextSystem.h
 *
 * @brief The system UI's text: two faces, three colour schemes.
 *
 * The controller draws two kinds of text and they want different things. A
 * preset's text is whatever the preset says: any of four sizes, either weight,
 * either spacing, and it changes as the user turns a knob. The system's own
 * text - the status bar, page and preset selections, forms - is always the
 * same two faces on a background the system chose itself.
 *
 * So it is stored the way it is used. Where TextUser carries six colours of
 * every face, this carries three schemes of two: white on whatever is behind
 * it, black on a white field, and white on a dimmed one. Which is what the
 * system UI was already asking for, and a sixth of the video memory.
 *
 * The faces are Arial Bold at the two sizes the UI was laid out around. Small
 * and Medium mean the same cell and the same baseline as they do in TextUser,
 * so a component moved between the two keeps its line height - but not its
 * widths, because TextUser's are the Open Sans label face.
 */

#pragma once

#include "TextAtlas.h"

class TextSystem : public TextAtlas
{
public:
    TextSystem(const uint8_t CSp, const uint8_t RSTp);
    virtual ~TextSystem() = default;

    /** Composites the system bands into the atlas. */
    void uploadFonts(void) override;

    /**
     * Draws a string in one of the three schemes.
     *
     * `colour` names the scheme rather than mixing one: white asks for white
     * ink on whatever is behind the text, black asks for black ink - which
     * brings a white field with it, because black ink needs something to be
     * black against - and anything else asks for white ink on a dimmed field.
     * A scheme that carries its own field paints it, so the caller does not
     * have to.
     *
     * Unlike TextUser this clips: a glyph that would cross `width` is dropped
     * rather than drawn, which is what the components expect.
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

private:
    static const FontFace &fontFor(TextStyle style);
    static const FontBand &bandFor(const FontFace &face, uint32_t colour);
};
