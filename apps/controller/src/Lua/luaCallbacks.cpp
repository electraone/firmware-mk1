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

#include "luaCallbacks.h"
#include <algorithm>
#include <deque>
#include <vector>

namespace
{
    /* Sorted by control id. A deque takes its memory in 512-byte blocks and
     * never asks for one piece the size of all of it, which a vector would
     * each time it doubled - with a preset resident the heap has no such
     * piece to give. */
    std::deque<ControlCallbacks> callbacks;

    /* The functions in use. The table, in the registry, maps a slot to its
     * function and the function back to its slot, which is how a function
     * given to a second control is found to be kept already. Slot zero is
     * none, so it is never handed out. */
    int functionsRef = LUA_NOREF;
    std::vector<uint16_t> uses;

    std::deque<ControlCallbacks>::iterator lowerBound(uint16_t controlId)
    {
        return (std::lower_bound(callbacks.begin(),
                                 callbacks.end(),
                                 controlId,
                                 [](const ControlCallbacks &entry, uint16_t id) {
                                     return (entry.controlId < id);
                                 }));
    }

    void pushFunctions(lua_State *L)
    {
        if (functionsRef == LUA_NOREF) {
            lua_newtable(L);
            lua_pushvalue(L, -1);
            functionsRef = luaL_ref(L, LUA_REGISTRYINDEX);
        } else {
            lua_rawgeti(L, LUA_REGISTRYINDEX, functionsRef);
        }
    }
} // namespace

ControlCallbacks *findControlCallbacks(uint16_t controlId)
{
    auto entry = lowerBound(controlId);

    return (((entry == callbacks.end()) || (entry->controlId != controlId))
                ? nullptr
                : &*entry);
}

ControlCallbacks &controlCallbacks(uint16_t controlId)
{
    auto entry = lowerBound(controlId);

    if ((entry == callbacks.end()) || (entry->controlId != controlId)) {
        entry = callbacks.insert(entry, ControlCallbacks{ controlId });
    }

    return (*entry);
}

namespace
{
    bool has(uint16_t controlId, uint16_t ControlCallbacks::*which)
    {
        const ControlCallbacks *entry = findControlCallbacks(controlId);

        return (entry && (entry->*which != ControlCallbacks::none));
    }
} // namespace

bool hasPaintCallback(uint16_t controlId)
{
    return (has(controlId, &ControlCallbacks::paint));
}

bool hasTouchCallback(uint16_t controlId)
{
    return (has(controlId, &ControlCallbacks::touch));
}

bool hasPotCallback(uint16_t controlId)
{
    return (has(controlId, &ControlCallbacks::pot));
}

bool hasPotTouchCallback(uint16_t controlId)
{
    return (has(controlId, &ControlCallbacks::potTouch));
}

bool hasSwitchCallback(uint16_t controlId)
{
    return (has(controlId, &ControlCallbacks::switchPress));
}

uint16_t retainCallbackFunction(lua_State *L)
{
    pushFunctions(L);
    lua_pushvalue(L, -2);

    if (lua_rawget(L, -2) == LUA_TNUMBER) {
        const uint16_t slot = (uint16_t)lua_tointeger(L, -1);

        lua_pop(L, 3);
        uses[slot]++;
        return (slot);
    }

    lua_pop(L, 1);

    if (uses.empty()) {
        uses.push_back(0);
    }

    /* The first slot a released function left, or a new one. */
    uint16_t slot = 1;

    while ((slot < uses.size()) && (uses[slot] != 0)) {
        slot++;
    }

    if (slot == uses.size()) {
        if (slot == UINT16_MAX) {
            lua_pop(L, 2);
            luaL_error(L, "too many callback functions");
            return (ControlCallbacks::none);
        }
        uses.push_back(0);
    }

    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, slot);
    lua_pushvalue(L, -2);
    lua_pushinteger(L, slot);
    lua_rawset(L, -3);
    lua_pop(L, 2);

    uses[slot] = 1;
    return (slot);
}

void releaseCallbackFunction(lua_State *L, uint16_t slot)
{
    if ((slot == ControlCallbacks::none) || (slot >= uses.size())
        || (uses[slot] == 0)) {
        return;
    }

    if (--uses[slot] > 0) {
        return;
    }

    pushFunctions(L);
    lua_rawgeti(L, -1, slot);
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pushnil(L);
    lua_rawseti(L, -2, slot);
    lua_pop(L, 1);
}

bool pushCallbackFunction(lua_State *L, uint16_t slot)
{
    if ((slot == ControlCallbacks::none) || (functionsRef == LUA_NOREF)) {
        return (false);
    }

    lua_rawgeti(L, LUA_REGISTRYINDEX, functionsRef);
    lua_rawgeti(L, -1, slot);
    lua_remove(L, -2);

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return (false);
    }

    return (true);
}

void clearControlCallbacks(lua_State *L)
{
    if (L) {
        for (const auto &entry : callbacks) {
            if (entry.userdata != ControlCallbacks::none) {
                luaL_unref(L, LUA_REGISTRYINDEX, entry.userdata);
            }
        }

        if (functionsRef != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, functionsRef);
        }
    }

    /* Given back, not only emptied: the next preset may have no script. */
    callbacks.clear();
    callbacks.shrink_to_fit();
    uses.clear();
    uses.shrink_to_fit();
    functionsRef = LUA_NOREF;
}
