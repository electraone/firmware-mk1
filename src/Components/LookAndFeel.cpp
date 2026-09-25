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
 * @file LookAndFeel.cpp
 *
 * @brief An implementation of visual style for common components.
 */

#include "LookAndFeel.h"

uint32_t LookAndFeel::backgroundColour = Colours565::black;
uint32_t LookAndFeel::altBackgroundColour = 0x0022;

/*
 * The chip's round rectangle clears one pixel at each corner when asked for
 * radius 2, and that pixel and its two neighbours along the edges at radius 3.
 * Laying the radius 2 shape down in a colour halfway to what is behind, and
 * the radius 3 shape over it in the real colour, leaves those two neighbours
 * in the blend: a two pixel corner that reads as smooth, for two commands
 * where writing the pixels by hand would take eight.
 *
 * A side too short for the radius is filled square instead, because the shape
 * then reaches past itself.
 */
void LookAndFeel::fillSmoothRoundRect(Graphics &g,
                                      uint16_t x,
                                      uint16_t y,
                                      uint16_t width,
                                      uint16_t height,
                                      uint32_t colour,
                                      uint32_t behind,
                                      uint16_t radius)
{
    const uint16_t side = (width < height) ? width : height;
    const uint16_t blendRadius = (radius > 0) ? radius - 1 : 0;

    g.setColour(colour);

    if ((radius == 0) || (side < (2 * blendRadius) + 3)) {
        g.fillRect(x, y, width, height);
        return;
    }

    g.setColour(Colours565::mix(colour, behind, smoothCornerBlend));
    g.fillRoundRect(x, y, width, height, blendRadius);
    g.setColour(colour);

    if (side >= (2 * radius) + 3) {
        g.fillRoundRect(x, y, width, height, radius);
        return;
    }

    g.fillRoundRect(x, y, width, height, blendRadius);
}

namespace
{
    /**
     * @brief One horizontal run inside a fader's track.
     *
     * Each end is rounded or square on its own, because the end a bar grows
     * from is the track's own end on a unipolar fader and a point inside it on
     * a bipolar one - square there, or the corner would show in the middle of
     * the track.
     */
    struct FaderRun {
        Graphics &g;
        uint16_t top;
        uint16_t thickness;
        uint16_t trackLength;

        void fill(uint16_t from, uint16_t length) const
        {
            g.fillRect(from, top, length, thickness);
        }

        void paint(uint16_t from,
                   uint16_t to,
                   bool roundFrom,
                   bool roundTo,
                   uint32_t colour,
                   uint32_t behind,
                   uint32_t trackColour) const
        {
            if (to <= from) {
                return;
            }

            const uint16_t shortest = LookAndFeel::smoothShortestSide;
            const uint16_t corner = LookAndFeel::smoothCornerSize;
            const uint16_t length = to - from;

            if ((!roundFrom && !roundTo) || (thickness < shortest)
                || (trackLength < shortest)) {
                g.setColour(colour);
                fill(from, length);
                return;
            }

            if (length >= shortest) {
                LookAndFeel::fillSmoothRoundRect(
                    g, from, top, length, thickness, colour, behind);

                /* A rounded shape was drawn for a run with a square end, so
                 * the square end is filled back in over its corner. */
                if (!roundFrom) {
                    g.setColour(colour);
                    fill(from, corner);
                }
                if (!roundTo) {
                    g.setColour(colour);
                    fill(to - corner, corner);
                }
                return;
            }

            /* Shorter than the corner treatment needs. A run against a
             * rounded end of the track is drawn at the shortest length that
             * rounds and then cut back to length in the track's colour, so
             * that the corner is still the track's corner. */
            if (roundFrom && (from == 0) && (length >= corner)) {
                LookAndFeel::fillSmoothRoundRect(
                    g, 0, top, shortest, thickness, colour, behind);
                g.setColour(trackColour);
                fill(from + length, shortest - length);
            } else if (roundTo && (to == trackLength) && (length >= corner)) {
                LookAndFeel::fillSmoothRoundRect(g,
                                                 trackLength - shortest,
                                                 top,
                                                 shortest,
                                                 thickness,
                                                 colour,
                                                 behind);
                g.setColour(trackColour);
                fill(trackLength - shortest, shortest - length);
            } else if ((roundFrom && (from == 0))
                       || (roundTo && (to == trackLength))) {
                /* A single column against a rounded end would stand out past
                 * the track's corner, and is too thin to read as a value. */
            } else {
                g.setColour(colour);
                fill(from, length);
            }
        }
    };
} // namespace

