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
 * @file Span.h
 *
 * @brief A view of a run of objects that live in a pool owned by someone
 *        else.
 *
 * A control's values and inputs used to be two std::vectors per control:
 * 24 bytes of handles plus two heap blocks each, for what is usually a
 * single value and a single one-byte input. The preset now keeps every value
 * and every input in one pool each, and a control holds a Span into them -
 * eight bytes, no allocation, and the same size(), operator[] and range-for
 * the vector offered, so the code that reads a control did not change.
 *
 * The pools never move an object once it is placed - see Pool.h - so a
 * Span is a plain pointer and a count.
 */

#pragma once

#include <cstddef>
#include <cstdint>

/*
 * Five bytes, packed: a pointer and a byte of count. A control carries two
 * of these, and a Span with a word-aligned pointer and a two-byte count was
 * padded to eight each - sixteen bytes a control, for runs that never exceed
 * sixteen values or twelve inputs. The Cortex-M4 reads an unaligned word in
 * one instruction, so the packing costs nothing on the path that uses it.
 */
template <typename T>
class __attribute__((packed)) Span
{
public:
    /** The longest run a Span can describe. */
    static constexpr size_t maxCount = 255;

    Span() : ptr(nullptr), count(0)
    {
    }

    Span(T *newPtr, size_t newCount)
        : ptr(newPtr), count(newCount > maxCount ? maxCount : newCount)
    {
    }

    size_t size(void) const
    {
        return (count);
    }

    bool empty(void) const
    {
        return (count == 0);
    }

    T &operator[](size_t i)
    {
        return (ptr[i]);
    }

    const T &operator[](size_t i) const
    {
        return (ptr[i]);
    }

    T *begin(void)
    {
        return (ptr);
    }

    T *end(void)
    {
        return (ptr + count);
    }

    const T *begin(void) const
    {
        return (ptr);
    }

    const T *end(void) const
    {
        return (ptr + count);
    }

    T *data(void)
    {
        return (ptr);
    }

private:
    T *ptr;
    uint8_t count;
};

static_assert(sizeof(Span<int>) == 5, "Span is meant to be five bytes");
