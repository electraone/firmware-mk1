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
 */

#pragma once

#include <cstdint>

struct ControlMetrics {
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

    /** The row a floating value's line box starts on, and the row a fixed one
     *  starts on. The mk2 derives both from the track and from the face the
     *  preset asked for; these are what this draws today. */
    uint16_t faderValueY;
    uint16_t faderFixedValueY;

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
    /* faderThinBarHeight  */ 9,
    /* faderThinInset      */ 36,
    /* faderBarHeight      */ 24,
    /* faderInset          */ 51,
    /* faderListForcesThin */ true,
    /* faderValueFloats    */ true,
    /* faderValueY         */ 13,
    /* faderFixedValueY    */ 0,
    /* nameInset           */ 20,
};

/* The two insets describe the same edge from opposite ends, so the gap left
 * under the track is the same whichever variant is drawn - which is what keeps
 * a fader that changes variant from moving its name. */
static_assert(controlMetrics.faderThinInset - controlMetrics.faderThinBarHeight
                  == controlMetrics.faderInset - controlMetrics.faderBarHeight,
              "the two fader variants would leave different gaps under the "
              "track, so a fader changing variant would move its name");
