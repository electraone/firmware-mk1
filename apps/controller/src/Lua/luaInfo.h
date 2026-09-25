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
 * @file luaInfo.h
 *
 * @brief Implements a Lua API for managing Info text (see model).
 */

#pragma once

#include "luaIntegration.h"

int luaopen_info(lua_State *L);

int info_setText(lua_State *L);

static const luaL_Reg info_functions[] = { { "setText", info_setText },
                                           { NULL, NULL } };
