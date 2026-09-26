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
 * @file luaController.h
 *
 * @brief An implementation of library to access information about the
 *  hardware controller. 
 */

#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

/**
 * @brief Which controller a script is running on.
 *
 * A preset written for the mk2 may draw into space this panel does not have, or
 * use a call that only the newer firmware carries. Asking rather than guessing
 * is what controller.require() is for, and the answer has to be a number a
 * script can compare as well as a name it can print.
 */
enum class ControllerModel { mk1 = 1, mk2 = 2, mini = 3 };

int luaopen_controller(lua_State *L);

int controller_uptime(lua_State *L);
int controller_getModel(lua_State *L);
int controller_getNumModel(lua_State *L);
int controller_getFirmwareVersion(lua_State *L);
int controller_getFirmwareNumVersion(lua_State *L);
int controller_require(lua_State *L);
int controller_isRequired(lua_State *L);

static const luaL_Reg controller_functions[] = {
    { "uptime", controller_uptime },
    { "getModel", controller_getModel },
    { "getNumModel", controller_getNumModel },
    { "getFirmwareVersion", controller_getFirmwareVersion },
    { "getFirmwareNumVersion", controller_getFirmwareNumVersion },
    { "require", controller_require },
    { "isRequired", controller_isRequired },
    { NULL, NULL }
};
