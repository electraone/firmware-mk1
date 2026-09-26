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
 * @file ControlMetrics.h
 *
 * @brief Where a control's parts sit, as numbers with names.
 *
 * These were written into the drawing code, each one where it was used:
 * a track nine pixels tall in one place and a track thirty-six pixels up
 * from the bottom in another, with nothing to say that the two are the same
 * edge seen from opposite ends. Changing the look of a fader meant reading
 * the fader to find out what the look was.
 *
 * So they are here instead, named as the mk2 names them - `ControlMetrics` is
 * its struct, and the fields it shares mean the same thing - which is what
 * makes the two products comparable at all: a difference between them is now
 * a different number in one table rather than a different line of drawing
 * buried in a component.
 *
 * One model, so one instance. The mk2 carries a column per model because it
 * builds for three; this does not, and a second column should arrive with a
 * second model rather than before one.
 *
 * The fader's numbers are the mk2's, because the cell is: 146 by 56 on both.
 * They are not what this drew before - the track was five rows shorter and
 * started five rows lower - and the difference was nothing but two products
 * drifting apart while nobody could see both sets of numbers at once.
 */

#pragma once

#include <cstdint>

struct ControlMetrics {
    /** How far above the bottom of a control its graphics end - the row a
     *  track's bottom edge and an envelope's baseline both land on. */
    uint16_t graphicsBottomInset;

    /**
     * @brief How far above the bottom of a control the tall graphics stop.
     *
     * An envelope's contour is not a strip beside a name the way a fader's
     * track is - it is the whole drawing. It ends here, which is the row a
     * pad's box ends on, so a row of controls has one bottom edge rather than
     * two. The same as graphicsBottomInset on this model and on the mk2,
     * whose cell is the same 56 rows; a taller cell is where the two part.
     */
    uint16_t boxBottomInset;

    /** The same for a text box, which draws its box a little higher than the
     *  pad beside it. */
    uint16_t textBoxBottomInset;

    /**
     * @brief How tall the thin graphics are: a thin fader's track, a list's
     *        dots, the bar that stands in for them.
     *
     * One number so that a row of controls reads as one row rather than as
     * several things that happen to be near each other.
     */
    uint16_t stripHeight;

    /*
     * Faders
     */

    /** The track's height, and how far its top sits above the bottom of the
     *  control, for the thin variant and for the full-size one. */
    uint16_t faderThinBarHeight;
    uint16_t faderThinInset;
    uint16_t faderBarHeight;
    uint16_t faderInset;

    /** Whether a fader with an overlay to show is drawn thin whatever variant
     *  the preset asked for. */
    bool faderListForcesThin;

    /** Whether a full-size fader's value rides along with the end of the bar,
     *  or sits in a fixed place above it. */
    bool faderValueFloats;

    /**
     * @brief Whether a dial carries no name of its own.
     *
     * A cell is 146 by 56, and a dial drawn there with room kept back for a
     * name leaves a knob barely thirty pixels across. The name is the part
     * that gives: it is the same word on every control of a row, while the
     * knob is the part that has to be read from across a desk.
     */
    bool faderDialHidesName;

    /**
     * @brief The empty rows between a value's baseline and what is under it.
     *
     * A thin fader's number stands this far above its track, so that a
     * control asking for a bigger face gets it without the number landing on
     * the graphics.
     */
    uint16_t valueGap;

    /*
     * XY pads
     */

    /** Taken off the height of an XY pad. Zero here and on the mk2, whose
     *  pads end on the name baseline and whose cell is 56 rows - there is
     *  nothing to close up. */
    uint16_t xyPadHeightInset;

    /*
     * Pads
     */

    /** Taken off the height of a pad standing on its own. */
    uint16_t padHeightInset;

    /**
     * @brief How far down the control a pad's box starts.
     *
     * Only a value-only pad reads it: LookAndFeel::paintPad() draws a plain
     * pad's box from the top of the control whatever the bounds say, so this
     * is what puts the value-only pad on the same row as the pad beside it.
     */
    uint16_t padBoxTopInset;

    /** Whether a pad prints its value across the middle rather than its name
     *  underneath. */
    bool padValueCentred;

    /** How far up from the bottom a checkbox is pushed, or zero to leave it
     *  where the name-or-no-name branch put it. */
    uint16_t padCheckboxBottomInset;

    /*
     * Lists
     */

    /** Taken off a value-only list's height before its box is drawn. */
    uint16_t listButtonBoxInset;

    /**
     * @brief How far a value-only list's dots sit above the bottom of its box.
     *
     * An inset rather than a row. The mk2 names the row, 35, because its cell
     * is always 56 tall; this cell is 81, and 35 would put the dots halfway up
     * the box instead of just inside its bottom edge. Fourteen is what 35 is
     * on the mk2, measured from the end of the box rather than from the top of
     * the control - the one of the two that means the same thing on a cell of
     * any height.
     */
    uint16_t listButtonDotsInset;

    /**
     * @brief Whether a box ends level with the names beside it.
     *
     * A value-only control prints no name of its own here, so its box has the
     * whole cell. Ending it where the neighbouring faders' names end is what
     * makes the row read as one row rather than as a box that overshoots.
     */
    bool boxesEndOnNameBaseline;

    /** How far a control's name sits above the bottom of the control. */
    uint16_t nameInset;
};

/**
 * @brief The one model this firmware runs on.
 *
 * Every number here was already in the firmware, none of them is new and none
 * has been changed; collecting them is the whole of it.
 */
inline constexpr ControlMetrics controlMetrics = {
    /* graphicsBottomInset */ 27,
    /* boxBottomInset     */ 27,
    /* textBoxBottomInset */ 24,
    /* stripHeight         */ 7,
    /* faderThinBarHeight  */ 7,
    /* faderThinInset      */ 34,
    /* faderBarHeight      */ 29,
    /* faderInset          */ 56,
    /* faderListForcesThin */ true,
    /* faderValueFloats    */ true,
    /* faderDialHidesName  */ true,
    /* valueGap            */ 6,
    /* xyPadHeightInset    */ 0,
    /* padHeightInset      */ 6,
    /* padBoxTopInset      */ 3,
    /* padValueCentred     */ true,
    /* padCheckboxBottomInset */ 0,
    /* listButtonBoxInset  */ 0,
    /* listButtonDotsInset */ 14,
    /* boxesEndOnNameBaseline */ true,
    /* nameInset           */ 20,
};

/* The two insets describe the same edge from opposite ends, so the gap left
 * under the track is the same whichever variant is drawn - which is what keeps
 * a fader that changes variant from moving its name. */
static_assert(controlMetrics.faderThinInset - controlMetrics.faderThinBarHeight
                  == controlMetrics.faderInset - controlMetrics.faderBarHeight,
              "the two fader variants would leave different gaps under the "
              "track, so a fader changing variant would move its name");

static_assert(controlMetrics.faderInset - controlMetrics.faderBarHeight
                  == controlMetrics.graphicsBottomInset,
              "a fader's track would not end where every other control's "
              "graphics end");