uint16_t LookAndFeel::knobCentre(uint16_t size)
{
    return ((size > 2) ? (uint16_t)(size / 2 - 1) : 0);
}

uint16_t LookAndFeel::knobInkHeight(uint16_t size)
{
    /* paintKnob's own arithmetic for the end of the track, and the two rows
     * the ring's outline and its mask leave under that point. */
    const uint16_t radius = knobCentre(size);
    const uint16_t yTrackStart = sinf(angleTrackStart) * (float)radius + radius;

    return (yTrackStart + 2);
}

uint16_t LookAndFeel::knobSizeForInkHeight(uint16_t inkHeight, uint16_t maxSize)
{
    uint16_t size = maxSize;

    while ((size > 1) && (knobInkHeight(size) > inkHeight)) {
        size--;
    }

    return (size);
}

void LookAndFeel::paintKnob(Graphics &g,
                            uint16_t x,
                            uint16_t y,
                            uint16_t width,
                            uint16_t height,
                            uint32_t colour,
                            int16_t min,
                            int16_t max,
                            int16_t val,
                            bool unipolar)
{
    uint32_t colourCenter = Colours565::darker(colour, 0.25f);
    uint32_t colourOutline1 = Colours565::darker(colour, 0.15f);
    uint32_t colourOutline = Colours565::darker(colour, 0.35f);
    uint32_t colourTrack = colour;

    float angle = map((float)val, min, max, angleTrackStart, angleTrackEnd);
    float angleZero = angleTrackStart;

    const bool zeroInside = !unipolar && (min < 0) && (max > 0);

    if (zeroInside) {
        angleZero = map((float)0.0f, min, max, angleTrackStart, angleTrackEnd);
    } else {
        angleZero = map((float)min, min, max, angleTrackStart, angleTrackEnd);
    }

    /* The centre one row above the middle, so the ring's top is on y rather
     * than a row below it - where every other control's graphics start. */
    uint16_t xCentre = (width / 2) + x;
    uint16_t radius = knobCentre(width);
    uint16_t yCentre = radius + y;

    // Track start and end points
    uint16_t xTrackStart = cosf(angleTrackStart) * (float)radius + xCentre;
    uint16_t yTrackStart = sinf(angleTrackStart) * (float)radius + yCentre;
    uint16_t xTrackEnd = cosf(angleTrackEnd) * (float)radius + xCentre;
    uint16_t yTrackEnd = sinf(angleTrackEnd) * (float)radius + yCentre;

    uint16_t xOrigin = cosf(angleZero) * (float)radius + xCentre;
    uint16_t yOrigin = sinf(angleZero) * (float)radius + yCentre;

    // Paint and apply the active arc
    paintKnobValue(g,
                   x,
                   y,
                   width,
                   height,
                   colourTrack,
                   xCentre,
                   yCentre,
                   radius,
                   angleZero,
                   angle);

    // Paint the outline
    g.setColour(colourOutline);
    g.drawCircle(xCentre, yCentre, radius - 1);
    g.setColour(colourOutline1);
    g.drawCircle(xCentre, yCentre, radius);

    // Mask the track
    g.setColour(Colours565::black);
    g.fillTriangle(
        xCentre, yCentre, xTrackStart, yTrackStart, xTrackEnd, yTrackEnd);
    g.fillRect(xTrackStart,
               yTrackStart,
               xTrackEnd - xTrackStart,
               height - yTrackStart + 1);

    /* The line to the origin. Where the value sits exactly on it the arc has
     * no length, and a bipolar dial would show nothing at all of where it
     * stands - so the line is the value, and takes the control's own colour
     * rather than the outline's. Only where zero is inside the range: a
     * unipolar dial draws its arc from the minimum, and one at its minimum is
     * meant to look empty. */
    g.setColour((zeroInside && (val == 0)) ? colourTrack : colourOutline);
    g.drawLine(xCentre, yCentre, xOrigin, yOrigin);

    // Mask the centre
    g.setColour(colourCenter);
    g.fillCircle(xCentre, yCentre, width / 3 - 1);
}

