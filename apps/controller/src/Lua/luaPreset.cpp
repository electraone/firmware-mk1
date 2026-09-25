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

#include "luaPreset.h"

int luaopen_preset(lua_State *L)
{
    luaL_newlib(L, preset_functions);
    return 1;
}

void preset_onLoad(void)
{
    const char *function = "onLoad";

    luaLE_getModuleFunction("preset", function);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
    luaLE_postFunctionCleanUp(L);
}

void preset_onReady(void)
{
    const char *function = "onReady";

    luaLE_getModuleFunction("preset", function);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
    luaLE_postFunctionCleanUp(L);
}

void preset_onExit(void)
{
    const char *function = "onExit";

    luaLE_getModuleFunction("preset", function);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
    luaLE_postFunctionCleanUp(L);
}
