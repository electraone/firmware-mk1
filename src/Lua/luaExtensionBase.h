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
 * @file luaExtension.h
 *
 * @brief A collection of E1 specific Lua functions.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#ifdef __cplusplus
}
#endif

#include "luaIntegrationBase.h"
#include "luaSysexBlock.h"
#include "luaMidi.h"
#include "luaWindow.h"
#include "luaController.h"
#include "luaTimer.h"
#include "luaLogger.h"
#include "luaBytes.h"
#include "luaTransport.h"

extern lua_State *L;

void luaLE_openEoslibs(lua_State *L, const luaL_Reg *libs);
void loadLuaModule(const char *filename);
void runLuaFunction(const char *functionName);
void runLuaString(const char *commandText);

lua_State *initLua(void);
void closeLua(void);
void executeElectraLua(const char *filename);
bool isLuaValid(const char *filename);

void loadBaseGlobalVariables(lua_State *L);
