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
 * @file luaLazyGlobals.cpp
 *
 * @brief Globals a script gets the first time it names them.
 */

#include "luaLazyGlobals.h"
#include "System.h"

extern "C" {
#include "lauxlib.h"
}

#include <cstring>

namespace
{
    struct Table {
        const LuaLazyGlobal *entries;
        size_t count;
    };

    /** One per layer. Two are in use, the base firmware and the app. */
    constexpr size_t maxTables = 4;
    Table tables[maxTables];
    size_t numTables = 0;

    const LuaLazyGlobal *find(const char *name)
    {
        for (size_t t = 0; t < numTables; t++) {
            size_t low = 0;
            size_t high = tables[t].count;

            while (low < high) {
                const size_t middle = (low + high) / 2;
                const int order = strcmp(name, tables[t].entries[middle].name);

                if (order == 0) {
                    return (&tables[t].entries[middle]);
                }

                if (order < 0) {
                    high = middle;
                } else {
                    low = middle + 1;
                }
            }
        }

        return (nullptr);
    }

    /** _G.__index(t, name): what the name is, made now and kept. */
    int lazyIndex(lua_State *L)
    {
        if (lua_type(L, 2) != LUA_TSTRING) {
            return (0);
        }

        const char *name = lua_tostring(L, 2);
        const LuaLazyGlobal *entry = find(name);

        if (!entry) {
            return (0);
        }

        switch (entry->kind) {
            case LuaLazyGlobal::Kind::number:
                lua_pushnumber(L, entry->number);
                lua_pushvalue(L, 2);
                lua_pushvalue(L, -2);
                lua_rawset(L, 1);
                return (1);

            case LuaLazyGlobal::Kind::module:
                /* As luaL_openlibs would have: in package.loaded and in _G. */
                luaL_requiref(L, name, entry->open, 1);
                return (1);

            case LuaLazyGlobal::Kind::type:
                entry->registerType(L);
                lua_pushvalue(L, 2);
                lua_rawget(L, 1);
                return (1);
        }

        return (0);
    }
} // namespace

void luaLE_addLazyGlobals(const LuaLazyGlobal *table, size_t count)
{
    for (size_t t = 0; t < numTables; t++) {
        if (tables[t].entries == table) {
            return;
        }
    }

    /* A binary search over a table out of order misses names that are in it,
     * which would read as a script error somewhere far from here. */
    for (size_t i = 1; i < count; i++) {
        if (strcmp(table[i - 1].name, table[i].name) >= 0) {
            System::logger.write(LOG_ERROR,
                                 "luaLE_addLazyGlobals: not sorted at %s",
                                 table[i].name);
        }
    }

    if (numTables == maxTables) {
        System::logger.write(LOG_ERROR, "luaLE_addLazyGlobals: no room");
        return;
    }

    tables[numTables++] = { table, count };
}

void luaLE_installLazyGlobals(lua_State *L)
{
    lua_pushglobaltable(L);
    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, lazyIndex);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}

bool luaLE_ensureType(lua_State *L, const char *typeName)
{
    if (luaL_getmetatable(L, typeName) != LUA_TNIL) {
        lua_pop(L, 1);
        return (true);
    }

    lua_pop(L, 1);

    const LuaLazyGlobal *entry = find(typeName);

    if (!entry || (entry->kind != LuaLazyGlobal::Kind::type)) {
        return (false);
    }

    entry->registerType(L);

    return (true);
}
