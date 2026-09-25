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

#include <cstdint>

enum class TextAlign { left = 0, center = 1, right = 2 };

/**
 * @brief The two styles the UI is written in terms of.
 *
 * These used to be the row a glyph sat on in the UI toolkit strip, which is
 * why medium was 13. They are now simply names for a face and a size: small
 * is the regular weight the strip carried, medium is the bold one.
 */
enum class TextStyle { smallTransparent = 0, mediumTransparent = 1 };

/**
 * @brief Which of the faces to draw with.
 *
 * Each size is separately rasterised at its own pixel size rather than one
 * master scaled up and down: a 12 px face and a 47 px face want different stem
 * weights and different spacing, and resampling gives neither.
 */
enum class TextSize {
    small = 0,
    medium = 1,
    large = 2,
    huge = 3,
};

/** The two weights. */
enum class TextFace {
    regular = 0,
    bold = 1,
};

/**
 * @brief How far one character moves the pen.
 *
 * Proportional gives every glyph its own width, which is what all of the UI
 * uses. Monospaced gives every glyph the same width, so a value that changes
 * does not shuffle the characters around it - a proportional readout counting
 * past 1 visibly jitters.
 *
 * It selects a face rather than widening the pen. Monospaced used to advance
 * the proportional face by its widest glyph, which is what a W or an @ costs:
 * at the Huge size the pen moved 47 px for a digit 26 px wide, and the result
 * read as loose single characters. The monospaced faces are Menlo, drawn for
 * a pitch that lands within a pixel of what the proportional digits already
 * advance by.
 */
enum class TextSpacing {
    proportional = 0,
    monospaced = 1,
};

/**
 * @brief A face: one size, one weight, one spacing.
 *
 * Three independent axes, each with its own vocabulary already - sixteen
 * combinations, which is why this is a triple and not one more enumerator.
 *
 * It is here rather than on Control because the components that draw with it
 * are in src/ and know nothing about a preset - LookAndFeel takes one to paint
 * a group's label.
 */
struct TextFont {
    TextSize size{ TextSize::medium };
    TextFace face{ TextFace::regular };
    TextSpacing spacing{ TextSpacing::proportional };
};

constexpr int numTextSizes = 4;
constexpr int numTextFaces = 2;
constexpr int numTextSpacings = 2;

/** True when the value names one of the sizes. */
inline bool isTextSize(int value)
{
    return ((value >= 0) && (value < numTextSizes));
}

/** True when the value names a face that exists. */
inline bool isTextFace(int value)
{
    return ((value >= 0) && (value < numTextFaces));
}

/** True when the value names a spacing that exists. */
inline bool isTextSpacing(int value)
{
    return ((value >= 0) && (value < numTextSpacings));
}
