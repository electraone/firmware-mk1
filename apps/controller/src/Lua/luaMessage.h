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
 * @file luaMessage.h
 *
 * @brief Implements a Lua API for managing Messages.
 */

#pragma once

#include "luaIntegration.h"

int message_create(lua_State *L);
int message_delete(lua_State *L);
int message_setType(lua_State *L);
int message_getType(lua_State *L);
int message_setParameterNumber(lua_State *L);
int message_getParameterNumber(lua_State *L);
int message_setDeviceId(lua_State *L);
int message_getDeviceId(lua_State *L);
int message_setValue(lua_State *L);
int message_getValue(lua_State *L);
int message_setOnValue(lua_State *L);
int message_getOnValue(lua_State *L);
int message_setOffValue(lua_State *L);
int message_getOffValue(lua_State *L);
int message_setMin(lua_State *L);
int message_getMin(lua_State *L);
int message_setMax(lua_State *L);
int message_getMax(lua_State *L);
int message_setRange(lua_State *L);
int message_print(lua_State *L);

void message_register(lua_State *L);

static const luaL_Reg message_functions[] = {
    { "setType", message_setType },
    { "getType", message_getType },
    { "setParameterNumber", message_setParameterNumber },
    { "getParameterNumber", message_getParameterNumber },
    { "setDeviceId", message_setDeviceId },
    { "getDeviceId", message_getDeviceId },
    { "setValue", message_setValue },
    { "getValue", message_getValue },
    { "setOnValue", message_setOnValue },
    { "getOnValue", message_getOnValue },
    { "setOffValue", message_setOffValue },
    { "getOffValue", message_getOffValue },
    { "setMin", message_setMin },
    { "getMin", message_getMin },
    { "setMax", message_setMax },
    { "getMax", message_getMax },
    { "setRange", message_setRange },
    { "print", message_print },
    { NULL, NULL }
};
