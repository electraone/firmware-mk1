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
 * @file luaLazyGlobals.h
 *
 * @brief Globals a script gets the first time it names them.
 *
 * Starting Lua for a preset used to build every module table, every object
 * metatable and every constant before the script ran a line: 26 KB of the
 * 34 KB a script cost, measured with the Access Virus TI loaded, most of it
 * for names the script never mentions. A constant was an entry in `_G` and an
 * interned name; a module a table of every function in it.
 *
 * Now `_G` has an `__index` that looks a missing name up in tables kept in
 * flash, and makes it then: a constant is pushed and stored, a module opened
 * the way `luaL_requiref` opens one, a type registered with its metatable and
 * its constructor. What is made is stored in `_G` like any global, so the
 * second use costs nothing and a script that assigns the name first keeps its
 * own value.
 *
 * What a script cannot tell apart from before: reading a name, calling it,
 * passing it on. What it can: `pairs(_G)` and `rawget(_G, name)` see only
 * what has been used.
 */

#pragma once

#include <cstddef>
#include <cstdint>

extern "C" {
#include "lua.h"
}

struct LuaLazyGlobal {
    enum class Kind : uint8_t { number, module, type };

    const char *name;
    Kind kind;
    lua_Number number;
    lua_CFunction open;
    void (*registerType)(lua_State *L);

    static constexpr LuaLazyGlobal constant(const char *name, lua_Number value)
    {
        return { name, Kind::number, value, nullptr, nullptr };
    }

    static constexpr LuaLazyGlobal module(const char *name, lua_CFunction open)
    {
        return { name, Kind::module, 0, open, nullptr };
    }

    /** A type's metatable, and the constructor of the same name where it has
     *  one. The metatable is named after the global. */
    static constexpr LuaLazyGlobal type(const char *name,
                                        void (*registerType)(lua_State *L))
    {
        return { name, Kind::type, 0, nullptr, registerType };
    }
};

/**
 * @brief Adds a table of names, which must be sorted by strcmp().
 *
 * A layer adds its own - the base firmware one, an application another - and
 * adding the same table twice is harmless. Takes effect for states installed
 * afterwards and for names not yet looked up in those already installed.
 */
void luaLE_addLazyGlobals(const LuaLazyGlobal *table, size_t count);

template <size_t N> void luaLE_addLazyGlobals(const LuaLazyGlobal (&table)[N])
{
    luaLE_addLazyGlobals(table, N);
}

/** Puts the lookup on the state's `_G`. */
void luaLE_installLazyGlobals(lua_State *L);

/**
 * @brief Makes sure a type's metatable exists before an object is pushed.
 *
 * @return false when no table names the type
 */
bool luaLE_ensureType(lua_State *L, const char *typeName);
