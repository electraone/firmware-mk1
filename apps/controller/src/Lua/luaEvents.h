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
 * @file luaEvents.h
 *
 * @brief Implements a Lua API for managing controller Events.
 */

#pragma once

#include "luaIntegration.h"

int events_subscribe(lua_State *L);
int events_setPort(lua_State *L);

void events_onPageChange(uint8_t newPageId, uint8_t oldPageId);
void events_onPotTouch(uint8_t potId, uint16_t controlId, bool touched);
void events_onUsbHostChange(uint8_t port, uint8_t eventType);

int luaopen_events(lua_State *L);

static const luaL_Reg events_functions[] = { { "subscribe", events_subscribe },
                                             { "setPort", events_setPort },
                                             { NULL, NULL } };
