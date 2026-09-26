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
 * @file luaGraphics.h
 *
 * @brief An implementation of the Graphics for E1 Lua. 
 */

#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

int luaopen_graphics(lua_State *L);

/**
 * @brief Opens the graphics library for as long as it exists.
 *
 * Every call in the library draws through the chip's active window, which is
 * set to the component being painted. One of these on the stack of a paint
 * callback is what tells the library the window means something; outside one,
 * a graphics call is an error rather than a shape in the wrong place.
 */
class LuaGraphicsScope
{
public:
    LuaGraphicsScope();
    ~LuaGraphicsScope();
};

int graphics_setColour(lua_State *L);
int graphics_setBackgroundColour(lua_State *L);
int graphics_drawPixel(lua_State *L);
int graphics_drawLine(lua_State *L);
int graphics_drawRect(lua_State *L);
int graphics_fillRect(lua_State *L);
int graphics_drawRoundRect(lua_State *L);
int graphics_fillRoundRect(lua_State *L);
int graphics_drawTriangle(lua_State *L);
int graphics_fillTriangle(lua_State *L);
int graphics_drawCircle(lua_State *L);
int graphics_fillCircle(lua_State *L);
int graphics_drawEllipse(lua_State *L);
int graphics_fillEllipse(lua_State *L);
int graphics_fillCurve(lua_State *L);
int graphics_print(lua_State *L);

/**
 * @brief Colour arithmetic, which every preset was writing for itself.
 *
 * Deliberately not behind graphicsCallAllowed(): these compute a number and
 * draw nothing, so a preset can work its palette out once at load rather than
 * per frame inside a paint callback.
 *
 * All three speak the 24 bit colours the Lua side uses everywhere - the six
 * colour globals the firmware hands a script are 0xF45C51 and friends - not
 * the 16 bit ones the panel stores.
 */
/**
 * @brief How wide a string will be, and how tall a line is.
 *
 * graphics.print() takes a width and an alignment, which centres inside a box
 * whose size is already known. It does not lay two strings side by side, fit a
 * badge to its label, or truncate with an ellipsis - all of which need the
 * measurement, and all of which were being done with a guessed per character
 * width.
 *
 * Measuring reads the font tables and touches no framebuffer, so unlike the
 * drawing calls these are allowed outside a paint callback: a preset can lay
 * itself out when it loads.
 */
/**
 * @brief A ring segment: what a dial is, and what every custom one drew by
 *        hand out of line segments computed in Lua.
 *
 * Angles are degrees measured clockwise from twelve o'clock, which is how a
 * dial reads and how a preset thinks about one. Built from the triangles the
 * driver already has rather than a new primitive, so it behaves the same on
 * both models and on the host.
 */
int graphics_drawArc(lua_State *L);

int graphics_getTextWidth(lua_State *L);
int graphics_getTextHeight(lua_State *L);

int graphics_rgb(lua_State *L);
int graphics_dim(lua_State *L);
int graphics_blend(lua_State *L);

static const luaL_Reg graphics_functions[] = {
    { "drawArc", graphics_drawArc },

    { "getTextWidth", graphics_getTextWidth },
    { "getTextHeight", graphics_getTextHeight },

    { "rgb", graphics_rgb },
    { "dim", graphics_dim },
    { "blend", graphics_blend },

    { "setColor", graphics_setColour },
    { "setBackgroundColor", graphics_setBackgroundColour },
    { "drawPixel", graphics_drawPixel },
    { "drawLine", graphics_drawLine },
    { "drawRect", graphics_drawRect },
    { "fillRect", graphics_fillRect },
    { "drawRoundRect", graphics_drawRoundRect },
    { "fillRoundRect", graphics_fillRoundRect },
    { "drawTriangle", graphics_drawTriangle },
    { "fillTriangle", graphics_fillTriangle },
    { "drawCircle", graphics_drawCircle },
    { "fillCircle", graphics_fillCircle },
    { "drawEllipse", graphics_drawEllipse },
    { "fillEllipse", graphics_fillEllipse },
    { "fillCurve", graphics_fillCurve },
    { "print", graphics_print },
    { NULL, NULL }
};
