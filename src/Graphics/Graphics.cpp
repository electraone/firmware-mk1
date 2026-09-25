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
 * @file Graphics.cpp
 *
 * @brief A wrapper class for all graphics releated functionality.
 */

#include "Graphics.h"

Graphics::Graphics(const uint8_t CSp, const uint8_t RSTp)
    : TextSystem(CSp, RSTp),
      TextUser(CSp, RSTp),
      Geometry(CSp, RSTp),
      BitmapTools(CSp, RSTp),
      BitmapReader(CSp, RSTp),
      VideoRam(CSp, RSTp),
      FrameBuffer(CSp, RSTp)
{
}

void Graphics::initialise(void)
{
    init();
    clear();
    setBacklight(true);
    resetExtRam();
    setBacklightbrightness(512);
    delay(20);
}
