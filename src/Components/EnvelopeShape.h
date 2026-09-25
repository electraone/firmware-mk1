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
 * @file EnvelopeShape.h
 *
 * @brief The geometry of one kind of envelope, and nothing else.
 */

#pragma once

#include "Point.h"
#include "Rectangle.h"
#include "Value.h"
#include "compat.h"
#include <cmath>
#include <cstdint>
#include <vector>

/**
 * @brief What distinguishes an ADSR from an AHDSR: a segment count and a
 *  contour.
 *
 * A shape does not know it is on a screen. It used to. Envelope derived from
 * MultiValue and so from Component, which meant ADSRControl had to inherit both
 * a control and a component to reach computePoints() - and since Component was
 * a virtual base, every event handler it inherited was ambiguous. The eight
 * forwarding overrides in each of the six envelope controls were not design;
 * they were the compiler asking which version to use, six times over.
 *
 * Shapes carry no state, so there is one instance of each for every control
 * that draws it.
 */
class EnvelopeShape
{
public:
    virtual ~EnvelopeShape() = default;

    /**
     * @brief Segments the shape carries.
     *
     * How many of the Control's values it reads, and how many handles the
     * control offers. Not the same as the number of segments it draws: an
     * ADSSR carries a break point and a slope that share one drawn span.
     */
    virtual uint8_t getNumSegments(void) const = 0;

    /** Contour points computePoints() writes, including the origin and the
     *  point the first segment rises from. */
    virtual uint8_t getNumPoints(void) const = 0;

    /**
     * @brief Lays the contour out inside the given bounds.
     *
     * @param bounds the area the contour is drawn in
     * @param values one per segment, already ranged
     * @param points written in place, getNumPoints() of them
     * @param baselineY written: where zero sits, in the same coordinates
     */
    virtual void computePoints(const Rectangle &bounds,
                               const std::vector<Value> &values,
                               std::vector<Point> &points,
                               int16_t &baselineY) const = 0;

    /**
     * @brief A display value on its way from MIDI into a segment.
     *
     * The identity for every shape but the DX7 envelope, whose rates count
     * down where the contour wants them counting up.
     *
     * @param segment which segment the value belongs to
     * @param value the translated display value
     * @param maxValue the segment's maximum
     * @return the value to store
     */
    virtual int16_t
        translateSegmentValue([[maybe_unused]] uint8_t segment,
                              int16_t value,
                              [[maybe_unused]] int16_t maxValue) const
    {
        return (value);
    }

protected:
    /**
     * @brief How wide one drawn segment is.
     *
     * The old spelling divided by the segment count plus a delay segment, but
     * the delay was never switched on - useDelay() had no callers in the tree.
     */
    static uint16_t getSegmentWidth(const Rectangle &bounds,
                                    uint8_t numDrawnSegments)
    {
        return (numDrawnSegments > 0 ? bounds.getWidth() / numDrawnSegments
                                     : 0);
    }

    /**
     * @brief map() onto a screen coordinate, with the degenerate range guarded.
     *
     * A control whose level range is a single point - min and max both zero is
     * the easy way to write one - makes map() divide by zero. The result is a
     * NaN in what becomes the integer y of a contour point, which is undefined
     * before it is ugly.
     */
    static int16_t mapLevel(float value,
                            float inMin,
                            float inMax,
                            float outMin,
                            float outMax)
    {
        constexpr float smallestUsefulRange = 1e-6f;

        if (fabsf(inMax - inMin) < smallestUsefulRange) {
            return ((int16_t)outMin);
        }
        return ((int16_t)map(value, inMin, inMax, outMin, outMax));
    }
};
