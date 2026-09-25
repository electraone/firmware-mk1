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
 * @file luaPage.h
 *
 * @brief Implements a Lua API for managing Page.
 */

#pragma once

#include "luaIntegration.h"

int pages_get(lua_State *L);
int pages_getActive(lua_State *L);
int pages_display(lua_State *L);
void pages_onChange(uint8_t newPageId, uint8_t oldPageId);
int pages_setActiveControlSet(lua_State *L);
int pages_getActiveControlSet(lua_State *L);
int page_delete(lua_State *L);
int page_getId(lua_State *L);
int page_setName(lua_State *L);
int page_getName(lua_State *L);
int page_print(lua_State *L);

void page_register(lua_State *L);

int luaopen_pages(lua_State *L);

static const luaL_Reg pages_functions[] = {
    { "get", pages_get },
    { "getActive", pages_getActive },
    { "display", pages_display },
    { "setActiveControlSet", pages_setActiveControlSet },
    { "getActiveControlSet", pages_getActiveControlSet },
    { NULL, NULL }
};

static const luaL_Reg page_functions[] = { { "getId", page_getId },
                                           { "getName", page_getName },
                                           { "setName", page_setName },
                                           { "print", page_print },
                                           { NULL, NULL } };
