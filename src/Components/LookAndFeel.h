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

    /**
     * @brief The dots under a list: one a entry, the active one lit.
     *
     * `bounds.getY()` is the centre of the circles, not their top - a circle
     * is placed by its middle and the bar below is placed by its top edge, so
     * a caller that has one bottom edge in mind has to be told which it is
     * giving. The default radius and spacing are the small marks a value-only
     * list carries; a list with a strip of its own passes its own.
     */
    static void paintDots(Graphics &g,
                          const Rectangle &bounds,
                          uint32_t colour,
                          const ListData *items,
                          uint16_t activeIndex,
                          uint16_t radius = 2,
                          uint16_t spacing = 8);

    /** The bar that stands in for the dots once they would not fit.
     *  `bounds.getY()` is its top edge. */
    static void paintBar(Graphics &g,
                         const Rectangle &bounds,
                         uint32_t colour,
                         const ListData *items,
                         uint16_t activeIndex,
                         uint16_t height = 5);

    /*
     * The corner treatment. The chip's round rectangle clears one pixel at
     * each corner at radius 2, and that pixel and its two neighbours at
     * radius 3; laying the smaller shape down in a colour halfway to what is
     * behind and the larger over it in the real colour leaves those
     * neighbours in the blend. A two pixel corner that reads as smooth, for
     * two commands where writing the pixels would take eight.
     */
    static constexpr uint16_t smoothCornerSize = 2;
    static constexpr uint16_t smoothCornerRadius = 3;
    static constexpr uint16_t smoothShortestSide = 7;
    static constexpr float smoothCornerBlend = 0.5f;

    /*
     * The dial. The track runs from below the left of the ring round to below
     * its right, which is the gap the two angles leave.
     */
    static constexpr float angleTrackStart = 2.007f;
    static constexpr float angleTrackEnd = 7.418f;

    /**
     * @brief A dial: a ring of the control's colour with an arc drawn in it.
     *
     * `unipolar` draws the arc from the minimum rather than from zero, so a
     * bipolar dial fills outwards from the middle and a unipolar one fills
     * from one end.
     */
    static void paintKnob(Graphics &g,
                          uint16_t x,
                          uint16_t y,
                          uint16_t width,
                          uint16_t height,
                          uint32_t colour,
                          int16_t min,
                          int16_t max,
                          int16_t val,
                          bool unipolar);

    /** The row the middle of a knob of that size sits on. */
    static uint16_t knobCentre(uint16_t size);

    /** How far down a knob of that size its ink reaches. */
    static uint16_t knobInkHeight(uint16_t size);

    /** The largest knob no taller than that, up to `maxSize`. */
    static uint16_t knobSizeForInkHeight(uint16_t inkHeight, uint16_t maxSize);

    /** The outline of a rounded rectangle, its corners blended both ways. */
    static void drawSmoothRoundRect(Graphics &g,
                                    uint16_t x,
                                    uint16_t y,
                                    uint16_t width,
                                    uint16_t height,
                                    uint32_t colour,
                                    uint32_t inside,
                                    uint32_t outside,
                                    uint16_t radius = smoothCornerRadius);

    /** The box a value-only control's reading sits in. */
    static void paintValueBox(Graphics &g,
                              const Rectangle &bounds,
                              uint32_t colour,
                              uint16_t boxHeight,
                              bool filled);

    /** A rounded rectangle whose corners are blended into what is behind. */
    static void fillSmoothRoundRect(Graphics &g,
                                    uint16_t x,
                                    uint16_t y,
                                    uint16_t width,
                                    uint16_t height,
                                    uint32_t colour,
                                    uint32_t behind,
                                    uint16_t radius = smoothCornerRadius);

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

    static void paintKnobValue(Graphics &g,
                               uint16_t x,
                               uint16_t y,
                               uint16_t width,
                               uint16_t height,
                               uint32_t colour,
                               uint16_t xCentre,
                               uint16_t yCentre,
                               uint16_t radius,
                               float angleZero,
                               float angle);

    static void drawKnobCurve(Graphics &g,
                              uint16_t x,
                              uint16_t y,
                              uint16_t width,
                              uint16_t height,
                              uint32_t colour,
                              uint16_t xCentre,
                              uint16_t yCentre,
                              uint16_t radius,
                              float angleStart,
                              float angleEnd);

    static void maskKnobUnused(Graphics &g,
                               uint16_t x,
                               uint16_t y,
                               uint16_t width,
                               uint16_t height,
                               uint16_t xCentre,
                               uint16_t yCentre,
                               uint8_t quadrant,
                               uint8_t quadrantStart,
                               uint8_t quadrantEnd);

    static void maskKnobCurveSeqment(Graphics &g,
                                     uint16_t x1,
                                     uint16_t y1,
                                     uint16_t x2,
                                     uint16_t y2,
                                     uint16_t x3,
                                     uint16_t y3,
                                     uint16_t x4,
                                     uint16_t y4);

    static uint8_t getKnobQuadrant(float angle);

    static bool findIntersection(uint16_t lineY,
                                 const Point &C,
                                 const Point &D,
                                 Point &intersection);

    // Slider
    static constexpr uint8_t indicatorSize = 22;
    static constexpr uint8_t indicatorRadius = indicatorSize / 2;
};