void LookAndFeel::paintKnobValue(Graphics &g,
                                 uint16_t x,
                                 uint16_t y,
                                 uint16_t width,
                                 uint16_t height,
                                 uint32_t colour,
                                 uint16_t xCentre,
                                 uint16_t yCentre,
                                 uint16_t radius,
                                 float angleZero,
                                 float angle)
{
    if (angleZero < angle) {
        drawKnobCurve(g,
                      x,
                      y,
                      width,
                      height,
                      colour,
                      xCentre,
                      yCentre,
                      radius,
                      angleZero,
                      angle);
    } else {
        drawKnobCurve(g,
                      x,
                      y,
                      width,
                      height,
                      colour,
                      xCentre,
                      yCentre,
                      radius,
                      angle,
                      angleZero);
    }
}

void LookAndFeel::drawKnobCurve(Graphics &g,
                                uint16_t x,
                                uint16_t y,
                                uint16_t width,
                                uint16_t height,
                                uint32_t colour,
                                uint16_t xCentre,
                                uint16_t yCentre,
                                uint16_t radius,
                                float angleStart,
                                float angleEnd)
{
    // Translate angles to coordinates
    uint16_t xStart = cosf(angleStart) * (float)radius + xCentre;
    uint16_t yStart = sinf(angleStart) * (float)radius + yCentre;
    uint16_t xEnd = cosf(angleEnd) * (float)radius + xCentre;
    uint16_t yEnd = sinf(angleEnd) * (float)radius + yCentre;

    // Find the quadrants the start and end points are in
    uint8_t quadrantStart = getKnobQuadrant(angleStart);
    uint8_t quadrantEnd = getKnobQuadrant(angleEnd);

    // Clear the curve area
    g.setColour(Colours565::black);
    g.fillRect(x, y, width, height);

    // Paint the full circle
    g.setColour(colour);
    g.fillCircle(xCentre, yCentre, radius);

    // Hide the unused segments
    for (uint8_t i = 0; i < 4; i++) {
        maskKnobUnused(g,
                       x,
                       y,
                       width,
                       height,
                       xCentre,
                       yCentre,
                       i,
                       quadrantStart,
                       quadrantEnd);
    }

    uint16_t x1 = 0;
    uint16_t y1 = 0;
    uint16_t x2 = 0;
    uint16_t y2 = 0;

    g.setColour(Colours565::black);

    // Mask the end of the curve
    if (quadrantEnd == 0) {
        x1 = x;
        y1 = yCentre;
        x2 = x;
        y2 = height - 1;
    } else if (quadrantEnd == 1) {
        x1 = xCentre;
        y1 = 0;
        x2 = x;
        y2 = 0;
    } else if (quadrantEnd == 2) {
        x1 = x + width - 1;
        y1 = yCentre;
        x2 = x + width - 1;
        y2 = 0;
    } else if (quadrantEnd == 3) {
        x1 = xCentre;
        y1 = height - 1;
        x2 = x + width - 1;
        y2 = height - 1;
    }

    maskKnobCurveSeqment(g, xCentre, yCentre, xEnd, yEnd, x1, y1, x2, y2);

    // Mask the start of the curve
    if (quadrantStart == 0) {
        x1 = xCentre;
        y1 = height - 1;
        x2 = x;
        y2 = height - 1;
    } else if (quadrantStart == 1) {
        x1 = x;
        y1 = yCentre;
        x2 = x;
        y2 = 0;
    } else if (quadrantStart == 2) {
        x1 = xCentre;
        y1 = 0;
        x2 = x + width - 1;
        y2 = 0;
    } else if (quadrantStart == 3) {
        x1 = x + width - 1;
        y1 = yCentre;
        x2 = x + width - 1;
        y2 = height - 1;
    }

    maskKnobCurveSeqment(g, xCentre, yCentre, xStart, yStart, x1, y1, x2, y2);
}

