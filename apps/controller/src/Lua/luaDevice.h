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
 * @file luaDevice.h
 *
 * @brief Implements a Lua API for managing Devices.
 */

#pragma once

#include "luaIntegration.h"

int device_create(lua_State *L);
int device_createNew(lua_State *L);
int device_delete(lua_State *L);
int device_getByPortChannel(lua_State *L);
int device_getId(lua_State *L);
int device_setName(lua_State *L);
int device_getName(lua_State *L);
int device_setRate(lua_State *L);
int device_getRate(lua_State *L);
int device_setPort(lua_State *L);
int device_getPort(lua_State *L);
int device_setChannel(lua_State *L);
int device_getChannel(lua_State *L);
int device_print(lua_State *L);

void device_register(lua_State *L);
int luaopen_devices(lua_State *L);

static const luaL_Reg device_functions[] = {
    { "getId", device_getId },
    { "setName", device_setName },
    { "getName", device_getName },
    { "setRate", device_setRate },
    { "getRate", device_getRate },
    { "setPort", device_setPort },
    { "getPort", device_getPort },
    { "setChannel", device_setChannel },
    { "getChannel", device_getChannel },
    { "print", device_print },
    { NULL, NULL }
};
