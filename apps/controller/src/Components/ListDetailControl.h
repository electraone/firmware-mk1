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
 * @file ListDetailControl.h
 *
 * @brief Implements an detail version of the List Control.
 */

#pragma once

#include "ListControl.h"

/**
 * @brief The detail a list opens: one column, or a page of a grid.
 *
 * Which of the two is ListDetailLayout's to say. Nothing scrolls: the list
 * used to be dragged under the finger out of an off-screen page, and every
 * item is on a page now.
 */
class ListDetailControl : public ListControl
{
public:
    ListDetailControl(const Control &control, MainDelegate &newDelegate);
    virtual ~ListDetailControl() = default;

    void paint(Graphics &g) override;
    void resized(void) override;

    void onTouchDown(const TouchEvent &touchEvent) override;
    void onTouchClick(const TouchEvent &touchEvent) override;
    void onTouchUp(const TouchEvent &touchEvent) override;
    void onTouchLongHold(const TouchEvent &touchEvent);
    void onPotTouchUp(const PotEvent &potEvent) override;
    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

private:
    /** @brief Air between an item's box and its label, either side. */
    static constexpr uint16_t itemTextMargin = 8;

    /**
     * @brief Draws one item's box, its label and its bitmap.
     *
     * The label is drawn in the list's own face - the one the preset chose
     * for its value on the page - centred on the row, and shortened with a
     * marker when that face is too wide for the box.
     */
    void paintItem(Graphics &g,
                   uint16_t itemIndex,
                   uint16_t x,
                   uint16_t y,
                   uint16_t width,
                   uint16_t height);

    /** @brief The single column, top aligned, nothing hidden. */
    void paintColumn(Graphics &g);

    /** @brief The full-screen grid, one page of it. */
    void paintGrid(Graphics &g);

    /**
     * @brief Draws one of the strips either side of the grid.
     *
     * @param g the canvas
     * @param x the strip's left edge
     * @param pointsRight which way the arrow points
     * @param enabled false when there is no page that way, and no arrow
     * @param pressed true while a finger is on it
     */
    void paintSideBar(Graphics &g,
                      uint16_t x,
                      bool pointsRight,
                      bool enabled,
                      bool pressed);

    /** @brief Which strip a point is on: -1 left, 1 right, 0 neither. */
    int8_t stripAt(uint16_t x);

    /** @brief Marks a strip as under the finger, or none, and redraws. */
    void setPressedStrip(int8_t strip);

    /** @brief The item the grid draws at this point, or -1 for none. */
    int16_t itemAt(uint16_t x, uint16_t y);

    /** @brief How many pages the list fills. Never zero. */
    uint16_t numPages(void);

    /** @brief Brings the page holding this item to the front. */
    void showPageOf(uint16_t itemIndex);

    /**
     * @brief Redraws the page, and the lock standing on top of it.
     *
     * The strips down the sides belong to this component and the lock is a
     * sibling drawn over the bottom of the right hand one. Painting a
     * component does not paint anything else - every component schedules
     * itself - so a page turn would erase the lock and leave it erased.
     */
    void repaintPage(void);

    /** @brief Takes the item as the control's value, as a touch would. */
    void chooseItem(uint16_t itemIndex);

    /** @name Grid geometry, all of it derived from the window's size @{ */
    uint16_t rowHeight(void);
    uint16_t columnWidth(void);
    uint16_t gridLeft(void);
    /** @} */

    bool valueChosen;

    /** Whether this list is drawn as the grid. Settled once, at construction:
     *  an overlay does not change length while its detail is open. */
    bool paged;

    /** The page on show, and which strip a finger is on. */
    uint16_t page;
    int8_t pressedStrip;

    uint16_t listItemWidth;
    uint16_t listItemHeight;
};