void LookAndFeel::maskKnobUnused(Graphics &g,
                                 [[maybe_unused]] uint16_t x,
                                 [[maybe_unused]] uint16_t y,
                                 uint16_t width,
                                 uint16_t height,
                                 uint16_t xCentre,
                                 uint16_t yCentre,
                                 uint8_t quadrant,
                                 uint8_t quadrantStart,
                                 uint8_t quadrantEnd)
{
    uint16_t x1 = 0;
    uint16_t y1 = 0;

    if (!((quadrant >= quadrantStart) && (quadrant <= quadrantEnd))) {
        if (quadrant == 0) {
            x1 = -(width / 2);
            y1 = height / 2;
        } else if (quadrant == 1) {
            x1 = -(width / 2);
            y1 = -(height / 2);
        } else if (quadrant == 2) {
            x1 = width / 2;
            y1 = -(height / 2);
        } else if (quadrant == 3) {
            x1 = width / 2;
            y1 = height / 2;
        }

        g.setColour(Colours565::black);
        g.fillRect(xCentre, yCentre, x1, y1);
    }
}

void LookAndFeel::maskKnobCurveSeqment(Graphics &g,
                                       uint16_t x1,
                                       uint16_t y1,
                                       uint16_t x2,
                                       uint16_t y2,
                                       uint16_t x3,
                                       uint16_t y3,
                                       uint16_t x4,
                                       uint16_t y4)
{
    g.fillTriangle(x1, y1, x2, y2, x3, y3);
    g.fillTriangle(x2, y2, x3, y3, x4, y4);
}

uint8_t LookAndFeel::getKnobQuadrant(float angle)
{
    return ((int)(floor((angle / 1.571f) - 1)) % 4);
}

void LookAndFeel::paintFaderTrack(Graphics &g,
                                  const Rectangle &bounds,
                                  uint32_t colour,
                                  uint16_t trackY,
                                  uint16_t trackHeight,
                                  int16_t barX,
                                  int16_t barWidth)
{
    const uint32_t colourTrack = Colours565::darker(colour, 0.3f);
    const int16_t width = (int16_t)bounds.getWidth();

    if (width <= 0) {
        return;
    }

    const FaderRun run{ g, trackY, trackHeight, (uint16_t)width };

    // The track the bar stands in
    run.paint(0,
              (uint16_t)width,
              true,
              true,
              colourTrack,
              backgroundColour,
              colourTrack);

    /* The bar runs between where it starts - zero, which is the track's left
     * end unless the fader is bipolar - and where the value has taken it. The
     * two ends are sorted rather than treated as a position and a length,
     * because a bipolar fader below its zero runs leftwards.
     *
     * The end it starts from is rounded only where it is the track's own end;
     * the end the value has moved to always is. */
    const int16_t zero = constrain(barX, (int16_t)0, width);
    const int16_t end =
        constrain((int16_t)(barX + barWidth), (int16_t)0, width);
    const bool zeroRounded = (zero == 0) || (zero == width);

    if (end > zero) {
        run.paint((uint16_t)zero,
                  (uint16_t)end,
                  zeroRounded,
                  true,
                  colour,
                  colourTrack,
                  colourTrack);
    } else if (end < zero) {
        run.paint((uint16_t)end,
                  (uint16_t)zero,
                  true,
                  zeroRounded,
                  colour,
                  colourTrack,
                  colourTrack);
    } else if ((zero > 0) && (zero < width)) {
        /* The value is sitting exactly on the zero of a bipolar fader, so the
         * bar has no length and the track alone says nothing about where the
         * value stands - an empty track reads as a fader at its minimum. One
         * column in the control's own colour marks the spot. Square at both
         * ends: it is nowhere near a corner of the track.
         *
         * Only where zero is inside the track. A unipolar fader starts its
         * bar at the left end, and one at its minimum is meant to look
         * empty. */
        run.paint((uint16_t)zero,
                  (uint16_t)(zero + 1),
                  false,
                  false,
                  colour,
                  colourTrack,
                  colourTrack);
    }
}

