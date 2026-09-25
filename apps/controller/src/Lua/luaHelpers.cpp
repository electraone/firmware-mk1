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

#include "luaHelpers.h"

int luaopen_helpers(lua_State *L)
{
    luaL_newlib(L, helpers_functions);
    return 1;
}

int helpers_slotToBounds(lua_State *L)
{
    lua_settop(L, 1);

    int slot = luaL_checkinteger(L, -1);

    luaL_argcheck(
        L, 1 <= slot && slot <= 36, 2, "failed: slot must be between 1 and 36");

    Rectangle bounds = controlSlotToBounds(slot);

    lua_newtable(L);
    luaLE_pushArrayInteger(L, 1, bounds.getX());
    luaLE_pushArrayInteger(L, 2, bounds.getY());
    luaLE_pushArrayInteger(L, 3, 158);
    luaLE_pushArrayInteger(L, 4, 56);

    return (1);
}

int helpers_boundsToSlot(lua_State *L)
{
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    // parse type
    lua_rawgeti(L, 1, 1);
    lua_rawgeti(L, 1, 2);
    lua_rawgeti(L, 1, 3);
    lua_rawgeti(L, 1, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int width = luaL_checkinteger(L, -2);
    int height = luaL_checkinteger(L, -1);

    uint8_t slot = controlBoundsToSlot(Rectangle(x, y, width, height));

    lua_pushinteger(L, slot);
    return (1);
}

int helpers_delay(lua_State *L)
{
    lua_settop(L, 1);

    int msecs = luaL_checkinteger(L, -1);

    luaL_argcheck(L,
                  1 <= msecs && msecs <= 5000,
                  2,
                  "failed: time interval must be between 1 and 5000");

    delay(msecs);

    return (0);
}