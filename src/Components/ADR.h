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
 * @file ADR.h
 *
 * @brief An attack / decay / release envelope contour.
 */

#pragma once

#include "EnvelopeShape.h"
#include <algorithm>

class ADR : public EnvelopeShape
{
public:
    uint8_t getNumSegments(void) const override
    {
        return (3);
    }

    uint8_t getNumPoints(void) const override
    {
        return (5);
    }

    void computePoints(const Rectangle &bounds,
                       const std::vector<Value> &values,
                       std::vector<Point> &points,
                       int16_t &baselineY) const override
    {
        const uint16_t segmentWidth = getSegmentWidth(bounds, 3);
        const uint16_t decayLength = segmentWidth * values[decay].getRelative();
        const float factor =
            segmentWidth > 0 ? (float)decayLength / (float)segmentWidth : 0.0f;

        baselineY = bounds.getHeight() - 1;

        // Origin
        points[0].x = 0;
        points[0].y = baselineY;

        // Where the attack rises from
        points[1].x = 0;
        points[1].y = baselineY;

        // Attack
        points[2].x = points[1].x + segmentWidth * values[attack].getRelative();
        points[2].y = 0;

        // Decay
        points[3].x = points[2].x + decayLength;
        points[3].y = baselineY - (baselineY * factor);

        // Release
        points[4].x =
            points[3].x + segmentWidth * values[release].getRelative();
        points[4].y = points[0].y;
    }

    static constexpr uint8_t attack = 0;
    static constexpr uint8_t decay = 1;
    static constexpr uint8_t release = 2;
};
