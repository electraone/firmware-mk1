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
