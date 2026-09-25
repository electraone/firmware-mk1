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
 * @file ScreenCapture.h
 *
 * @brief Writes what is on the panel to a file.
 *
 * A row at a time, straight from the display's memory to the card. The mk2
 * reads the whole picture into main memory first and writes it out
 * afterwards, which it can afford; 1.2 MB is five times all the memory this
 * part has, so the two halves are interleaved here and a single row's worth
 * of buffer is all it costs.
 *
 * Nothing hands off to another thread either. On the mk2 the display belongs
 * to the LCD thread and a screenshot taken from the application thread
 * interleaved with painting on the same bus; here there is one thread, and
 * the picture cannot change under the read because nothing else runs while it
 * happens.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace ScreenCapture
{
    /** The directory a screenshot is written to, and the API serves as "tmp". */
    constexpr const char *directory = "/tmp";

    /**
 * @brief Writes the panel to a 16 bit BMP.
 *
 * @param filename an absolute path on the card
 * @return false when the file could not be written
 */
    bool save(const char *filename);

    /**
 * @brief Writes the panel to the next free scrnNNN.bmp in `directory`.
 *
 * @param filename where the name it chose is left, for the caller to log
 * @param size the size of that buffer
 */
    bool saveNext(char *filename, size_t size);

} // namespace ScreenCapture
