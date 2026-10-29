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
 * @file Rectangle.h
 *
 * @brief A representation of a rectangular object.
 */

#pragma once

/**
 * @brief Represents rectangular area at given position.
 */
class Rectangle
{
public:
    Rectangle() : x(0), y(0), width(0), height(0)
    {
    }

    Rectangle(uint16_t newX,
              uint16_t newY,
              uint16_t newWidth,
              uint16_t newHeight)
        : x(newX), y(newY), width(newWidth), height(newHeight)
    {
    }

    Rectangle(const Rectangle &rect) = default;

    Rectangle &operator=(const Rectangle &rect) = default;

    /* No virtual destructor: nothing is deleted through a Rectangle pointer,
     * and the vtable it brought cost four bytes in every control and group
     * of every preset, and in every component on the screen. */
    ~Rectangle() = default;

    /**
     * Checks if the rectangle is empty.
     *
     * @return true, when the rectangle has any area.
     */
    bool isEmpty() const
    {
        return ((width == 0) || (height == 0));
    }

    /**
     * Returns X coordinate of the top-left corner.
     */
    uint16_t getX() const
    {
        return (x);
    }

    /**
     * Returns y coordinate of the top-left corner.
     */
    uint16_t getY() const
    {
        return (y);
    }

    /**
     * Returns width of the rectangle.
     */
    uint16_t getWidth() const
    {
        return (width);
    }

    /**
     * Returns height of the rectangle.
     */
    uint16_t getHeight() const
    {
        return (height);
    }

    /**
     * Returns X coordinate of the right side of the rectangle.
     */
    uint16_t getRight() const
    {
        return (x + width);
    }

    /**
     * Returns Y coordinate of the bottom side of the rectangle.
     */
    uint16_t getBottom() const
    {
        return (y + height);
    }

    /**
     * Returns X coordinate of the rectangle centre.
     */
    uint16_t getCentreX() const
    {
        return (x + (width / 2));
    }

    /**
     * Returns Y coordinate of the rectangle centre.
     */
    uint16_t getCentreY() const
    {
        return (y + (height / 2));
    }

    /**
     * Returns copy of the rectangle.
     */
    Rectangle getBounds(void) const
    {
        return (*this);
    }

    /**
     * Returns bounds relative to the its origin.
     */
    Rectangle getLocalBounds(void) const
    {
        return (Rectangle(0, 0, width, height));
    }

    /**
     * Sets the bounding boax of the rectangle using another rectangle.
     *
     * @param rect position and location given by Rectangle object.
     */
    void setBounds(const Rectangle &rect)
    {
        x = rect.x;
        y = rect.y;
        width = rect.width;
        height = rect.height;
    }

    /**
     * Sets the bounding box of the rectangle using position and size.
     *
     * @param newX a new X coordinate.
     * @param newY a new Y coordinate.
     * @param newWidth a new width of the rectangle.
     * @param newHeight a new height of the rectangle.
     */
    void setBounds(uint16_t newX,
                   uint16_t newY,
                   uint16_t newWidth,
                   uint16_t newHeight)
    {
        x = newX;
        y = newY;
        width = newWidth;
        height = newHeight;
    }

    /**
     * Sets the position of the top-left corner of the rectangle.
     *
     * @param newX a new X coordinate.
     * @param newY a new Y coordinate.
     */
    void setPosition(uint16_t newX, uint16_t newY)
    {
        x = newX;
        y = newY;
    }

    /**
     * Sets the size of the rectangle.
     *
     * @param newWidth a new width of the rectangle.
     * @param newHeight a new height of the rectangle.
     */
    void setSize(uint16_t newWidth, uint16_t newHeight)
    {
        width = newWidth;
        height = newHeight;
    };

    /**
     * Sets the new X coordinate.
     */
    void setX(uint16_t newX)
    {
        x = newX;
    }

    /**
     * Sets the new Y coordinate.
     */
    void setY(uint16_t newY)
    {
        y = newY;
    }

    /**
     * Sets the new width of the rectangle.
     */
    void setWidth(uint16_t newWidth)
    {
        width = newWidth;
    }

    /**
     * Sets the new height of the rectangle.
     */
    void setHeight(uint16_t newHeight)
    {
        height = newHeight;
    }

    /**
     * Sets the position of the rectangle's centre.
     *
     * @param newX a new X coordinate.
     * @param newY a new Y coordinate.
     */
    void setCentre(uint16_t newCentreX, uint16_t newCentreY)
    {
        x = newCentreX - (width / 2);
        y = newCentreY - (height / 2);
    }

    /**
     * Prints information about bounds
     */
    void print(uint8_t logLevel) const
    {
        /*
        System::logger.write(LOG_ERROR,
            "bounds: [%d %d %d %d]", getX(), getY(), getWidth(), getHeight());
         */
    }

private:
    struct {
        uint16_t x : 11;
        uint16_t y : 10;
        uint16_t width : 11;
        uint16_t height : 10;
    };
};
