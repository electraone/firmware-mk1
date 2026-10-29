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
 * @file luaLogger.h
 *
 * @brief The logger library: formatted lines into the controller's log.
 */

#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

int luaopen_logger(lua_State *L);

/**
 * @brief Writes one formatted line to the log at LOG_LUA.
 *
 * print() sends each of its arguments as a separate log message, so a line
 * built from values needs concatenation or string.format first. This formats
 * in C++ with the conversions string.format knows - %d %i %u %c %o %x %X,
 * %a %A %e %E %f %F %g %G, %s and %% - and hands the result to
 * System::logger.write(), one message however many values go into it. The
 * line carries the "lua: " prefix a print() line does.
 */
int logger_write(lua_State *L);

static const luaL_Reg logger_functions[] = {
    { "write", logger_write },
    { NULL, NULL }
};
