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
 * @file UiCommand.cpp
 *
 * @brief The names of the things the instrument can be told to do.
 */

#include "UiCommand.h"

#include <cstring>

namespace
{
    /* One entry per UiCommand::Type, indexed by the enumerator itself, so a
     * lookup by command is a subscript rather than a search. */

    const char *const wireNames[] = {
        nullptr, /* UiCommand::Type::None */

#define E1_COMMAND(id, wire, disp, param, fn) wire,
#include "Commands.inc"
#undef E1_COMMAND
    };

    const char *const displayNames[] = {
        nullptr, /* UiCommand::Type::None */

#define E1_COMMAND(id, wire, disp, param, fn) disp,
#include "Commands.inc"
#undef E1_COMMAND
    };

    const char *const parameterNames[] = {
        nullptr, /* UiCommand::Type::None */

#define E1_COMMAND(id, wire, disp, param, fn) param,
#include "Commands.inc"
#undef E1_COMMAND
    };

    constexpr size_t commandCount = static_cast<size_t>(UiCommand::Type::Count);

    static_assert((sizeof(wireNames) / sizeof(wireNames[0])) == commandCount,
                  "wireNames does not cover every UiCommand::Type");
    static_assert((sizeof(displayNames) / sizeof(displayNames[0]))
                      == commandCount,
                  "displayNames does not cover every UiCommand::Type");
    static_assert((sizeof(parameterNames) / sizeof(parameterNames[0]))
                      == commandCount,
                  "parameterNames does not cover every UiCommand::Type");

    const char *lookup(const char *const *names, UiCommand::Type command)
    {
        const size_t index = static_cast<size_t>(command);
        return ((index < commandCount) ? names[index] : nullptr);
    }
} // namespace

UiCommand::Type UiCommand::translate(const char *name)
{
    if (name) {
        for (size_t i = 0; i < commandCount; i++) {
            if (wireNames[i] && (strcmp(wireNames[i], name) == 0)) {
                return (static_cast<UiCommand::Type>(i));
            }
        }
    }

    return (UiCommand::Type::None);
}

const char *UiCommand::translateToText(UiCommand::Type command)
{
    const char *name = lookup(wireNames, command);
    return (name ? name : "none");
}

const char *UiCommand::translateToDisplayName(UiCommand::Type command)
{
    const char *name = lookup(displayNames, command);
    return (name ? name : translateToText(command));
}

const char *UiCommand::translateToParameterName(UiCommand::Type command)
{
    return (lookup(parameterNames, command));
}
