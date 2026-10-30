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
 * @file luaParameterMap.h
 *
 * @brief Implements a Lua API for managing ParameterMap.
 */

#pragma once

#include "luaIntegration.h"
#include "ParameterMap.h"

int luaopen_parameterMap(lua_State *L);

int parameterMap_resetAll(lua_State *L);
int parameterMap_resetDevice(lua_State *L);
int parameterMap_set(lua_State *L);
int parameterMap_apply(lua_State *L);
int parameterMap_modulate(lua_State *L);
int parameterMap_send(lua_State *L);
int parameterMap_get(lua_State *L);
int parameterMap_getValues(lua_State *L);
int parameterMap_map(lua_State *L);
int parameterMap_keep(lua_State *L);
int parameterMap_recall(lua_State *L);
int parameterMap_forget(lua_State *L);
int parameterMap_print(lua_State *L);

void parameterMap_onChange(LookupEntry *entry, Origin origin);

static const luaL_Reg parameterMap_functions[] = {
    { "resetAll", parameterMap_resetAll },
    { "resetDevice", parameterMap_resetDevice },
    { "set", parameterMap_set },
    { "apply", parameterMap_apply },
    { "modulate", parameterMap_modulate },
    { "send", parameterMap_send },
    { "get", parameterMap_get },
    { "getValues", parameterMap_getValues },
    { "map", parameterMap_map },
    { "keep", parameterMap_keep },
    { "recall", parameterMap_recall },
    { "forget", parameterMap_forget },
    { "print", parameterMap_print },
    { NULL, NULL }
};
