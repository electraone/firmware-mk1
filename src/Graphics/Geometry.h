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
 * @file Geometry.h
 *
 * @brief A 2D graphic drawing engine.
 */

#pragma once

#include "FrameBuffer.h"

class Geometry : public virtual FrameBuffer
{
public:
    Geometry(const uint8_t CSp, const uint8_t RSTp) : FrameBuffer(CSp, RSTp)
    {
    }

    ~Geometry() = default;

    /**
     * Sets the current drawing colour.
     *
     * @param colour in RGB565 format.
     */
    void setColour(uint32_t colour)
    {
        FrameBuffer::setForegroundColor(colour);
    }

    /**
     * Gets the current drawing colour.
     *
     * @return colour in RGB565 format.
     */
    uint32_t getColour(void)
    {
        return (FrameBuffer::getForegroundColor());
    }

    /**
     * Draws a single pixel.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     */
    void drawPixel(uint16_t x, uint16_t y)
    {
        FrameBuffer::drawPixel(x, y);
    }

    /**
     * Draws a line.
     *
     * @param x1 X coordinate of the start point.
     * @param y1 Y coordinate of the start point.
     * @param x2 X coordinate of the end point.
     * @param y2 Y coordinate of the end point.
     */
    void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
    {
        FrameBuffer::drawLine(x1, y1, x2, y2);
    }

    // \todo for the sake of simplicity, only horizontal and vertical
    //       dotted lines were added. This might get improved so that
    //       stroke types and all types of lines are supported

    /**
     * Draws a rectangular shape.
     *
     * @param x X coordinate of the top-left corner.
     * @param y Y coordinate of the top-left corner.
     * @param width width of the rectangle.
     * @param height height of the rectangle.
     */
    void drawRect(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height)
    {
        FrameBuffer::drawRect(x1, y1, width, height);
    }

    /**
     * Fills a rectangular shape.
     *
     * @param x X coordinate of the top-left corner.
     * @param y Y coordinate of the top-left corner.
     * @param width width of the rectangle.
     * @param height height of the rectangle.
     */
    void fillRect(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height)
    {
        FrameBuffer::fillRect(x1, y1, width, height);
    }

