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

/* Whether the running script has been told onEnter. A preset switched away
 * from before its first repaint pass never reached onReady and onEnter, and
 * must not be told it is leaving a preset it was never told it entered. */
static bool entered = false;

static void runLifecycleFunction(const char *function)
{
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

/*
 * The rest of the lifecycle, as the mk2 calls it for a preset that is not
 * kept in memory. This model only ever has the one preset, and switching to
 * another destroys it, so leaving and exiting come together and every entry
 * is a fresh load:
 *
 *     load    main chunk, onLoad, initial values, onReady, onEnter
 *     switch  onLeave, onExit
 *     script  onExit, then the new script's load (as the mk2: no onLeave)
 */
bool preset_isEntered(void)
{
    return (entered);
}

void preset_onEnter(void)
{
    runLifecycleFunction("onEnter");
    entered = true;
}

void preset_onLeave(void)
{
    if (entered) {
        runLifecycleFunction("onLeave");
        entered = false;
    }
}

void preset_onExit(void)
{
    entered = false;

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
