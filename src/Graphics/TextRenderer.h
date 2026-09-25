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
 * @file TextRenderer.h
 *
 * @brief What every way of drawing text has to be able to do.
 *
 * The controller has more than one text implementation because text is asked
 * for under conditions that pull in opposite directions - a preset's text
 * wants any size, weight and colour, the system's own text wants to be cheap
 * and is always the same two faces. There is no reason to expect that to be
 * the last word, so the components draw through this rather than through a
 * particular renderer.
 *
 * The interface is the intersection, expressed in TextStyle: a name for a
 * (size, face) pair that every implementation has to be able to honour. An
 * implementation that can do more - TextUser can take a size, a face, a
 * spacing and a colour - offers that on its own type, and whoever needs it
 * holds that type instead of this one.
 *
 * A style means the same size and weight everywhere, so its text sits on the
 * same rows in both renderers. It is not the same raster: TextSystem keeps the
 * Arial Bold the system UI was laid out around, and TextUser draws the Open
 * Sans label face a preset's controls are read in - same cell, same baseline,
 * narrower advances.
 *
 * To add an implementation: derive from this and from VideoRam, implement the
 * six calls, and hand it out from Graphics.
 */

#pragma once

#include <cstdint>

#include "TextBase.h"

class TextRenderer
{
public:
    virtual ~TextRenderer() = default;

    /**
     * Puts whatever the implementation draws from into video RAM.
     *
     * Called once, after the display is up and before anything asks for text.
     */
    virtual void uploadFonts(void) = 0;

    /**
     * Draws a string.
     *
     * Coordinates are relative to the active window, the same as every other
     * drawing call, and y is the top of the line box. width and align
     * position the string within a box starting at x.
     *
     * colour is a request, not a guarantee: an implementation that draws from
     * pre-coloured pixels can only offer the colours it was built with, and
     * says in its own header which ones those are. Ask the implementation, not
     * this interface, before relying on it.
     */
    virtual void printText(uint16_t x,
                           uint16_t y,
                           const char *text,
                           TextStyle style,
                           uint16_t width,
                           TextAlign align = TextAlign::left,
                           uint32_t colour = 0xFFFF) = 0;

    /**
     * Lays a known background behind a string that is about to be drawn.
     *
     * Anti-aliasing has to be blended against something, so the background a
     * string is put on is the background its edges were blended towards.
     */
    virtual void paintTextPlaceHolder(uint16_t x,
                                      uint16_t y,
                                      const char *text,
                                      TextStyle style,
                                      uint16_t width,
                                      TextAlign align,
                                      uint32_t backgroundColor = 0x0000) = 0;

    /** One glyph per line, reading downwards. */
    virtual void
        printVerticalText(uint16_t x, uint16_t y, const char *text) = 0;

    /** The width the string will occupy. */
    virtual uint16_t getTextWidth(const char *text, TextStyle style) = 0;

    /** The height of one line in the given style. */
    virtual uint16_t getLineHeight(TextStyle style) = 0;
};