void LookAndFeel::paintBarHorizontal(Graphics &g,
                                     const Rectangle &bounds,
                                     uint32_t colour,
                                     int16_t min,
                                     int16_t max,
                                     int16_t val)
{
    uint32_t colourTrack = Colours565::darker(colour, 0.3f);

    uint16_t barHeight = bounds.getHeight() * 1.0f;
    uint16_t padding = (bounds.getHeight() - barHeight) / 2;

    uint16_t barX = map(0, min, max, 0, bounds.getWidth());
    uint16_t barWidth = map(val, min, max, 0, bounds.getWidth()) - barX;

    // Paint the track background
    g.setColour(colourTrack);
    g.fillRect(0, padding, bounds.getWidth(), barHeight);

    // Paint the active bar
    g.setColour(colour);
    g.fillRect(barX, padding, barWidth, barHeight);

    // Paint the outline
    g.drawRect(0, padding, bounds.getWidth(), barHeight);
}

void LookAndFeel::paintBarVertical(Graphics &g,
                                   const Rectangle &bounds,
                                   uint32_t colour,
                                   int16_t min,
                                   int16_t max,
                                   int16_t val)
{
    uint32_t colourTrack = Colours565::darker(colour, 0.3f);

    uint16_t barWidth = bounds.getWidth() * 1.0f;
    uint16_t padding = (bounds.getWidth() - barWidth) / 2;

    uint16_t barY = map(0, min, max, 0, bounds.getHeight());
    uint16_t barHeight = map(val, min, max, 0, bounds.getHeight()) - barY;

    // Paint the track background
    g.setColour(colourTrack);
    g.fillRect(padding, 0, barWidth, bounds.getHeight());

    // Paint the active bar
    g.setColour(colour);
    g.fillRect(padding, bounds.getHeight() - barY, barWidth, -barHeight);

    // Paint the outline
    g.drawRect(padding, 0, barWidth, bounds.getHeight());
}

void LookAndFeel::paintPad(Graphics &g,
                           const Rectangle &bounds,
                           uint32_t colour,
                           bool isMomentary,
                           bool state)
{
    uint32_t colourOff = Colours565::darker(colour, 0.3f);
    uint32_t colourAccent = colour;
    uint32_t colourOn = Colours565::darker(colour, 0.4f);
    uint16_t backgroundColour = colourOff;
    uint16_t activeBarHeight = bounds.getHeight() - 4;
    uint16_t radius = 5;

    if (state) {
        backgroundColour = colourOn;
    }

    g.setColour(Colours565::darker(colourOff, 0.5f));
    g.fillRoundRect(0, 0, bounds.getWidth(), bounds.getHeight(), radius);
    g.setColour(backgroundColour);
    g.fillRoundRect(0, 0, bounds.getWidth(), activeBarHeight, radius);

    if (!isMomentary && state) {
        g.setColour(colourAccent);
        g.fillRoundRect(0, 0, 20, activeBarHeight, radius);
        g.setColour(backgroundColour);
        g.fillRect(18, 0, 2, activeBarHeight);
        g.setColour(Colours565::black);
        g.drawLine(15, 0, 15, activeBarHeight - 1);
        g.setColour(Colours565::darker(colour, 0.15f));
        g.drawLine(16, 0, 16, activeBarHeight - 1);
        g.setColour(Colours565::darker(colour, 0.30f));
        g.drawLine(17, 0, 17, activeBarHeight - 1);
    }
}

