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
#include <map>

namespace
{
    std::map<uint16_t, ControlCallbacks> callbacks;
} // namespace

ControlCallbacks *findControlCallbacks(uint16_t controlId)
{
    auto entry = callbacks.find(controlId);

    return ((entry == callbacks.end()) ? nullptr : &entry->second);
}

ControlCallbacks &controlCallbacks(uint16_t controlId)
{
    return (callbacks[controlId]);
}

namespace
{
    bool has(uint16_t controlId, int ControlCallbacks::*which)
    {
        const ControlCallbacks *entry = findControlCallbacks(controlId);

        return (entry && (entry->*which != LUA_REFNIL));
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

void clearControlCallbacks(lua_State *L)
{
    if (L) {
        for (auto &entry : callbacks) {
            for (int ref : { entry.second.userdata,
                             entry.second.paint,
                             entry.second.touch,
                             entry.second.pot,
                             entry.second.potTouch,
                             entry.second.switchPress }) {
                if (ref != LUA_REFNIL) {
                    luaL_unref(L, LUA_REGISTRYINDEX, ref);
                }
            }
        }
    }

    callbacks.clear();
}
