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
 * @file luaWindow.cpp
 *
 * @brief An implementation of the Window object for E1 Lua. 
 */

#include "luaWindow.h"
#include "Component.h"
#include "System.h"

bool lua_windowRepaintEnabled;

/**
 * @brief The metatable a Component userdata carries.
 *
 * It was never created, so luaL_checkudata(L, 1, "Component") could not match
 * anything and window.addAndMakeVisible() always raised - while
 * window.findChildById() set a metatable it looked up and found nil, which
 * clears the metatable rather than setting one. Between them the two calls
 * could not be used together at all.
 */
void window_registerComponent(lua_State *L)
{
    luaL_newmetatable(L, "Component");
    lua_pop(L, 1);
}

int luaopen_window(lua_State *L)
{
    luaL_newlib(L, window_functions);
    return (1);
}

int window_addAndMakeVisible(lua_State *L)
{
    lua_settop(L, 1);

    /* Component, not Button. Every component inherits Component virtually, so
     * converting a Button pointer to a Component one reads an offset out of the
     * object - and a Component pointer reinterpreted as a Button never had one
     * to read. */
    Component *component =
        *reinterpret_cast<Component **>(luaL_checkudata(L, 1, "Component"));

    if (component) {
        System::windowManager.getActiveWindow()->addAndMakeVisible(component);
        System::windowManager.repaintActive();
    }

    return (0);
}

int window_repaint([[maybe_unused]] lua_State *L)
{
    System::windowManager.repaintActive();
    return (0);
}

int window_clear([[maybe_unused]] lua_State *L)
{
    return (0);
}

int window_stop([[maybe_unused]] lua_State *L)
{
    System::tasks.stopWindowRepaint();
    return (0);
}

int window_resume([[maybe_unused]] lua_State *L)
{
    System::tasks.resumeWindowRepaint();
    return (0);
}

int window_findChildById(lua_State *L)
{
    lua_settop(L, 1);
    int id = luaL_checkinteger(L, 1);

    Component *component =
        System::windowManager.getActiveWindow()->findChildById(id);

    if (component) {
        luaLE_pushObject(L, "Component", component);
        return (1);
    }

    return (0);
}
