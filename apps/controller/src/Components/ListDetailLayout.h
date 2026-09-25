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
 * @file ListDetailLayout.h
 *
 * @brief Which shape a list's detail window takes, and how big it is.
 *
 * Three places have to agree about this and none of them can see the other
 * two: the window decides the bounds before the detail exists, the Detail
 * arranges the lock and the content inside those bounds, and the control
 * paints and hit-tests the items. The rule lives here so it is written once.
 *
 * Two shapes:
 *
 *   column   one column of items standing beside the control it belongs to.
 *            What every list detail here has looked like.
 *   paged    the whole screen, four columns of six, with a strip down either
 *            side that turns the page - for a list too long for a column.
 *
 * Nothing scrolls. The list used to be dragged under the finger, rendered
 * into a spare page of video memory and copied out a window at a time; every
 * item is on a page now, and a page is either on the screen or one tap away.
 */

#pragma once

#include "Control.h"
#include "Overlay.h"
#include "LookAndFeel.h"

namespace ListDetailLayout
{
    /** One item's box in the single column, and the height of its row. */
    static constexpr uint16_t columnItemHeight = 80;

    /** How many items one column of the paged grid holds. A list no longer than
 *  this is drawn as the single column. */
    static constexpr uint16_t itemsInColumn = 6;

    /** The paged grid: four of those columns, filling the screen. */
    static constexpr uint16_t gridColumns = 4;
    static constexpr uint16_t itemsInPage = gridColumns * itemsInColumn;

    /** The strips down either side of the grid: the previous page on the left,
 *  the next page - and the lock, at the bottom - on the right. */
    static constexpr uint16_t sideBarWidth = 40;

    /** How far each strip stands in from the edge of the screen: as far as the
 *  bottom bar, so the strips' outer edges and the bar's ends line up. */
    static constexpr uint16_t sideBarInset = LookAndFeel::bottomBarMarginX;

    /** The gap between one item's box and the next, in the paged grid. */
    static constexpr uint16_t gridItemGap = 20;

    /** What is left of the gap between a side strip and the item next to it once
 *  the item's own half of gridItemGap is counted, so that the gap around the
 *  grid is the same as the gap inside it. */
    static constexpr uint16_t gridSideGap = gridItemGap / 2;

    /** The lock, at the same size it has beside a short list. */
    static constexpr uint16_t lockDiameter = 80;

    /** How far the lock sits inside the bottom and the outer edge of the right
 *  hand strip. */
    static constexpr uint16_t lockMargin = 10;

    /** How far the < and > sit above the middle of their strip. A glyph centred
 *  by its ink still reads as sitting low. */
    static constexpr uint16_t sideBarGlyphLift = 5;

    /** One row of the grid, gap included, in a window of the given height. */
    inline uint16_t rowHeight(uint16_t windowHeight)
    {
        return (windowHeight / itemsInColumn);
    }

    /** The top of the strips: level with the top of the first row of items. */
    inline uint16_t sideBarTop(void)
    {
        return (gridItemGap / 2);
    }

    /** How tall the strips are: down to the bottom of a full column's last item,
 *  so the strips and a full grid start and end together. */
    inline uint16_t sideBarHeight(uint16_t windowHeight)
    {
        return ((itemsInColumn * rowHeight(windowHeight)) - gridItemGap);
    }

    /** The bottom of the strips, one past the last row they cover. */
    inline uint16_t sideBarBottom(uint16_t windowHeight)
    {
        return (sideBarTop() + sideBarHeight(windowHeight));
    }

    /**
 * @brief How many items the control's list holds.
 *
 * A list whose JSON carried no values, or one naming an overlay id that does
 * not resolve, has no list at all and answers zero.
 */
    inline uint16_t numItems(const Control &control)
    {
        if (control.values.empty()) {
            return (0);
        }

        const Overlay *overlay = control.values[0].getOverlay();

        return (overlay ? overlay->getNumItems() : 0);
    }

    /** @brief Whether this control's list is drawn as the full-screen grid. */
    inline bool isPaged(const Control &control)
    {
        return (numItems(control) > itemsInColumn);
    }
} // namespace ListDetailLayout
