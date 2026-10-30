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
 * @file luaLE.cpp
 *
 * @brief A collection of E1 specific Lua functions.
 */

#include "luaExtensionBase.h"
#include "luaLazyGlobals.h"
#include "SysexBlock.h"
#include "System.h"

extern bool lua_windowRepaintEnabled;

/*
 * What every script can name, made when it first does - see luaLazyGlobals.h.
 * Sorted by strcmp(), so capitals first; luaLE_addLazyGlobals() says so in the
 * log if an entry is out of order.
 *
 * The standard libraries a script may never touch are here too. The base
 * library, `package` and `string` are opened up front: the first two put
 * functions straight into _G, and `string` gives every string its methods.
 */
static const LuaLazyGlobal baseGlobals[] = {
    LuaLazyGlobal::constant("ACTIVE_SENSING", 254),
    LuaLazyGlobal::constant("BUTTONS", 32),
    LuaLazyGlobal::constant("BUTTON_1", 1),
    LuaLazyGlobal::constant("BUTTON_2", 2),
    LuaLazyGlobal::constant("BUTTON_3", 3),
    LuaLazyGlobal::constant("BUTTON_4", 4),
    LuaLazyGlobal::constant("BUTTON_5", 5),
    LuaLazyGlobal::constant("BUTTON_6", 6),
    LuaLazyGlobal::constant("CHANNEL_PRESSURE", 208),
    LuaLazyGlobal::constant("CLOCK", 248),
    LuaLazyGlobal::constant("CONTINUE", 251),
    LuaLazyGlobal::constant("CONTROL_CHANGE", 176),
    LuaLazyGlobal::type("Component", window_registerComponent),
    LuaLazyGlobal::constant("HEIGHT", 4),
    LuaLazyGlobal::constant("INTERNAL", 0),
    LuaLazyGlobal::constant("LUA", 2),
    LuaLazyGlobal::constant("MIDI", 1),
    LuaLazyGlobal::constant("MIDI_IO", 0),
    LuaLazyGlobal::constant("MODEL_MINI", (int)ControllerModel::mini),
    LuaLazyGlobal::constant("MODEL_MK1", (int)ControllerModel::mk1),
    LuaLazyGlobal::constant("MODEL_MK2", (int)ControllerModel::mk2),
    LuaLazyGlobal::constant("NONE", 0),
    LuaLazyGlobal::constant("NOTE_OFF", 128),
    LuaLazyGlobal::constant("NOTE_ON", 144),
    LuaLazyGlobal::constant("PITCH_BEND", 224),
    LuaLazyGlobal::constant("POLY_PRESSURE", 160),
    LuaLazyGlobal::constant("PORT_1", 0),
    LuaLazyGlobal::constant("PORT_2", 1),
    LuaLazyGlobal::constant("PORT_CTRL", 2),
    LuaLazyGlobal::constant("POTS", 8),
    LuaLazyGlobal::constant("POT_1", 1),
    LuaLazyGlobal::constant("POT_10", 10),
    LuaLazyGlobal::constant("POT_11", 11),
    LuaLazyGlobal::constant("POT_12", 12),
    LuaLazyGlobal::constant("POT_2", 2),
    LuaLazyGlobal::constant("POT_3", 3),
    LuaLazyGlobal::constant("POT_4", 4),
    LuaLazyGlobal::constant("POT_5", 5),
    LuaLazyGlobal::constant("POT_6", 6),
    LuaLazyGlobal::constant("POT_7", 7),
    LuaLazyGlobal::constant("POT_8", 8),
    LuaLazyGlobal::constant("POT_9", 9),
    LuaLazyGlobal::constant("PROGRAM_CHANGE", 192),
    LuaLazyGlobal::constant("RESET", 255),
    LuaLazyGlobal::constant("SONG_POSITION", 242),
    LuaLazyGlobal::constant("SONG_SELECT", 243),
    LuaLazyGlobal::constant("START", 250),
    LuaLazyGlobal::constant("STOP", 252),
    LuaLazyGlobal::constant("SYSEX", 240),
    LuaLazyGlobal::type("SysexBlock", sysexBlock_register),
    LuaLazyGlobal::constant("TIME_CODE_QUARTER_FRAME", 241),
    LuaLazyGlobal::constant("TOUCH", 16),
    LuaLazyGlobal::constant("TOUCH_POINT_1", 1),
    LuaLazyGlobal::constant("TOUCH_POINT_2", 2),
    LuaLazyGlobal::constant("TOUCH_POINT_3", 3),
    LuaLazyGlobal::constant("TOUCH_POINT_4", 4),
    LuaLazyGlobal::constant("TOUCH_POINT_5", 5),
    LuaLazyGlobal::constant("TUNE_REQUEST", 246),
    LuaLazyGlobal::constant("USB_DEV", 1),
    LuaLazyGlobal::constant("USB_HOST", 2),
    LuaLazyGlobal::constant("USB_HOST_PORT", 4),
    LuaLazyGlobal::constant("WIDTH", 3),
    LuaLazyGlobal::constant("WINDOWS", 64),
    LuaLazyGlobal::constant("X", 1),
    LuaLazyGlobal::constant("Y", 2),
    LuaLazyGlobal::module("bytes", luaopen_bytes),
    LuaLazyGlobal::module("controller", luaopen_controller),
    LuaLazyGlobal::module("coroutine", luaopen_coroutine),
    LuaLazyGlobal::module("debug", luaopen_debug),
    LuaLazyGlobal::module("logger", luaopen_logger),
    LuaLazyGlobal::module("math", luaopen_math),
    LuaLazyGlobal::module("midi", luaopen_midi),
    LuaLazyGlobal::module("table", luaopen_table),
    LuaLazyGlobal::module("timer", luaopen_timer),
    LuaLazyGlobal::module("transport", luaopen_transport),
    LuaLazyGlobal::module("window", luaopen_window),
};

