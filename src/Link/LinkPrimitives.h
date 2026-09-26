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
 * @file LinkPrimitives.h
 *
 * @brief The drawing vocabulary of a captured custom control paint.
 *
 * A custom control is painted by its preset's Lua, and the script runs only
 * on the primary - so what travels to a satellite is not the control but the
 * drawing: one op per graphics.* call the paint callback made, arguments
 * verbatim, in the control-relative coordinates the callback drew in. The
 * vocabulary is exactly the Lua graphics library's sixteen functions, which
 * is what makes the capture complete: there is nothing else a paint callback
 * can draw with.
 *
 * The recorder is armed by the capture pass and read by the Lua graphics
 * binding: while a recorder is active, every graphics call appends its op
 * here and touches no hardware at all. That is also what frees the capture
 * from the display thread - recording needs no screen, so it runs where the
 * Lua already lives.
 *
 * In src/Link beside the records, because both ends read these bytes: the
 * primary writes them and a satellite - or a computer speaking the protocol -
 * replays them. Depends on the codec only, never on the display.
 */

#pragma once

#include "src/Link/LinkCodec.h"

namespace Link
{
    /** @brief One drawing call, as the Lua graphics library names them. */
    enum class Primitive : uint8_t {
        None = 0,
        SetColour = 1,
        SetBackgroundColour = 2,
        Pixel = 3,
        Line = 4,
        Rect = 5,
        FillRect = 6,
        RoundRect = 7,
        FillRoundRect = 8,
        Triangle = 9,
        FillTriangle = 10,
        Circle = 11,
        FillCircle = 12,
        Ellipse = 13,
        FillEllipse = 14,
        Curve = 15,
        Print = 16
    };

    /**
     * @brief Records drawing calls into a caller-owned buffer.
     *
     * A thin dialect of the codec's Writer: every op is an opcode byte and
     * its arguments, little endian, so the satellite reads frames with the
     * same Reader it reads everything else with. A frame that outgrows the
     * buffer is truncated at a whole op - the overflow flag says so, and the
     * sender logs it rather than sending half an op.
     */
    class PrimitiveRecorder
    {
    public:
        PrimitiveRecorder(uint8_t *newBuffer, size_t newCapacity);

        void setColour(uint32_t colour);
        void setBackgroundColour(uint32_t colour);
        void pixel(int16_t x, int16_t y);
        void line(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
        void rect(Primitive op,
                  int16_t x,
                  int16_t y,
                  int16_t width,
                  int16_t height);
        void roundRect(Primitive op,
                       int16_t x,
                       int16_t y,
                       int16_t width,
                       int16_t height,
                       int16_t radius);
        void triangle(Primitive op,
                      int16_t x1,
                      int16_t y1,
                      int16_t x2,
                      int16_t y2,
                      int16_t x3,
                      int16_t y3);
        void circle(Primitive op, int16_t x, int16_t y, int16_t radius);
        void ellipse(Primitive op,
                     int16_t x,
                     int16_t y,
                     int16_t radiusX,
                     int16_t radiusY);
        void curve(int16_t x, int16_t y, int16_t radius, int16_t segment);
        void print(int16_t x,
                   int16_t y,
                   const char *text,
                   int16_t width,
                   uint8_t align,
                   uint8_t face,
                   uint8_t size,
                   uint8_t spacing);

        /** @brief How many bytes the frame holds. */
        size_t getLength(void) const;

        /** @brief Whether anything was cut for not fitting. */
        bool hasOverflowed(void) const;

    private:
        /**
         * @brief Whether one whole op of this many bytes still fits.
         *
         * Checked before the opcode goes in, so a frame never ends mid-op:
         * the writer would take the bytes that fit and refuse the rest, and
         * the satellite would replay a rectangle with half its geometry.
         */
        bool fits(size_t opBytes);

        Writer writer;
        bool overflowed;
        size_t capacity;
    };

    /**
     * @brief The recorder the Lua graphics binding writes to, when there is
     *  one.
     *
     * Null except inside a capture pass. Armed and cleared only while the
     * Lua state lock is held, and read only from inside Lua graphics calls -
     * which also run under that lock - so a paint on the display thread and
     * a capture on the application thread can never see each other's answer.
     */
    extern PrimitiveRecorder *activePrimitiveRecorder;
} // namespace Link
