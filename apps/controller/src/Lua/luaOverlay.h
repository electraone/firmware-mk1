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
 * @file luaOverlay.h
 *
 * @brief Implements a Lua API for managing data model lists / overlays.
 */

#pragma once

#include "luaIntegration.h"

int overlay_create(lua_State *L);
int overlay_createNew(lua_State *L);
int overlay_delete(lua_State *L);
int overlay_print(lua_State *L);
void overlay_register(lua_State *L);
int luaopen_overlays(lua_State *L);

static const luaL_Reg overlays_functions[] = { { "get", overlay_create },
                                               { "create", overlay_createNew },
                                               { NULL, NULL } };

static const luaL_Reg overlay_functions[] = { { "print", overlay_print },
                                              { NULL, NULL } };
