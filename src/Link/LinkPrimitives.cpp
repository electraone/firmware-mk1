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

#include "src/Link/LinkPrimitives.h"
#include <cstring>

namespace Link
{
    PrimitiveRecorder *activePrimitiveRecorder = nullptr;

    PrimitiveRecorder::PrimitiveRecorder(uint8_t *newBuffer,
                                         size_t newCapacity)
        : writer(newBuffer, newCapacity),
          overflowed(false),
          capacity(newCapacity)
    {
    }

    bool PrimitiveRecorder::fits(size_t opBytes)
    {
        if ((writer.getLength() + opBytes) > capacity) {
            overflowed = true;
            return (false);
        }

        return (true);
    }

    void PrimitiveRecorder::setColour(uint32_t colour)
    {
        if (fits(1 + 3)) {
            writer.writeU8((uint8_t)Primitive::SetColour);
            writer.writeU24(colour);
        }
    }

    void PrimitiveRecorder::setBackgroundColour(uint32_t colour)
    {
        if (fits(1 + 3)) {
            writer.writeU8((uint8_t)Primitive::SetBackgroundColour);
            writer.writeU24(colour);
        }
    }

    void PrimitiveRecorder::pixel(int16_t x, int16_t y)
    {
        if (fits(1 + 4)) {
            writer.writeU8((uint8_t)Primitive::Pixel);
            writer.writeI16(x);
            writer.writeI16(y);
        }
    }

    void PrimitiveRecorder::line(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
    {
        if (fits(1 + 8)) {
            writer.writeU8((uint8_t)Primitive::Line);
            writer.writeI16(x1);
            writer.writeI16(y1);
            writer.writeI16(x2);
            writer.writeI16(y2);
        }
    }

    void PrimitiveRecorder::rect(Primitive op,
                                 int16_t x,
                                 int16_t y,
                                 int16_t width,
                                 int16_t height)
    {
        if (fits(1 + 8)) {
            writer.writeU8((uint8_t)op);
            writer.writeI16(x);
            writer.writeI16(y);
            writer.writeI16(width);
            writer.writeI16(height);
        }
    }

    void PrimitiveRecorder::roundRect(Primitive op,
                                      int16_t x,
                                      int16_t y,
                                      int16_t width,
                                      int16_t height,
                                      int16_t radius)
    {
        if (fits(1 + 10)) {
            writer.writeU8((uint8_t)op);
            writer.writeI16(x);
            writer.writeI16(y);
            writer.writeI16(width);
            writer.writeI16(height);
            writer.writeI16(radius);
        }
    }

    void PrimitiveRecorder::triangle(Primitive op,
                                     int16_t x1,
                                     int16_t y1,
                                     int16_t x2,
                                     int16_t y2,
                                     int16_t x3,
                                     int16_t y3)
    {
        if (fits(1 + 12)) {
            writer.writeU8((uint8_t)op);
            writer.writeI16(x1);
            writer.writeI16(y1);
            writer.writeI16(x2);
            writer.writeI16(y2);
            writer.writeI16(x3);
            writer.writeI16(y3);
        }
    }

    void PrimitiveRecorder::circle(Primitive op,
                                   int16_t x,
                                   int16_t y,
                                   int16_t radius)
    {
        if (fits(1 + 6)) {
            writer.writeU8((uint8_t)op);
            writer.writeI16(x);
            writer.writeI16(y);
            writer.writeI16(radius);
        }
    }

    void PrimitiveRecorder::ellipse(Primitive op,
                                    int16_t x,
                                    int16_t y,
                                    int16_t radiusX,
                                    int16_t radiusY)
    {
        if (fits(1 + 8)) {
            writer.writeU8((uint8_t)op);
            writer.writeI16(x);
            writer.writeI16(y);
            writer.writeI16(radiusX);
            writer.writeI16(radiusY);
        }
    }

    void PrimitiveRecorder::curve(int16_t x,
                                  int16_t y,
                                  int16_t radius,
                                  int16_t segment)
    {
        if (fits(1 + 8)) {
            writer.writeU8((uint8_t)Primitive::Curve);
            writer.writeI16(x);
            writer.writeI16(y);
            writer.writeI16(radius);
            writer.writeI16(segment);
        }
    }

    void PrimitiveRecorder::print(int16_t x,
                                  int16_t y,
                                  const char *text,
                                  int16_t width,
                                  uint8_t align,
                                  uint8_t face,
                                  uint8_t size,
                                  uint8_t spacing)
    {
        /* The writer cuts a string at maxStringLength, so the op is sized
         * for what will actually be written rather than what was asked. */
        size_t textLength = (text != nullptr) ? strlen(text) : 0;

        if (textLength > maxStringLength) {
            textLength = maxStringLength;
        }

        if (fits(1 + 10 + 1 + textLength)) {
            writer.writeU8((uint8_t)Primitive::Print);
            writer.writeI16(x);
            writer.writeI16(y);
            writer.writeI16(width);
            writer.writeU8(align);
            writer.writeU8(face);
            writer.writeU8(size);
            writer.writeU8(spacing);
            writer.writeString((text != nullptr) ? text : "");
        }
    }

    size_t PrimitiveRecorder::getLength(void) const
    {
        return (writer.getLength());
    }

    bool PrimitiveRecorder::hasOverflowed(void) const
    {
        return (overflowed);
    }
} // namespace Link