void LookAndFeel::paintSliderHorizontal(Graphics &g,
                                        const Rectangle &bounds,
                                        uint32_t colour,
                                        int16_t min,
                                        int16_t max,
                                        int16_t val)
{
    uint32_t colourTrack = Colours565::darker(colour, 0.3f);

    uint16_t padding = (bounds.getHeight() - indicatorSize) / 2;
    uint16_t indicatorX =
        map(val, min, max, 0, bounds.getWidth() - indicatorSize);

    // Slider track
    g.setColour(colourTrack);
    g.fillRect(0, padding + indicatorRadius - 2, bounds.getWidth(), 4);

    // Indicator
    g.setColour(colour);
    g.fillElipse(
        indicatorX + indicatorRadius, padding + indicatorRadius, 11, 11);
}

void LookAndFeel::paintSliderVertical(Graphics &g,
                                      const Rectangle &bounds,
                                      uint32_t colour,
                                      int16_t min,
                                      int16_t max,
                                      int16_t val)
{
    uint32_t colourTrack = Colours565::darker(colour, 0.3f);

    uint16_t padding = (bounds.getWidth() - indicatorSize) / 2;
    uint16_t indicatorY =
        map(val, min, max, 0, bounds.getHeight() - indicatorSize);

    // Slider track
    g.setColour(colourTrack);
    g.fillRect(padding + indicatorRadius - 2, 0, 4, bounds.getHeight());

    // Indicator
    g.setColour(colour);
    g.fillElipse(padding + indicatorRadius,
                 bounds.getHeight() - indicatorY - indicatorRadius,
                 11,
                 11);
}

void LookAndFeel::paintMeterHorizontal(Graphics &g,
                                       const Rectangle &bounds,
                                       uint32_t colourActive,
                                       uint32_t colourInactive,
                                       uint32_t colourBackground,
                                       int16_t min,
                                       int16_t max,
                                       int16_t val)
{
    uint16_t barHeight = bounds.getHeight() * 1.0f;
    uint16_t padding = (bounds.getHeight() - barHeight) / 2;

    uint16_t barX = map(0, min, max, 0, bounds.getWidth());
    uint16_t barWidth = map(val, min, max, 0, bounds.getWidth()) - barX;

    // Paint the track background
    g.setColour(colourInactive);
    g.fillRect(0, padding, bounds.getWidth(), barHeight);

    // Paint the active bar
    g.setColour(colourActive);
    g.fillRect(barX, padding, barWidth, barHeight);

    // Split the bar into segments
    for (uint16_t x = 0; x < bounds.getWidth() - 2;
         x += (bounds.getWidth() / 10)) {
        g.setColour(colourBackground);
        g.fillRect(x, 0, 2, bounds.getHeight());
    }
}

void LookAndFeel::paintList(Graphics &g,
                            const Rectangle &bounds,
                            uint32_t colour,
                            const ListData *items,
                            uint16_t activeIndex)
{
    ListDataItem item = items->getByIndex(activeIndex);

    if (item.isBitmapEmpty()) {
        // Print the label
        g.user().printText(0,
                           0,
                           item.getLabel(),
                           bounds.getWidth(),
                           TextAlign::center,
                           Colours565::white,
                           TextSize::medium);
    } else {
        // display bitmap image
        uint16_t paddingBitmap = ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;
        item.paintBitmap(paddingBitmap, 0, colour);
    }
    // Paint the graphics
    if (items->getNumItems() < 16) {
        paintDots(g, bounds, colour, items, activeIndex);
    } else {
        paintBar(g, bounds, colour, items, activeIndex);
    }
}

void LookAndFeel::paintButtonList(Graphics &g,
                                  const Rectangle &bounds,
                                  uint32_t colour,
                                  const ListData *items,
                                  uint16_t activeIndex,
                                  bool active)
{
    if (active) {
        g.setColour(Colours565::darker(colour, 0.2f));
        g.fillRoundRect(0, 0, bounds.getWidth(), bounds.getHeight(), 5);
    }

    g.setColour(colour);
    g.drawRoundRect(0, 0, bounds.getWidth(), bounds.getHeight(), 5);

    ListDataItem item = items->getByIndex(activeIndex);

    if (item.isBitmapEmpty()) {
        // Print the label
        g.user().printText(0,
                           bounds.getHeight() * 0.3f,
                           item.getLabel(),
                           bounds.getWidth(),
                           TextAlign::center,
                           Colours565::white,
                           TextSize::medium);
    } else {
        // display bitmap image
        uint16_t paddingBitmap = ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;
        item.paintBitmap(
            paddingBitmap, bounds.getHeight() * 0.3f, Colours565::white);
    }
}

