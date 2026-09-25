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
 * @file TextBase.h
 *
 * @brief Common data for all types of text objects.
 */

#pragma once

#if DEFAULT_BPP == 24
#define FRAME_UI_TOOLKIT 0x004B0000
#else
#define FRAME_UI_TOOLKIT 0x00258000
#endif

enum class TextAlign { left, center, right };

/**
 * @brief A face to draw with.
 *
 * These used to be the row a glyph sat on in the UI strip, which is why
 * medium was 13. The strip is packed now and a glyph is found by its own
 * position, so a style is simply a name for a family, a size and a variant.
 *
 * The family is the rule the mk2 follows and this now follows too: the
 * firmware's own windows are written in the system faces, a preset's controls
 * in the user faces. Which one a caller wants is a property of what it is
 * drawing, not of how big it wants it, so it is in the name.
 */
enum class TextStyle {
    systemSmall = 0,
    systemMedium,
    systemMediumInverse, /**< black on a white field */
    systemMediumDimmed, /**< white on a dimmed field */
    userSmall,
    userMedium,
    userLarge,

    /* The names the call sites used when there was only one family. They
     * draw with the system faces, which is what they did before, so a call
     * site that has not been looked at yet is unchanged rather than wrong. */
    smallWhiteOnBlack = systemSmall,
    smallTransparent = systemSmall,
    mediumWhiteOnBlack = systemMedium,
    mediumTransparent = systemMedium,
    mediumBlackOnWhite = systemMediumInverse,
    mediumInverseTransparent = systemMediumInverse,
    mediumWhiteOnDimmed = systemMediumDimmed,

    /* Large only ever draws a preset's values, so it is a user face. */
    largeWhiteOnBlack = userLarge,
    largeTransparent = userLarge,
};
