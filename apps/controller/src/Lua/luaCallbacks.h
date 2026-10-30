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
 * The control's own Lua object, which carries the fields a script sets on it,
 * and the five callbacks it may have subscribed to.
 *
 * The mk2 keeps these on the Control itself. They are here instead because a
 * Control is held for every control of every preset, and an entry is made only
 * the first time a script asks for one, so a preset with one custom control
 * pays for one control.
 *
 * Fourteen bytes. It was 24 in a std::map, whose node made it 48 on the heap;
 * with the Access Virus TI's controls given a painter and a knob handler each,
 * this and the shared functions below took a control from 205 bytes to about
 * half that. A callback is a slot in one table of the functions in use, not a
 * registry reference of its own: sixty controls drawn by one function hold it
 * once.
 */
struct ControlCallbacks {
    /** Zero in any of the fields below means none. */
    static constexpr uint16_t none = 0;

    uint16_t controlId;

    /** A registry reference. */
    uint16_t userdata;

    /** Slots in the shared table - see pushCallbackFunction(). */
    uint16_t paint;
    uint16_t touch;
    uint16_t pot;
    uint16_t potTouch;
    uint16_t switchPress;
};

/**
 * @brief What the control has, or nullptr when a script has never mentioned it.
 *
 * Good until the next controlCallbacks() call that makes an entry.
 */
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
 * @brief Keeps the function on top of the stack, and pops it.
 *
 * The same function given to many controls is kept once and counted.
 *
 * @return its slot
 */
uint16_t retainCallbackFunction(lua_State *L);

/** One use fewer of the slot's function, which goes with its last use. */
void releaseCallbackFunction(lua_State *L, uint16_t slot);

/** Pushes the slot's function. Pushes nothing and answers false for none. */
bool pushCallbackFunction(lua_State *L, uint16_t slot);

/**
 * @brief Drops every reference, unref'ing each in the given state.
 *
 * Called when a preset is unloaded: the controls it named are gone, and the
 * next preset's control one may reuse an id.
 */
void clearControlCallbacks(lua_State *L);
