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
 * @file luaCallbacks.h
 *
 * @brief The Lua functions a Control has been given, and its Lua object.
 */

#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <cstdint>

/**
 * @brief What a script has hung off one control.
 *
 * Six registry references: the control's own Lua object, which carries the
 * fields a script sets on it, and the five callbacks it may have subscribed
 * to.
 *
 * The mk2 keeps these on the Control itself. They are here instead because a
 * Control is held for every control of every preset - a hundred and thirty of
 * them on the preset this was measured against - and six references on each
 * is three kilobytes of a device that has a quarter of a megabyte, spent
 * almost entirely on controls no script will ever mention. An entry is made
 * the first time one is asked for, so a preset with one custom control pays
 * for one control.
 */
struct ControlCallbacks {
    int userdata = LUA_REFNIL;
    int paint = LUA_REFNIL;
    int touch = LUA_REFNIL;
    int pot = LUA_REFNIL;
    int potTouch = LUA_REFNIL;
    int switchPress = LUA_REFNIL;
};

/** What the control has, or nullptr when a script has never mentioned it. */
ControlCallbacks *findControlCallbacks(uint16_t controlId);

/** The same, made empty if it was not there. */
ControlCallbacks &controlCallbacks(uint16_t controlId);

/** Whether the control has a callback of that kind. */
bool hasPaintCallback(uint16_t controlId);
bool hasTouchCallback(uint16_t controlId);
bool hasPotCallback(uint16_t controlId);
bool hasPotTouchCallback(uint16_t controlId);
bool hasSwitchCallback(uint16_t controlId);

/**
 * @brief Drops every reference, unref'ing each in the given state.
 *
 * Called when a preset is unloaded: the controls it named are gone, and the
 * next preset's control one may reuse an id.
 */
void clearControlCallbacks(lua_State *L);
