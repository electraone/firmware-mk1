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
 * @file luaIntegration.h
 *
 * @brief A collection of function to interface C++ and Lua
 * 
 * The module consists of Controller app specific functionality.
 */

#pragma once

#include "luaIntegrationBase.h"
#include "Device.h"

// table composers
void luaLE_pushDevice(const Device &device);

// validators
int luaLE_checkDeviceId(lua_State *L, int idx);
int luaLE_checkParameterType(lua_State *L, int idx);
int luaLE_checkParameterNumber(lua_State *L, int idx);
int luaLE_checkMidiValue(lua_State *L, int idx);
int luaLE_checkControlId(lua_State *L, int idx);
int luaLE_checkInterface(lua_State *L, int idx);
int luaLE_checkPort(lua_State *L, int idx);
int luaLE_checkChannel(lua_State *L, int idx);
int luaLE_checkDevicelId(lua_State *L, int idx);
int luaLE_checkPageId(lua_State *L, int idx);
int luaLE_checkControlSetId(lua_State *L, int idx);
int luaLE_checkGroupId(lua_State *L, int idx);
int luaLE_checkValueIndex(lua_State *L, int idx);
int luaLE_checkEvents(lua_State *L, int idx);
int luaLE_checkListId(lua_State *L, int idx);
