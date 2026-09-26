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
 * @file luaCommands.cpp
 *
 * @brief The commands module: what a script can ask the instrument to do.
 */

#include "luaCommands.h"
#include "CommandManager.h"
#include "System.h"

CommandManager *luaCommandManager = nullptr;

int luaopen_commands(lua_State *L)
{
    luaL_newlib(L, commands_functions);
    return (1);
}

/*
 * commands.run("openPresetMenu") and commands.run("switchPage", 2).
 *
 * The same road a button on the panel and a host over SysEx take, so a script
 * cannot ask for anything they cannot, and none of them can ask for something
 * the others do differently.
 */
int commands_run(lua_State *L)
{
    const int numArguments = lua_gettop(L);

    const char *name = luaL_checkstring(L, 1);
    const lua_Integer parameter =
        (numArguments >= 2) ? luaL_checkinteger(L, 2) : 0;

    const UiCommand::Type command = UiCommand::translate(name);

    if (command == UiCommand::Type::None) {
        return (luaL_error(L, "run: no such command: %s", name));
    }

    if ((parameter < INT16_MIN) || (parameter > INT16_MAX)) {
        return (luaL_error(
            L, "run: %s: parameter out of range: %d", name, (int)parameter));
    }

    if (!luaCommandManager) {
        return (luaL_error(L, "run: commands are not available"));
    }

    lua_pushboolean(L, luaCommandManager->run(command, (int16_t)parameter));
    return (1);
}

/** @brief Whether a name is one the instrument knows. */
int commands_exists(lua_State *L)
{
    lua_settop(L, 1);

    const char *name = luaL_checkstring(L, 1);

    lua_pushboolean(L, UiCommand::translate(name) != UiCommand::Type::None);
    return (1);
}

/*
 * Every name that can be run, so that a script can offer them rather than
 * carry its own copy of the list and go stale.
 */
int commands_getNames(lua_State *L)
{
    lua_settop(L, 0);
    lua_newtable(L);

    int count = 0;

    for (uint8_t i = 1; i < UiCommand::count; i++) {
        const UiCommand::Type command = static_cast<UiCommand::Type>(i);

        lua_pushstring(L, UiCommand::translateToText(command));
        lua_rawseti(L, -2, ++count);
    }

    return (1);
}
