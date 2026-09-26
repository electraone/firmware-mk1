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
 * The module consists of generic non-E1 specific functionality.
 */

#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

extern lua_State *L;

/**
 * @brief Puts the stack back where it was, however the call left it.
 *
 * A C++ to Lua call pushes the function, its arguments and - when the call
 * fails - the error message. lua_pcall() balances the first two and leaves the
 * third, and a handler that turns out not to exist leaves the value the lookup
 * found. Neither is popped at most of the call sites, so a preset whose
 * handler errors on every MIDI message grows the stack one value per message
 * until it runs into the stack the state was given.
 *
 * Declaring one of these at the top of a callback makes that impossible, and
 * costs nothing when the call goes well. It is not luaLE_postFunctionCleanUp()
 * because that also runs a full garbage collection, which is far too much to
 * pay per MIDI message.
 */
class LuaStackGuard
{
public:
    explicit LuaStackGuard(lua_State *state)
        : state(state), top(lua_gettop(state))
    {
    }

    ~LuaStackGuard()
    {
        lua_settop(state, top);
    }

    LuaStackGuard(const LuaStackGuard &) = delete;
    LuaStackGuard &operator=(const LuaStackGuard &) = delete;

private:
    lua_State *state;
    int top;
};

void luaLE_pushTableString(lua_State *L, const char *key, const char *value);
void luaLE_pushTableInteger(lua_State *L, const char *key, int value);
void luaLE_pushTableObject(lua_State *L,
                           const char *key,
                           const char *objectName,
                           const void *object);
void luaLE_pushArrayInteger(lua_State *L, int key, int value);
void luaLE_pushArrayString(lua_State *L, int key, const char *value);
void luaLE_pushArrayObject(lua_State *L,
                           int key,
                           const char *objectName,
                           const void *object);
void luaLE_pushObject(lua_State *L, const char *objectName, const void *object);
void luaLE_getModuleFunction(lua_State *L,
                             const char *module,
                             const char *function);
void luaLE_addObjectMethods(lua_State *L, const luaL_Reg *l);
void luaLE_postFunctionCleanUp(lua_State *L);
void luaLE_dumpstack(lua_State *L);
bool luaLE_functionExists(const char *module, const char *function);
bool luaLE_checkBoolean(lua_State *L, int idx);

/**
 * @brief An optional flag, given either as a boolean or as 0 / 1.
 *
 * The flag arguments of the MIDI library - lsbFirst, resetRpn - were read with
 * luaL_optinteger(), so a script passing the `true` that the argument's name
 * asks for got a type error rather than the flag it meant.
 */
bool luaLE_optBoolean(lua_State *L, int idx, bool defaultValue);
void luaLE_getModuleFunction(const char *module, const char *function);