// Envelope -----------------------------------------------------------------

void LookAndFeel::paintEnvelope(Graphics &g,
                                const Rectangle &bounds,
                                uint32_t colour,
                                uint16_t baselineY,
                                const std::vector<Point> &points,
                                uint8_t activeSegment,
                                bool showActiveSegment)
{
    paintEnvelopeFills(
        g, bounds, colour, baselineY, points, activeSegment, showActiveSegment);
    paintEnvelopeMarkers(g, bounds, colour, baselineY, points);
    paintEnvelopeBaseline(g, bounds, colour, baselineY);
    paintEnvelopeContour(g, bounds, colour, points);
}

void LookAndFeel::paintEnvelopeContour(Graphics &g,
                                       [[maybe_unused]] const Rectangle &bounds,
                                       uint32_t colour,
                                       const std::vector<Point> &points)
{
    g.setColour(colour);

    for (uint8_t i = 0; i < std::max(0, (int)(points.size() - 1)); i += 1) {
        g.drawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
    }
}

void LookAndFeel::paintEnvelopeBaseline(Graphics &g,
                                        const Rectangle &bounds,
                                        [[maybe_unused]] uint32_t colour,
                                        uint16_t baselineY)
{
    g.setColour(Colours565::black);
    g.drawLine(0, baselineY, bounds.getWidth(), baselineY);
    g.setColour(Colours565::darkgrey);
    g.drawHorizontalDottedLine(0, bounds.getWidth(), baselineY);
}

void LookAndFeel::paintEnvelopeMarkers(Graphics &g,
                                       [[maybe_unused]] const Rectangle &bounds,
                                       [[maybe_unused]] uint32_t colour,
                                       uint16_t baselineY,
                                       const std::vector<Point> &points)
{
    g.setColour(Colours565::black);

    for (uint8_t i = 0; i < std::max(0, (int)(points.size() - 1)); i += 1) {
        g.drawLine(points[i].x, points[i].y, points[i].x, baselineY);
    }
}

void LookAndFeel::paintEnvelopeFills(Graphics &g,
                                     [[maybe_unused]] const Rectangle &bounds,
                                     uint32_t colour,
                                     uint16_t baselineY,
                                     const std::vector<Point> &points,
                                     uint8_t activeSegment,
                                     bool showActiveSegment)
{
    uint32_t darker = Colours565::darker(colour, 0.2f);

    for (uint8_t i = 0; i < std::max(0, (int)(points.size() - 1)); i++) {
        Point intersection(0, 0);

        if (showActiveSegment
            && (i == (activeSegment + 1))) { // the delay is ignored
            g.setColour(colour);
        } else {
            g.setColour(darker);
        }

        if (findIntersection(
                baselineY, points[i], points[i + 1], intersection)) {
            // The segment intersects with the baseline

            // g.setColour(darker);

            // Paint two wedges
            g.fillTriangle(points[i].x,
                           points[i].y,
                           points[i].x,
                           intersection.y,
                           intersection.x - 1,
                           intersection.y);
            g.fillTriangle(intersection.x,
                           intersection.y,
                           points[i + 1].x - 1,
                           intersection.y,
                           points[i + 1].x - 1,
                           points[i + 1].y);

        } else {
            // The segment does not intersect with the baseline
            uint8_t xi = 0;
            uint8_t yi = 0;

            // Set wedge points based on the quadrant
            if (points[i].y <= baselineY) {
                xi = (points[i].y > points[i + 1].y) ? i + 1 : i;
                yi = (points[i].y > points[i + 1].y) ? i : i + 1;
            } else {
                xi = (points[i].y > points[i + 1].y) ? i : i + 1;
                yi = (points[i].y > points[i + 1].y) ? i + 1 : i;
            }

            // g.setColour(darker);

            // Paint the wedge
            g.fillTriangle(points[i].x,
                           points[i].y,
                           points[i + 1].x - 1,
                           points[i + 1].y,
                           points[xi].x,
                           points[yi].y);

            // Paint the rectangle to fill space between wedge and the baseline
            if ((points[i].y != baselineY) && (points[i + 1].y != baselineY)) {
                g.fillRect(points[i].x,
                           points[yi].y,
                           points[i + 1].x - points[i].x,
                           baselineY - points[yi].y);
            }
        }
    }
}

