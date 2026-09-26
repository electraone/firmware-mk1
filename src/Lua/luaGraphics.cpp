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

#include "luaGraphics.h"
#include "Colours.h"
#include "Hardware.h"
#include "TextBase.h"
#include <cmath>

namespace
{
    /* Set for as long as a custom control is painting. See
     * graphicsCallAllowed(). */
    bool paintingCustomControl = false;
} // namespace

LuaGraphicsScope::LuaGraphicsScope()
{
    paintingCustomControl = true;
}

LuaGraphicsScope::~LuaGraphicsScope()
{
    paintingCustomControl = false;
}

int luaopen_graphics(lua_State *L)
{
    luaL_newlib(L, graphics_functions);
    return (1);
}

/**
 * Refuses a graphics call made from anywhere but a control's paint callback.
 *
 * Every call below draws through the chip's active window, which the repaint
 * manager sets to the component being painted. Outside a paint that window is
 * whatever the last component left behind, so a script drawing from onLoad, a
 * timer or a MIDI handler would put its shapes somewhere arbitrary - over
 * another control, or off the screen.
 *
 * Raising a Lua error rather than quietly ignoring the call is deliberate: the
 * preset author gets told which call was wrong, at the point it was made.
 */
static bool graphicsCallAllowed(lua_State *L, const char *function)
{
    if (paintingCustomControl) {
        return (true);
    }

    luaL_error(L,
               "graphics.%s: only allowed from a control's paint callback",
               function);
    return (false);
}

int graphics_setColour(lua_State *L)
{
    if (!graphicsCallAllowed(L, "setColour")) {
        return (0);
    }
    lua_settop(L, 1);
    int colour = luaL_checkinteger(L, -1);

    Hardware::screen.setColour(Colours888::toRGB565(colour));
    return (0);
}

/**
 * @brief graphics.setBackgroundColor(colour)
 *
 * Tells the text renderer what it is drawing onto. Anti-aliased glyphs are
 * blended on the CPU because the chip has no per-pixel alpha, so text printed
 * over a colour this does not match gets a fringe in the wrong one. Defaults
 * to black, which is what a control is cleared to.
 */
int graphics_setBackgroundColour(lua_State *L)
{
    if (!graphicsCallAllowed(L, "setBackgroundColor")) {
        return (0);
    }
    lua_settop(L, 1);
    int colour = luaL_checkinteger(L, -1);

    Hardware::screen.user().setTextBackgroundColour(
        Colours888::toRGB565(colour));
    return (0);
}

int graphics_drawPixel(lua_State *L)
{
    if (!graphicsCallAllowed(L, "drawPixel")) {
        return (0);
    }
    lua_settop(L, 2);
    int x = luaL_checkinteger(L, -2);
    int y = luaL_checkinteger(L, -1);

    Hardware::screen.drawPixel(x, y);
    return (0);
}

int graphics_drawLine(lua_State *L)
{
    if (!graphicsCallAllowed(L, "drawLine")) {
        return (0);
    }
    lua_settop(L, 4);
    int x1 = luaL_checkinteger(L, -4);
    int y1 = luaL_checkinteger(L, -3);
    int x2 = luaL_checkinteger(L, -2);
    int y2 = luaL_checkinteger(L, -1);

    Hardware::screen.drawLine(x1, y1, x2, y2);
    return (0);
}

