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
 * @file Graphics.h
 *
 * @brief A wrapper class for all graphics releated functionality.
 */

#pragma once

#include "Colours.h"
#include "TextRenderer.h"
#include "TextSystem.h"
#include "TextUser.h"
#include "Point.h"
#include "Rectangle.h"
#include "Geometry.h"
#include "BitmapTools.h"
#include "BitmapReader.h"
#include "VideoRam.h"

// Hardware configuration
#define RA8876_CS 5
#define RA8876_RESET 6

/**
 *
 */
class Graphics : public TextSystem,
                 public TextUser,
                 public Geometry,
                 public BitmapTools,
                 public BitmapReader,
                 public virtual VideoRam
{
public:
    Graphics(const uint8_t CSp, const uint8_t RSTp = 255);
    ~Graphics() = default;

    void initialise(void);

    /** The renderer for the controller's own interface. */
    TextRenderer &system(void)
    {
        return (*static_cast<TextSystem *>(this));
    }

    /** The renderer for anything a preset defines. */
    TextUser &user(void)
    {
        return (*static_cast<TextUser *>(this));
    }
};
