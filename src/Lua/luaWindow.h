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
 * @file luaWindow.h
 *
 * @brief An implementation of the Window object for E1 Lua. 
 */

#pragma once

#include "luaIntegrationBase.h"

int luaopen_window(lua_State *L);

/** Creates the metatable a Component userdata is given. */
void window_registerComponent(lua_State *L);

int window_addAndMakeVisible(lua_State *L);
int window_clear(lua_State *L);
int window_stop(lua_State *L);
int window_resume(lua_State *L);
int window_repaint(lua_State *L);
int window_findChildById(lua_State *L);

static const luaL_Reg window_functions[] = {
    { "addAndMakeVisible", window_addAndMakeVisible },
    { "clear", window_clear },
    { "stop", window_stop },
    { "resume", window_resume },
    { "repaint", window_repaint },
    { "findChildById", window_findChildById },
    { NULL, NULL }
};
