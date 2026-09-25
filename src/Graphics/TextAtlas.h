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
 * @file TextAtlas.h
 *
 * @brief The atlas both renderers draw out of, and how a string is put on it.
 *
 * A character is one BTE copy of an already coloured, already anti-aliased
 * box of pixels held in the display controller's memory. That is what the chip
 * is good at - a blit costs about 42 us of setting registers plus 0.09 us a
 * pixel - and it is why the mk2's layered text, three blits a glyph, was
 * measured here at three times the cost and left there.
 *
 * The colour has to be in those pixels because the chip cannot put it there on
 * the way past: its raster operations are boolean, so they can mask a glyph
 * but not modulate one, and its one blending register takes a single value for
 * a whole blit rather than a value per pixel. So a face is stored once per
 * colour it can be drawn in - six, for a preset - and text can be drawn in
 * those colours and no others.
 *
 * Once per colour in video memory, that is, not in flash. A glyph's shape does
 * not change with its colour, so what ships is the shape alone: four bits a
 * pixel of coverage. The bands are composited from that into the display's
 * memory when the unit starts. 184 KB of coverage in flash becomes 4.1 MB of
 * coloured pixels over there, which is a trade worth making in only one
 * direction.
 *
 * The atlas is composed a row at a time rather than a glyph at a time. A
 * transfer costs its 42 us whatever it carries, so 60,000 glyph sized writes
 * would take the best part of three seconds; 2,300 row sized ones take about a
 * second and a half, which is the SPI link and nothing else.
 *
 * @section masks The other atlas
 *
 * A band cannot leave a background alone. Its edge pixels were mixed against
 * the field it was composited on, so the same glyph on a fader's coloured bar
 * wears a dark halo, and the band built the other way up wears a light one -
 * neither is text, both are dirt.
 *
 * So the user faces are also kept as one bit masks, and drawn with the
 * blitter's colour expansion: the set bits take a colour given per blit and
 * the clear ones are not written at all, so there is no fringe to have been
 * mixed against anything. One blit a glyph, the same as a band, in any colour
 * on any background - and no anti-aliasing, which is what text on those
 * backgrounds had anyway, from the character generator this replaces.
 *
 * Text on black keeps the bands, because they look better and that is nearly
 * all the text there is. setTextBackgroundColour() is what chooses.
 */

#pragma once

#include "TextRenderer.h"
#include "VideoRam.h"
#include "Fonts/Fonts.h"

class TextAtlas : public TextRenderer, public virtual VideoRam
{
public:
    TextAtlas(const uint8_t CSp, const uint8_t RSTp);
    virtual ~TextAtlas() = default;

    /** Where the one bit masks sit, immediately past the coloured bands. */
    static constexpr uint32_t maskAddress =
        (uint32_t)FRAME_GLYPHS + (uint32_t)fontAtlasRows * fontAtlasWidth * 2;

protected:
    /** Composites every band of every one of the faces into the atlas. */
    void uploadFaces(const FontFace *faces, uint8_t count);

    /** Copies the faces' one bit masks into the mask atlas. */
    void uploadMasks(const FontFace *faces, uint8_t count);

    /**
     * @brief Puts a string on the screen, one blit a character.
     *
     * `clip` drops a glyph that would cross `width` rather than drawing it.
     * The system UI wants that and a preset's controls do not, which is the
     * only thing the two renderers do differently here.
     */
    void drawString(uint16_t x,
                    uint16_t y,
                    const char *text,
                    const FontFace &face,
                    const FontBand &band,
                    uint16_t width,
                    TextAlign align,
                    bool clip);

    /**
     * @brief Puts a string on the screen by expanding its masks.
     *
     * The colour is given per blit and the background is never read, so this
     * draws in any colour on anything - at the price of the anti-aliasing,
     * which cannot survive not knowing what it would be mixed with.
     */
    void drawStringExpanded(uint16_t x,
                            uint16_t y,
                            const char *text,
                            const FontFace &face,
                            uint32_t colour,
                            uint16_t width,
                            TextAlign align,
                            bool clip);

    /**
     * @brief One glyph, expanded in a colour, cropped to `width`.
     *
     * The crop is on the right only: the mask is read a byte at a time, so a
     * glyph can lose its right hand columns and not its left. Drawing it
     * whole and then again cropped, in the other colour, is what puts two
     * colours in one character.
     */
    void expandGlyph(uint32_t address,
                     const FontFace &face,
                     uint16_t index,
                     uint16_t x,
                     uint16_t y,
                     uint16_t width,
                     uint32_t colour);

    /** The width the string will occupy in the face. */
    static uint16_t measureString(const char *text, const FontFace &face);

    /** Lays a background behind a string about to be drawn in the face. */
    void fillTextBox(uint16_t x,
                     uint16_t y,
                     const char *text,
                     const FontFace &face,
                     uint16_t width,
                     TextAlign align,
                     uint32_t backgroundColor);

    /** One glyph per line, reading downwards. */
    void drawVertical(uint16_t x,
                      uint16_t y,
                      const char *text,
                      const FontFace &face,
                      const FontBand &band);

    /** Where a character sits in a face's tables. Anything the faces do not
     *  carry is drawn as a space. */
    static uint16_t glyphIndex(char character);
};
