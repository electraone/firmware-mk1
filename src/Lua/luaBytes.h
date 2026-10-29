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
 * @file luaBytes.h
 *
 * @brief The byte handling every preset that talks to a synthesiser writes.
 *
 * Building and parsing SysEx is most of what a preset for a real instrument
 * does, and there was nothing here to do it with: no seven bit packing, no
 * nibble split, no checksum, no hex. Every preset did it with string.format,
 * tables of numbers and hand written loops, differently, and the review calls
 * the result the commonest class of community preset bug.
 *
 * Every function takes its data as **a Lua string or an array of numbers**,
 * the same rule sysexBlock follows: a script that built a message with
 * string.char has a string, one that assembled it from parameters has a
 * table, and neither should have to convert before calling. Every function
 * returns an array, except toHex which returns a string - the two forms a
 * caller wants to look at.
 *
 * All of it is pure arithmetic on bytes: no display, no preset, no MIDI, so
 * it can be called from anywhere, including at load time.
 */

#pragma once

#include "luaIntegrationBase.h"

int luaopen_bytes(lua_State *L);

int bytes_toHex(lua_State *L);
int bytes_fromHex(lua_State *L);
int bytes_pack7(lua_State *L);
int bytes_unpack7(lua_State *L);
int bytes_toNibbles(lua_State *L);
int bytes_fromNibbles(lua_State *L);
int bytes_checksum(lua_State *L);
int bytes_toString(lua_State *L);

static const luaL_Reg bytes_functions[] = {
    { "toHex", bytes_toHex },
    { "fromHex", bytes_fromHex },
    { "pack7", bytes_pack7 },
    { "unpack7", bytes_unpack7 },
    { "toNibbles", bytes_toNibbles },
    { "fromNibbles", bytes_fromNibbles },
    { "checksum", bytes_checksum },
    { "toString", bytes_toString },
    { NULL, NULL }
};
