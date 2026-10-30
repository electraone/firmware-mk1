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

#include "luaEvents.h"
#include "MainDelegate.h"

extern MainDelegate *luaDelegate;

int luaopen_events(lua_State *L)
{
    luaL_newlib(L, events_functions);
    return 1;
}

int events_subscribe(lua_State *L)
{
    lua_settop(L, 1);
    int events = luaLE_checkEvents(L, 1);
    luaDelegate->setSubscribedEvents(events);
    return (0);
}

int events_setPort(lua_State *L)
{
    lua_settop(L, 1);
    int port = luaLE_checkPort(L, 1);
    luaDelegate->setControlPort(port);
    return (0);
}

void events_onPageChange(uint8_t newPageId, uint8_t oldPageId)
{
    const char *function = "onPageChange";
    luaLE_getModuleFunction("events", function);

    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, newPageId);
        lua_pushnumber(L, oldPageId);
        if (lua_pcall(L, 2, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
    luaLE_postFunctionCleanUp(L);
}

void events_onPotTouch(uint8_t potId, uint16_t controlId, bool touched)
{
    const char *function = "onPotTouch";
    luaLE_getModuleFunction("events", function);

    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, potId);
        lua_pushnumber(L, controlId);
        lua_pushboolean(L, touched);
        if (lua_pcall(L, 3, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
    luaLE_postFunctionCleanUp(L);
}

void events_onUsbHostChange(uint8_t port, uint8_t eventType)
{
    const char *function = "onUsbHostChange";
    luaLE_getModuleFunction("events", function);

    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, port);
        lua_pushnumber(L, eventType);
        if (lua_pcall(L, 2, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
    luaLE_postFunctionCleanUp(L);
}