int graphics_drawRect(lua_State *L)
{
    if (!graphicsCallAllowed(L, "drawRect")) {
        return (0);
    }
    lua_settop(L, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int width = luaL_checkinteger(L, -2);
    int height = luaL_checkinteger(L, -1);

    Hardware::screen.drawRect(x, y, width, height);
    return (0);
}

int graphics_fillRect(lua_State *L)
{
    if (!graphicsCallAllowed(L, "fillRect")) {
        return (0);
    }
    lua_settop(L, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int width = luaL_checkinteger(L, -2);
    int height = luaL_checkinteger(L, -1);

    Hardware::screen.fillRect(x, y, width, height);
    return (0);
}

int graphics_drawRoundRect(lua_State *L)
{
    if (!graphicsCallAllowed(L, "drawRoundRect")) {
        return (0);
    }
    lua_settop(L, 5);
    int x = luaL_checkinteger(L, -5);
    int y = luaL_checkinteger(L, -4);
    int width = luaL_checkinteger(L, -3);
    int height = luaL_checkinteger(L, -2);
    int radius = luaL_checkinteger(L, -1);

    Hardware::screen.drawRoundRect(x, y, width, height, radius);
    return (0);
}

int graphics_fillRoundRect(lua_State *L)
{
    if (!graphicsCallAllowed(L, "fillRoundRect")) {
        return (0);
    }
    lua_settop(L, 5);
    int x = luaL_checkinteger(L, -5);
    int y = luaL_checkinteger(L, -4);
    int width = luaL_checkinteger(L, -3);
    int height = luaL_checkinteger(L, -2);
    int radius = luaL_checkinteger(L, -1);

    Hardware::screen.fillRoundRect(x, y, width, height, radius);
    return (0);
}

int graphics_drawTriangle(lua_State *L)
{
    if (!graphicsCallAllowed(L, "drawTriangle")) {
        return (0);
    }
    lua_settop(L, 6);
    int x1 = luaL_checkinteger(L, -6);
    int y1 = luaL_checkinteger(L, -5);
    int x2 = luaL_checkinteger(L, -4);
    int y2 = luaL_checkinteger(L, -3);
    int x3 = luaL_checkinteger(L, -2);
    int y3 = luaL_checkinteger(L, -1);

    Hardware::screen.drawTriangle(x1, y1, x2, y2, x3, y3);
    return (0);
}

int graphics_fillTriangle(lua_State *L)
{
    if (!graphicsCallAllowed(L, "fillTriangle")) {
        return (0);
    }
    lua_settop(L, 6);
    int x1 = luaL_checkinteger(L, -6);
    int y1 = luaL_checkinteger(L, -5);
    int x2 = luaL_checkinteger(L, -4);
    int y2 = luaL_checkinteger(L, -3);
    int x3 = luaL_checkinteger(L, -2);
    int y3 = luaL_checkinteger(L, -1);

    Hardware::screen.fillTriangle(x1, y1, x2, y2, x3, y3);
    return (0);
}

int graphics_drawCircle(lua_State *L)
{
    if (!graphicsCallAllowed(L, "drawCircle")) {
        return (0);
    }
    lua_settop(L, 3);
    int centreX = luaL_checkinteger(L, -3);
    int centreY = luaL_checkinteger(L, -2);
    int radius = luaL_checkinteger(L, -1);

    Hardware::screen.drawCircle(centreX, centreY, radius);
    return (0);
}

int graphics_fillCircle(lua_State *L)
{
    if (!graphicsCallAllowed(L, "fillCircle")) {
        return (0);
    }
    lua_settop(L, 3);
    int centreX = luaL_checkinteger(L, -3);
    int centreY = luaL_checkinteger(L, -2);
    int radius = luaL_checkinteger(L, -1);

    Hardware::screen.fillCircle(centreX, centreY, radius);
    return (0);
}

int graphics_drawEllipse(lua_State *L)
{
    if (!graphicsCallAllowed(L, "drawEllipse")) {
        return (0);
    }
    lua_settop(L, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int radiusX = luaL_checkinteger(L, -2);
    int radiusY = luaL_checkinteger(L, -1);

    Hardware::screen.drawElipse(x, y, radiusX, radiusY);
    return (0);
}

int graphics_fillEllipse(lua_State *L)
{
    if (!graphicsCallAllowed(L, "fillEllipse")) {
        return (0);
    }
    lua_settop(L, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int radiusX = luaL_checkinteger(L, -2);
    int radiusY = luaL_checkinteger(L, -1);

    Hardware::screen.fillElipse(x, y, radiusX, radiusY);
    return (0);
}

int graphics_fillCurve(lua_State *L)
{
    if (!graphicsCallAllowed(L, "fillCurve")) {
        return (0);
    }
    lua_settop(L, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int radius = luaL_checkinteger(L, -2);
    int segment = luaL_checkinteger(L, -1);

    Hardware::screen.fillCurve(x, y, radius, segment);
    return (0);
}

/**
 * @brief graphics.print(x, y, text, width, align [, fontface, size, spacing])
 *
 * The three trailing arguments are optional and default to REGULAR, MEDIUM and
 * PROPORTIONAL. They are checked rather than cast, so a preset naming a size
 * that does not exist is told so at the call rather than drawing something
 * arbitrary.
 */
int graphics_print(lua_State *L)
{
    if (!graphicsCallAllowed(L, "print")) {
        return (0);
    }

    const int arguments = lua_gettop(L);

    lua_settop(L, 8);

    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    const char *text = luaL_checkstring(L, 3);
    int width = luaL_checkinteger(L, 4);
    int align = luaL_checkinteger(L, 5);

    int face = (int)TextFace::regular;
    int size = (int)TextSize::medium;
    int spacing = (int)TextSpacing::proportional;

    if (arguments >= 6) {
        face = luaL_checkinteger(L, 6);

        if (!isTextFace(face)) {
            return (
                luaL_error(L, "graphics.print: unknown fontface: %d", face));
        }
    }

    if (arguments >= 7) {
        size = luaL_checkinteger(L, 7);

        if (!isTextSize(size)) {
            return (luaL_error(L, "graphics.print: unknown size: %d", size));
        }
    }

    if (arguments >= 8) {
        spacing = luaL_checkinteger(L, 8);

        if (!isTextSpacing(spacing)) {
            return (
                luaL_error(L, "graphics.print: unknown spacing: %d", spacing));
        }
    }

    /* The layered renderer rather than the CGRAM one: anti-aliased, in the
     * colour graphics.setColor left behind, and with no text cursor to walk
     * between characters. */
    Hardware::screen.user().printText(x,
                                      y,
                                      text,
                                      width,
                                      (TextAlign)align,
                                      Hardware::screen.Geometry::getColour(),
                                      (TextSize)size,
                                      (TextFace)face,
                                      (TextSpacing)spacing);
    return (0);
}

/** @brief One channel of a 24 bit colour, clamped to 0..255. */
static uint32_t channel(lua_Number value)
{
    if (value < 0.0) {
        return (0);
    }
    if (value > 255.0) {
        return (255);
    }
    return ((uint32_t)(value + 0.5));
}

int graphics_rgb(lua_State *L)
{
    lua_settop(L, 3);

    const uint32_t red = channel(luaL_checknumber(L, 1));
    const uint32_t green = channel(luaL_checknumber(L, 2));
    const uint32_t blue = channel(luaL_checknumber(L, 3));

    lua_pushinteger(L, (lua_Integer)((red << 16) | (green << 8) | blue));
    return (1);
}

int graphics_dim(lua_State *L)
{
    lua_settop(L, 2);

    const uint32_t colour = (uint32_t)luaL_checkinteger(L, 1);
    const lua_Number factor = luaL_checknumber(L, 2);

    if (factor < 0.0) {
        return (luaL_error(L, "failed: factor must not be negative"));
    }

    /* The controller's own arithmetic, not a second opinion: this is what
     * darkens an overlay's colour for a group tint, so a script asking for a
     * dimmer shade of something gets the shade the panel would have drawn. */
    lua_pushinteger(L, (lua_Integer)Colours888::darker(colour, (float)factor));
    return (1);
}

int graphics_blend(lua_State *L)
{
    lua_settop(L, 3);

    const uint32_t from = (uint32_t)luaL_checkinteger(L, 1);
    const uint32_t to = (uint32_t)luaL_checkinteger(L, 2);
    lua_Number position = luaL_checknumber(L, 3);

    if (position < 0.0) {
        position = 0.0;
    } else if (position > 1.0) {
        position = 1.0;
    }

    uint32_t mixed = 0;

    for (int shift = 16; shift >= 0; shift -= 8) {
        const lua_Number a = (lua_Number)((from >> shift) & 0xff);
        const lua_Number b = (lua_Number)((to >> shift) & 0xff);

        mixed |= channel(a + ((b - a) * position)) << shift;
    }

    lua_pushinteger(L, (lua_Integer)mixed);
    return (1);
}

/** @brief face, size and spacing from the tail of the argument list. */
static bool readFace(lua_State *L,
                     const char *function,
                     int firstArgument,
                     int arguments,
                     int &face,
                     int &size,
                     int &spacing)
{
    face = (int)TextFace::regular;
    size = (int)TextSize::medium;
    spacing = (int)TextSpacing::proportional;

    if (arguments >= firstArgument) {
        face = luaL_checkinteger(L, firstArgument);

        if (!isTextFace(face)) {
            luaL_error(L, "graphics.%s: unknown fontface: %d", function, face);
            return (false);
        }
    }

    if (arguments >= (firstArgument + 1)) {
        size = luaL_checkinteger(L, firstArgument + 1);

        if (!isTextSize(size)) {
            luaL_error(L, "graphics.%s: unknown size: %d", function, size);
            return (false);
        }
    }

    if (arguments >= (firstArgument + 2)) {
        spacing = luaL_checkinteger(L, firstArgument + 2);

        if (!isTextSpacing(spacing)) {
            luaL_error(
                L, "graphics.%s: unknown spacing: %d", function, spacing);
            return (false);
        }
    }

    return (true);
}

int graphics_getTextWidth(lua_State *L)
{
    const int arguments = lua_gettop(L);

    lua_settop(L, 4);

    const char *text = luaL_checkstring(L, 1);

    int face = 0;
    int size = 0;
    int spacing = 0;

    if (!readFace(L, "getTextWidth", 2, arguments, face, size, spacing)) {
        return (0);
    }

    lua_pushinteger(
        L,
        Hardware::screen.user().getTextWidth(
            text, (TextSize)size, (TextFace)face, (TextSpacing)spacing));
    return (1);
}

int graphics_getTextHeight(lua_State *L)
{
    const int arguments = lua_gettop(L);

    lua_settop(L, 3);

    int face = 0;
    int size = 0;
    int spacing = 0;

    if (!readFace(L, "getTextHeight", 1, arguments, face, size, spacing)) {
        return (0);
    }

    lua_pushinteger(
        L,
        Hardware::screen.user().getLineHeight((TextSize)size, (TextFace)face));
    return (1);
}

int graphics_drawArc(lua_State *L)
{
    if (!graphicsCallAllowed(L, "drawArc")) {
        return (0);
    }

    const int arguments = lua_gettop(L);

    lua_settop(L, 6);

    const lua_Number centreX = luaL_checknumber(L, 1);
    const lua_Number centreY = luaL_checknumber(L, 2);
    const lua_Number radius = luaL_checknumber(L, 3);
    const lua_Number fromDegrees = luaL_checknumber(L, 4);
    const lua_Number toDegrees = luaL_checknumber(L, 5);

    lua_Number thickness = 1.0;

    if (arguments >= 6) {
        thickness = luaL_checknumber(L, 6);
    }

    if (radius <= 0.0) {
        return (luaL_error(L, "graphics.drawArc: radius must be positive"));
    }

    if (thickness < 1.0) {
        thickness = 1.0;
    }

    const lua_Number inner =
        (radius - (thickness / 2.0) < 0.0) ? 0.0 : radius - (thickness / 2.0);
    const lua_Number outer = radius + (thickness / 2.0);

    lua_Number sweep = toDegrees - fromDegrees;

    if (sweep < 0.0) {
        sweep = -sweep;
    }

    if (sweep > 360.0) {
        sweep = 360.0;
    }

    /*
     * One step per degree of sweep, and never fewer than four: a short arc
     * still wants a couple of segments to read as a curve, and a long one at
     * this radius does not get smoother for having more than the pixels it
     * covers. Bounded so a script asking for a full circle at a huge radius
     * cannot spend the whole frame here.
     */
    int steps = (int)(sweep + 0.5);

    if (steps < 4) {
        steps = 4;
    } else if (steps > 360) {
        steps = 360;
    }

    const lua_Number direction = (toDegrees >= fromDegrees) ? 1.0 : -1.0;
    const lua_Number stepDegrees = (sweep / (lua_Number)steps) * direction;

    /* Clockwise from twelve o'clock, on a screen whose y grows downwards. */
    auto pointAt =
        [&](lua_Number degrees, lua_Number r, uint16_t &x, uint16_t &y) {
            const lua_Number radians = (degrees - 90.0) * (M_PI / 180.0);
            x = (uint16_t)(centreX + (std::cos(radians) * r) + 0.5);
            y = (uint16_t)(centreY + (std::sin(radians) * r) + 0.5);
        };

    for (int step = 0; step < steps; step++) {
        const lua_Number a = fromDegrees + (stepDegrees * (lua_Number)step);
        const lua_Number b = a + stepDegrees;

        uint16_t innerAx, innerAy, outerAx, outerAy;
        uint16_t innerBx, innerBy, outerBx, outerBy;

        pointAt(a, inner, innerAx, innerAy);
        pointAt(a, outer, outerAx, outerAy);
        pointAt(b, inner, innerBx, innerBy);
        pointAt(b, outer, outerBx, outerBy);

        /* The segment as two triangles, which is the only filled shape with
         * arbitrary corners the driver offers. */

        Hardware::screen.fillTriangle(
            innerAx, innerAy, outerAx, outerAy, outerBx, outerBy);
        Hardware::screen.fillTriangle(
            innerAx, innerAy, outerBx, outerBy, innerBx, innerBy);
    }

    return (0);
}
