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
 * @file UiCommand.h
 *
 * @brief The names of the things the instrument can be told to do.
 *
 * Generated from Commands.inc - see the note at the top of it.
 */

#pragma once

#include <cstdint>

namespace UiCommand
{
    /**
     * @brief One command.
     *
     * The underlying type is named rather than left to the compiler because
     * this travels in the configuration's structures, and the build asks for
     * short enums: saying so here means the size does not depend on a flag.
     */
    enum class Type : uint8_t {
        None = 0,

#define E1_COMMAND(id, wire, disp, param, fn) id,
#include "Commands.inc"
#undef E1_COMMAND

        Count
    };

    /** @brief The command a name stands for, or None if nothing does. */
    Type translate(const char *name);

    /** @brief The name of a command, or "none". */
    const char *translateToText(Type command);

    /** @brief A label for a person, falling back to the name. */
    const char *translateToDisplayName(Type command);

    /**
     * @brief What the command's parameter means, or nullptr when it takes none.
     *
     * Documentation the firmware itself carries, so that a surface offering
     * commands can say what a command wants rather than the caller having to
     * look it up.
     */
    const char *translateToParameterName(Type command);

    /** @brief How many commands there are, the zero value included. */
    constexpr uint8_t count = static_cast<uint8_t>(Type::Count);

} // namespace UiCommand
