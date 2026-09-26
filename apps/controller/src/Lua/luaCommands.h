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
 * @file luaCommands.h
 *
 * @brief The commands module: what a script can ask the instrument to do.
 */

#pragma once

#include "luaIntegration.h"

class CommandManager;

/** Set once the application has built one. */
extern CommandManager *luaCommandManager;

int luaopen_commands(lua_State *L);

int commands_run(lua_State *L);
int commands_exists(lua_State *L);
int commands_getNames(lua_State *L);

static const luaL_Reg commands_functions[] = {
    { "run", commands_run },
    { "exists", commands_exists },
    { "getNames", commands_getNames },
    { NULL, NULL }
};
