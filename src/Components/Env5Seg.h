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
 * @file Env5Seg.h
 *
 * @brief A DX7-style five-segment level and rate envelope contour.
 */

#pragma once

#include "EnvelopeShape.h"
#include <algorithm>

class Env5Seg : public EnvelopeShape
{
public:
    uint8_t getNumSegments(void) const override
    {
        return (8);
    }

    uint8_t getNumPoints(void) const override
    {
        return (7);
    }

    /* A DX7 rate counts down: the larger the number, the faster the segment,
     * where the contour wants a length. Odd handles are the rates. */
    int16_t translateSegmentValue(uint8_t segment,
                                  int16_t value,
                                  int16_t maxValue) const override
    {
        return ((segment % 2) ? (int16_t)(maxValue - value) : value);
    }

    static float getMinimumLevel(const std::vector<Value> &values)
    {
        const float minLevel =
            std::min(std::min(values[level1].get(), values[level2].get()),
                     std::min(values[level3].get(), values[level4].get()));

        return (std::min(0.0f, minLevel));
    }

    static float getMaximumLevel(const std::vector<Value> &values)
    {
        const float maxLevel =
            std::max(std::max(values[level1].get(), values[level2].get()),
                     std::max(values[level3].get(), values[level4].get()));

        return (std::max(0.0f, maxLevel));
    }

    void computePoints(const Rectangle &bounds,
                       const std::vector<Value> &values,
                       std::vector<Point> &points,
                       int16_t &baselineY) const override
    {
        const uint16_t segmentWidth = getSegmentWidth(bounds, 5);
        const float maxY = bounds.getHeight() - 1;

        const float boundaryMin = getMinimumLevel(values);
        const float boundaryMax = getMaximumLevel(values);

        const int16_t level1Y = mapLevel(
            values[level1].get(), boundaryMin, boundaryMax, maxY, 0.0f);
        const int16_t level2Y = mapLevel(
            values[level2].get(), boundaryMin, boundaryMax, maxY, 0.0f);
        const int16_t level3Y = mapLevel(
            values[level3].get(), boundaryMin, boundaryMax, maxY, 0.0f);
        const int16_t level4Y = mapLevel(
            values[level4].get(), boundaryMin, boundaryMax, maxY, 0.0f);

        baselineY = mapLevel(0.0f, boundaryMin, boundaryMax, maxY, 0.0f);

        // Origin
        points[0].x = 0;
        points[0].y = level4Y;

        // Where the first segment starts from
        points[1].x = 0;
        points[1].y = level4Y;

        // Segment 1
        points[2].x = points[1].x + segmentWidth * values[rate1].getRelative();
        points[2].y = level1Y;

        // Segment 2
        points[3].x = points[2].x + segmentWidth * values[rate2].getRelative();
        points[3].y = level2Y;

        // Segment 3
        points[4].x = points[3].x + segmentWidth * values[rate3].getRelative();
        points[4].y = level3Y;

        // Segment 4
        points[5].x = points[4].x + segmentWidth;
        points[5].y = level3Y;

        // Segment 5
        points[6].x = points[5].x + segmentWidth * values[rate4].getRelative();
        points[6].y = level4Y;
    }

    static constexpr uint8_t level1 = 0;
    static constexpr uint8_t rate1 = 1;
    static constexpr uint8_t level2 = 2;
    static constexpr uint8_t rate2 = 3;
    static constexpr uint8_t level3 = 4;
    static constexpr uint8_t rate3 = 5;
    static constexpr uint8_t level4 = 6;
    static constexpr uint8_t rate4 = 7;
};