bool LookAndFeel::findIntersection(uint16_t lineY,
                                   const Point &C,
                                   const Point &D,
                                   Point &intersection)
{
    // Envelope baseline represented as a1x + b1y = c1
    int a1 = 0;
    int b1 = -1024;
    int c1 = b1 * lineY;

    // Line CD represented as a2x + b2y = c2
    int a2 = D.y - C.y;
    int b2 = C.x - D.x;
    int c2 = a2 * C.x + b2 * C.y;

    int determinant = a1 * b2 - a2 * b1;

    if (determinant != 0) {
        if (!(((C.y <= lineY) && (D.y <= lineY))
              || ((C.y > lineY) && (D.y > lineY)))) {
            uint16_t x = (b2 * c1 - b1 * c2) / determinant;
            uint16_t y = (a1 * c2 - a2 * c1) / determinant;
            intersection = Point(x, y);
            return (true);
        }
    }

    return (false);
}

void LookAndFeel::paintDots(Graphics &g,
                            const Rectangle &bounds,
                            uint32_t colour,
                            const ListData *items,
                            uint16_t activeIndex)
{
    // Paint the dots
    uint8_t paddingDots =
        (bounds.getWidth() - (items->getNumItems() * 8)) / 2 + 4;
    uint16_t yPosition = 23;

    // paint dots
    for (uint16_t i = 0; i < items->getNumItems(); i++) {
        g.setColour((i == activeIndex) ? colour
                                       : Colours565::darker(colour, 0.5));
        g.fillCircle(paddingDots + i * 8, yPosition, 2);
    }
}

void LookAndFeel::paintBar(Graphics &g,
                           const Rectangle &bounds,
                           uint32_t colour,
                           const ListData *items,
                           uint16_t activeIndex)
{
    uint16_t lastItem = items->getMaxIndex();
    uint32_t dark = Colours565::darker(colour, 0.5);
    uint32_t light = Colours565::lighter(colour, 0.5);

    uint16_t fullFaderLength = bounds.getWidth() * 0.9;
    float step = (float)fullFaderLength / (float)lastItem;
    fullFaderLength += step;
    uint16_t paddingFader = (bounds.getWidth() - fullFaderLength) / 2;
    float faderLength = std::max(1.0f, abs(step * (activeIndex))) + step;
    uint16_t yPosition = 25;

    // Paint the track
    g.setColour(dark);
    g.fillRect(paddingFader, yPosition - 2, fullFaderLength, 5);

    g.setColour(backgroundColour);
    if (step > 2) {
        for (uint16_t x = step + paddingFader;
             x <= (fullFaderLength + paddingFader);
             x += step) {
            g.drawLine(x, yPosition - 2, x, yPosition + 2);
        }
    }

    // Paint the active
    g.setColour(light);
    g.fillRect(paddingFader + faderLength - step, yPosition - 2, step, 5);

    // Round the conrners
    g.setColour(Colours565::black);
    g.drawPixel(paddingFader, yPosition - 2);
    g.drawPixel(paddingFader, yPosition + 2);
    g.drawPixel(paddingFader + fullFaderLength - 1, yPosition - 2);
    g.drawPixel(paddingFader + fullFaderLength - 1, yPosition + 2);
}
