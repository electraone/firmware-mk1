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

#include "luaInfo.h"
#include "MainDelegate.h"

extern MainDelegate *luaDelegate;

int luaopen_info(lua_State *L)
{
    luaL_newlib(L, info_functions);
    return 1;
}

int info_setText(lua_State *L)
{
    lua_settop(L, 1);
    const char *text = luaL_checkstring(L, 1);

    luaDelegate->setInfoText(text);

    return (0);
}