lua_State *initLua(void)
{
    lua_State *lua = nullptr;
    // Enable printf/sprintf to print floats
    asm(".global _printf_float");

    lua = luaL_newstate(); /* opens Lua */
    luaL_requiref(lua, LUA_GNAME, luaopen_base, 1);
    luaL_requiref(lua, LUA_LOADLIBNAME, luaopen_package, 1);
    luaL_requiref(lua, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(lua, 3);

    /* Everything else - libraries, modules, types and constants - is made the
     * first time the script names it. */
    luaLE_addLazyGlobals(baseGlobals);
    luaLE_installLazyGlobals(lua);

    return (lua);
}

void closeLua(void)
{
    if (L) {
        lua_close(L);
        L = nullptr;
    }

    sysexBlock_releaseAll();
}

void loadLuaModule(const char *filename)
{
    // Clear the stack
    lua_settop(L, 0);

    int error = luaL_dofile(L, filename);

    if (error) {
        System::logger.write(
            LOG_LUA,
            "loadLuaModule: error loading file: filename=%s, error=%s",
            filename,
            lua_tostring(L, -1));
        lua_pop(L, 1); /* pop error message from the stack */
    } else {
        System::logger.write(
            LOG_LUA,
            "loadLuaModule: Lua extension file initialized: file=%s",
            filename);
    }
}

void runLuaFunction(const char *functionName)
{
    lua_getglobal(L, functionName);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 functionName,
                                 lua_tostring(L, -1));
        }
    }
}

void runLuaString(const char *commandText)
{
    lua_settop(L, 0);

    int error = luaL_dostring(L, commandText);

    if (error) {
        System::logger.write(LOG_LUA,
                             "runLuaString: error loading file: error=%s",
                             lua_tostring(L, -1));
        lua_pop(L, 1); /* pop error message from the stack */
    }
}

void executeElectraLua(const char *filename)
{
    lua_windowRepaintEnabled = false;
    System::logger.write(LOG_LUA, "loadLua debug output:");
    System::logger.write(LOG_LUA, "---- START ----");
    loadLuaModule(filename);
    System::logger.write(LOG_LUA, "----- END -----");
    lua_windowRepaintEnabled = true;
}

bool isLuaValid(const char *filename)
{
    // @todo do the Lua source validation here
    return (Hardware::sdcard.exists(filename));
}