    /**
     * @brief Fills a rectangular shape.
     * 
     * @param rect a rectangular area to fill.
     */
    void fillRect(const Rectangle &rect)
    {
        FrameBuffer::fillRect(
            rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
    }

    /**
     * Draws a rectangular shape with rounded corners.
     *
     * @param x X coordinate of the top-left corner.
     * @param y Y coordinate of the top-left corner.
     * @param width width of the rectangle.
     * @param height height of the rectangle.
     */
    void drawRoundRect(uint16_t x1,
                       uint16_t y1,
                       uint16_t width,
                       uint16_t height,
                       uint16_t radius)
    {
        FrameBuffer::drawRoundRect(x1, y1, width, height, radius);
    }

    /**
     * Fills a rectangular shape with rounded corners.
     *
     * @param x X coordinate of the top-left corner.
     * @param y Y coordinate of the top-left corner.
     * @param width width of the rectangle.
     * @param height height of the rectangle.
     */
    void fillRoundRect(uint16_t x1,
                       uint16_t y1,
                       uint16_t width,
                       uint16_t height,
                       uint16_t radius)
    {
        FrameBuffer::fillRoundRect(x1, y1, width, height, radius);
    }

    /**
     * Draws a triangular shape.
     *
     * @param x1 X coordinate of the first corner.
     * @param y1 Y coordinate of the first corner.
     * @param x2 X coordinate of the second corner.
     * @param y2 Y coordinate of the second corner.
     * @param x3 X coordinate of the third corner.
     * @param y3 Y coordinate of the third corner.
     */
    void drawTriangle(uint16_t x1,
                      uint16_t y1,
                      uint16_t x2,
                      uint16_t y2,
                      uint16_t x3,
                      uint16_t y3)
    {
        FrameBuffer::drawTriangle(x1, y1, x2, y2, x3, y3);
    }

    /**
     * Fills a triangular shape.
     *
     * @param x1 X coordinate of the first corner.
     * @param y1 Y coordinate of the first corner.
     * @param x2 X coordinate of the second corner.
     * @param y2 Y coordinate of the second corner.
     * @param x3 X coordinate of the third corner.
     * @param y3 Y coordinate of the third corner.
     */
    void fillTriangle(uint16_t x1,
                      uint16_t y1,
                      uint16_t x2,
                      uint16_t y2,
                      uint16_t x3,
                      uint16_t y3)
    {
        FrameBuffer::fillTriangle(x1, y1, x2, y2, x3, y3);
    }

    /**
     * Draws a circle shape.
     *
     * @param centreX X coordinate of the circle's centre.
     * @param centreY Y coordinate of the circle's centre.
     * @param radius radius of the circle.
     */
    void drawCircle(uint16_t centreX, uint16_t centreY, uint16_t radius)
    {
        FrameBuffer::drawCircle(centreX, centreY, radius);
    }

    /**
     * Fills a circle shape.
     *
     * @param centreX X coordinate of the circle's centre.
     * @param centreY Y coordinate of the circle's centre.
     * @param radius radius of the circle.
     */
    void fillCircle(uint16_t centreX, uint16_t centreY, uint16_t radius)
    {
        FrameBuffer::fillCircle(centreX, centreY, radius);
    }

    /**
     * Draws an eliptical shape.
     *
     * @param centreX X coordinate of the ellipse's centre.
     * @param centreY Y coordinate of the ellipse's centre.
     * @param radiusX radius of the ellipse on the X axis.
     * @param radiusY radius of the ellipse on the Y axis.
     */
    void drawElipse(uint16_t centreX,
                    uint16_t centreY,
                    uint16_t radiusX,
                    uint16_t radiusY)
    {
        FrameBuffer::drawElipse(centreX, centreY, radiusX, radiusY);
    }

    /**
     * Fills an eliptical shape.
     *
     * @param centreX X coordinate of the ellipse's centre.
     * @param centreY Y coordinate of the ellipse's centre.
     * @param radiusX radius of the ellipse on the X axis.
     * @param radiusY radius of the ellipse on the Y axis.
     */
    void fillElipse(uint16_t centreX,
                    uint16_t centreY,
                    uint16_t radiusX,
                    uint16_t radiusY)
    {
        FrameBuffer::fillElipse(centreX, centreY, radiusX, radiusY);
    }

    // \todo curve drawing functions are missing and parameters do not look ok.
    // \todo segment needs to be an enum.
    /**
     * Fills a segment of a circular shape.
     *
     * @param centreX X coordinate of the curve's centre.
     * @param centreY Y coordinate of the curve's centre.
     * @param radius radius of the curve.
     * @param segment a segment to draw.
     */
    void fillCurve(uint16_t centreX,
                   uint16_t centreY,
                   uint16_t radius,
                   uint8_t segment)
    {
        FrameBuffer::fillCurve(centreX, centreY, radius, segment);
    }

    /**
     * Fills area of current graphical object (eg. component) with given colour.
     *
     * @param colour colour in RGB565 format.
     */
    void fillAll(uint32_t colour)
    {
        uint32_t originalColour = FrameBuffer::getForegroundColor();
        FrameBuffer::setForegroundColor(colour);
        FrameBuffer::fillRect(0,
                              0,
                              FrameBuffer::getActiveWindowWidth(),
                              FrameBuffer::getActiveWindowHeight());
        FrameBuffer::setForegroundColor(originalColour);
    }

    /**
     * Fills area of current graphical object (eg. component) with the
     * current colour.
     *
     * @see setColour
     */
    void fillAll(void)
    {
        fillAll(getColour());
    }
};
