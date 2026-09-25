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
 * @file LookAndFeel.h
 *
 * @brief An implementation of visual style for common components.
 */

#pragma once

#include "Graphics.h"
#include "ListData.h"
#include "ControlMetrics.h"

class LookAndFeel
{
public:
    static uint32_t backgroundColour;
    static uint32_t altBackgroundColour;

    static void paintBarHorizontal(Graphics &g,
                                   const Rectangle &bounds,
                                   uint32_t colour,
                                   int16_t min,
                                   int16_t max,
                                   int16_t val);

    static void paintBarVertical(Graphics &g,
                                 const Rectangle &bounds,
                                 uint32_t colour,
                                 int16_t min,
                                 int16_t max,
                                 int16_t val);

    /**
     * @brief A fader's track, and the bar standing in it.
     *
     * `trackY` and `trackHeight` are where the track sits and how tall it is;
     * `barX` is where the bar starts, which is the left end of the track
     * unless the fader is bipolar, and `barWidth` how far the value has moved
     * it. The width is signed because a bipolar fader below zero runs
     * leftwards from its start.
     */
    static void paintFaderTrack(Graphics &g,
                                const Rectangle &bounds,
                                uint32_t colour,
                                uint16_t trackY,
                                uint16_t trackHeight,
                                int16_t barX,
                                int16_t barWidth);

    static void paintPad(Graphics &g,
                         const Rectangle &bounds,
                         uint32_t colour,
                         bool isMomentary,
                         bool state);

    static void paintSliderHorizontal(Graphics &g,
                                      const Rectangle &bounds,
                                      uint32_t colour,
                                      int16_t min,
                                      int16_t max,
                                      int16_t val);

    static void paintSliderVertical(Graphics &g,
                                    const Rectangle &bounds,
                                    uint32_t colour,
                                    int16_t min,
                                    int16_t max,
                                    int16_t val);

    static void paintMeterHorizontal(Graphics &g,
                                     const Rectangle &bounds,
                                     uint32_t colourActive,
                                     uint32_t colourInactive,
                                     uint32_t colourBackground,
                                     int16_t min,
                                     int16_t max,
                                     int16_t val);

    static void paintList(Graphics &g,
                          const Rectangle &bounds,
                          uint32_t colour,
                          const ListData *items,
                          uint16_t activeIndex);

    static void paintButtonList(Graphics &g,
                                const Rectangle &bounds,
                                uint32_t colour,
                                const ListData *items,
                                uint16_t activeIndex,
                                bool active);

    static void paintEnvelope(Graphics &g,
                              const Rectangle &bounds,
                              uint32_t colour,
                              uint16_t baselineY,
                              const std::vector<Point> &points,
                              uint8_t activeSegment,
                              bool showActiveSegment);

    static void paintDots(Graphics &g,
                          const Rectangle &bounds,
                          uint32_t colour,
                          const ListData *items,
                          uint16_t activeIndex);

    static void paintBar(Graphics &g,
                         const Rectangle &bounds,
                         uint32_t colour,
                         const ListData *items,
                         uint16_t activeIndex);

private:
    static void paintEnvelopeContour(Graphics &g,
                                     const Rectangle &bounds,
                                     uint32_t colour,
                                     const std::vector<Point> &points);

    static void paintEnvelopeBaseline(Graphics &g,
                                      const Rectangle &bounds,
                                      uint32_t colour,
                                      uint16_t baselineY);

    static void paintEnvelopeMarkers(Graphics &g,
                                     const Rectangle &bounds,
                                     uint32_t colour,
                                     uint16_t baselineY,
                                     const std::vector<Point> &points);

    static void paintEnvelopeFills(Graphics &g,
                                   const Rectangle &bounds,
                                   uint32_t colour,
                                   uint16_t baselineY,
                                   const std::vector<Point> &points,
                                   uint8_t activeSegment,
                                   bool showActiveSegment);

    static bool findIntersection(uint16_t lineY,
                                 const Point &C,
                                 const Point &D,
                                 Point &intersection);

    // Slider
    static constexpr uint8_t indicatorSize = 22;
    static constexpr uint8_t indicatorRadius = indicatorSize / 2;
};
