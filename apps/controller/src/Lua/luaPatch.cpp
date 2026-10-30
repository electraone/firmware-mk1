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

#include "luaPatch.h"
#include "SysexBlock.h"
#include "Device.h"
#include "MainDelegate.h"

extern MainDelegate *luaDelegate;

int luaopen_patch(lua_State *L)
{
    luaL_newlib(L, patch_functions);
    return 1;
}

void runOnResponse(const Device &device,
                   uint8_t responseId,
                   SysexBlock &sysexBlock)
{
    luaLE_getModuleFunction(L, "patch", "onResponse");

    if (lua_isfunction(L, -1)) {
        luaLE_pushDevice(device);

        lua_pushnumber(L, responseId);
        luaLE_pushObject(L, "SysexBlock", &sysexBlock);

        if (lua_pcall(L, 3, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function 'onResponse': %s",
                                 lua_tostring(L, -1));
        }
    }
}

void runOnRequest(const Device &device)
{
    luaLE_getModuleFunction(L, "patch", "onRequest");

    if (lua_isfunction(L, -1)) {
        luaLE_pushDevice(device);

        if (lua_pcall(L, 1, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function 'onRequest': %s",
                                 lua_tostring(L, -1));
        }
    }
}

int patch_requestAll(lua_State *L)
{
    lua_settop(L, 0);

    luaDelegate->requestAllPatches();

    return (0);
}
