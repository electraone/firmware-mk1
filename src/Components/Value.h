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
 * @file Value.cpp
 *
 * @brief A Value object to keep components numeric value(s).
 */

#pragma once

#include "compat.h"
#include <cmath>

class Value
{
public:
    Value() : absolute(0.0f), min(0.0f), max(1.0f), relative(0.0f)
    {
    }

    Value(float newAbsolute, float newMin, float newMax)
        : absolute(newAbsolute), min(newMin), max(newMax)
    {
        calculateRelative();
    }

    void set(float newAbsolute)
    {
        absolute = constrain(newAbsolute, min, max);
        calculateRelative();
    }

    void apply(float delta)
    {
        set(absolute + (delta * getStep()));
    }

    void setMin(float newMin)
    {
        min = newMin;
        calculateRelative();
    }

    void setMax(float newMax)
    {
        max = newMax;
        calculateRelative();
    }

    void setRange(float newMin, float newMax)
    {
        min = newMin;
        max = newMax;
        calculateRelative();
    }

    float getRelative(void) const
    {
        return (relative);
    }

    float get(void) const
    {
        return (absolute);
    }

    float getMin(void) const
    {
        return (min);
    }

    float getMax(void) const
    {
        return (max);
    }

    float getStep(void) const
    {
        return ((max - min) / 128.0f); // \todo fixed resolution for now
    }

private:
    /* The relative value is the absolute one expressed as a fraction of the
     * maximum, which is what the envelope shapes want when they turn a segment
     * value into a fraction of the segment's width. It used to be spelled as a
     * map() onto min/max .. 1, which is the same number by a longer route - but
     * that route divides by max twice over, and a range topping out at zero
     * (-64 .. 0 is a perfectly ordinary one) made it infinite, then NaN, and
     * finally an undefined cast to the integer coordinates of a contour point.
     */
    void calculateRelative(void)
    {
        constexpr float smallestUsefulMax = 1e-6f;

        relative = (fabsf(max) > smallestUsefulMax) ? (absolute / max) : 0.0f;
    }

    float absolute;
    float min;
    float max;
    float relative;
};
