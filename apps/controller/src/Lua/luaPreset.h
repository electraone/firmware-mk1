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
 * @file luaPreset.h
 *
 * @brief Implements a Lua API for managing Preset life-cycle.
 */

#pragma once

#include "luaIntegration.h"

int luaopen_preset(lua_State *L);

void preset_onLoad(void);
void preset_onReady(void);
void preset_onExit(void);

static const luaL_Reg preset_functions[] = { { NULL, NULL } };
