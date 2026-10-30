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
 * @file CustomValueIds.h
 *
 * @brief The ids a preset gave the values of its Custom controls.
 *
 * Every other control type has its ids in a table in flash, so a value needs
 * only a number. A Custom control's are whatever the preset wrote, and they
 * are what a script tells its values and its knobs apart by. A value has no
 * room for one and a preset of hundreds of values should not make room, so
 * they live here, packed, and a preset with no Custom controls pays nothing.
 *
 * Kept in the MCU's RAM rather than the display's: a script looks them up
 * while painting, once per value, and a VramStore read takes the SPI bus the
 * paint is using.
 */

#pragma once

#include <cstddef>
#include <cstdint>

class Control;

namespace CustomValueIds
{
    /** Longer ids are cut, which is what keeps one control's within a pool
     *  page: MaxCustomValues of these. */
    constexpr size_t maxIdLength = 31;

    /**
     * @brief Records a control's ids.
     *
     * @param packed the ids in value order, each with its terminator
     * @param size the bytes in @p packed
     * @param count how many ids that is
     */
    void set(const Control *control,
             const char *packed,
             size_t size,
             uint8_t count);

    /** The id of the control's value at @p index, or nullptr. Good for as
     *  long as the preset is loaded. */
    const char *get(const Control *control, uint8_t index);

    /** Finds the value with that id, and answers false when there is none. */
    bool find(const Control *control, const char *id, uint8_t &index);

    /** Forgets every id. Called when a preset goes away. */
    void clear(void);
} // namespace CustomValueIds
