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
 * @file luaValue.h
 *
 * @brief Implements a Lua API for managing ControlValue.
 */

#pragma once

#include "luaIntegration.h"

int value_create(lua_State *L);
int value_delete(lua_State *L);
int value_getId(lua_State *L);
int value_setDefault(lua_State *L);
int value_getDefault(lua_State *L);
int value_setMin(lua_State *L);
int value_getMin(lua_State *L);
int value_setMax(lua_State *L);
int value_getMax(lua_State *L);
int value_setRange(lua_State *L);
int value_getOverlayId(lua_State *L);
int value_setOverlayId(lua_State *L);
int value_getMessage(lua_State *L);
int value_getControl(lua_State *L);
int value_overrideValue(lua_State *L);
int value_cancelOverride(lua_State *L);
int value_print(lua_State *L);

void value_register(lua_State *L);

static const luaL_Reg value_functions[] = {
    { "getId", value_getId },
    { "setDefault", value_setDefault },
    { "getDefault", value_getDefault },
    { "setMin", value_setMin },
    { "getMin", value_getMin },
    { "setMax", value_setMax },
    { "getMax", value_getMax },
    { "setRange", value_setRange },
    { "getOverlayId", value_getOverlayId },
    { "setOverlayId", value_setOverlayId },
    { "getMessage", value_getMessage },
    { "getControl", value_getControl },
    { "overrideValue", value_overrideValue },
    { "cancelOverride", value_cancelOverride },
    { "print", value_print },
    { NULL, NULL }
};
