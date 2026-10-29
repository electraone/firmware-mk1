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

#include "luaHelpers.h"
#include "SignMode.h"
#include <cmath>

int luaopen_helpers(lua_State *L)
{
    luaL_newlib(L, helpers_functions);
    return 1;
}

int helpers_slotToBounds(lua_State *L)
{
    lua_settop(L, 1);

    int slot = luaL_checkinteger(L, 1);

    luaL_argcheck(
        L, 1 <= slot && slot <= 36, 1, "failed: slot must be between 1 and 36");

    Rectangle bounds = controlSlotToBounds(slot);

    /* The size comes from the rectangle rather than being written out again
     * here, so that a change to the slot geometry cannot leave the two
     * disagreeing. */
    lua_newtable(L);
    luaLE_pushArrayInteger(L, 1, bounds.getX());
    luaLE_pushArrayInteger(L, 2, bounds.getY());
    luaLE_pushArrayInteger(L, 3, bounds.getWidth());
    luaLE_pushArrayInteger(L, 4, bounds.getHeight());

    return (1);
}

int helpers_boundsToSlot(lua_State *L)
{
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    // parse type
    lua_rawgeti(L, 1, 1);
    lua_rawgeti(L, 1, 2);
    lua_rawgeti(L, 1, 3);
    lua_rawgeti(L, 1, 4);
    int x = luaL_checkinteger(L, -4);
    int y = luaL_checkinteger(L, -3);
    int width = luaL_checkinteger(L, -2);
    int height = luaL_checkinteger(L, -1);

    uint8_t slot = controlBoundsToSlot(Rectangle(x, y, width, height));

    lua_pushinteger(L, slot);
    return (1);
}

int helpers_delay(lua_State *L)
{
    lua_settop(L, 1);

    int msecs = luaL_checkinteger(L, 1);

    luaL_argcheck(L,
                  1 <= msecs && msecs <= 5000,
                  1,
                  "failed: time interval must be between 1 and 5000");

    delay(msecs);

    return (0);
}

int helpers_clamp(lua_State *L)
{
    lua_settop(L, 3);

    const lua_Number value = luaL_checknumber(L, 1);
    const lua_Number low = luaL_checknumber(L, 2);
    const lua_Number high = luaL_checknumber(L, 3);

    /* Given the wrong way round it still answers inside the range, rather
     * than answering `low` for everything the way a naive pair of ifs does. */
    const lua_Number lower = (low <= high) ? low : high;
    const lua_Number upper = (low <= high) ? high : low;

    lua_pushnumber(L, (value < lower) ? lower : (value > upper) ? upper : value);
    return (1);
}

/** @brief The shared body of map() and scale(); curve 1.0 is a straight line. */
static int mapWithCurve(lua_State *L, lua_Number curve)
{
    const lua_Number value = luaL_checknumber(L, 1);
    const lua_Number inLow = luaL_checknumber(L, 2);
    const lua_Number inHigh = luaL_checknumber(L, 3);
    const lua_Number outLow = luaL_checknumber(L, 4);
    const lua_Number outHigh = luaL_checknumber(L, 5);

    /* An empty input range has no answer to give. Returning outLow would be a
     * plausible wrong number, and the caller cannot tell it from a real one.
     *
     * Compared against a tolerance rather than for equality - the target
     * builds with -Werror=float-equal, and a range too narrow to divide by
     * answers nonsense just as surely as one that is exactly empty. */
    if (std::fabs(inHigh - inLow) < 1e-9) {
        return (luaL_error(
            L, "failed: the input range is empty (%f to %f)", inLow, inHigh));
    }

    lua_Number position = (value - inLow) / (inHigh - inLow);

    if (position < 0.0) {
        position = 0.0;
    } else if (position > 1.0) {
        position = 1.0;
    }

    if (std::fabs(curve - 1.0) > 1e-9) {
        position = std::pow(position, curve);
    }

    lua_pushnumber(L, outLow + (position * (outHigh - outLow)));
    return (1);
}

int helpers_map(lua_State *L)
{
    lua_settop(L, 5);
    return (mapWithCurve(L, 1.0));
}

int helpers_scale(lua_State *L)
{
    const int arguments = lua_gettop(L);

    lua_settop(L, 6);

    /*
     * The exponent applied to the position before it is spread over the output
     * range: above 1 the low end gets more of the travel, below 1 the high
     * end does. One is a straight line, and is the default so that scale() and
     * map() agree when the curve is left out.
     */
    lua_Number curve = 1.0;

    if (arguments >= 6) {
        curve = luaL_checknumber(L, 6);

        if (curve <= 0.0) {
            return (luaL_error(L, "failed: curve must be greater than zero"));
        }
    }

    return (mapWithCurve(L, curve));
}

/** @brief A signed encoding as a script names it, and the numbers it holds. */
struct SignedEncoding {
    SignMode signMode;
    uint8_t bitWidth;
    int16_t min;
    int16_t max;
    uint16_t midiMax;
};

/**
 * @brief Reads the optional bitWidth and signMode arguments.
 *
 * Fourteen bits is the widest value a message carries; a wider one would not
 * fit the int16_t the translation works in. Only the two absolute signed
 * encodings are accepted. signBit2 and binOffset are relative modes, and a
 * control with no sign maps its MIDI range onto its display range, which
 * needs the control's ranges and is not a question of notation.
 */
static SignedEncoding checkSignedEncoding(lua_State *L)
{
    const lua_Integer bitWidth = luaL_optinteger(L, 2, 7);

    luaL_argcheck(L,
                  (1 <= bitWidth) && (bitWidth <= 14),
                  2,
                  "failed: bitWidth must be between 1 and 14");

    const SignMode signMode =
        translateSignMode(luaL_optstring(L, 3, "twosComplement"));

    luaL_argcheck(L,
                  (signMode == SignMode::twosComplement)
                      || (signMode == SignMode::signBit),
                  3,
                  "failed: signMode must be twosComplement or signBit");

    SignedEncoding encoding;

    encoding.signMode = signMode;
    encoding.bitWidth = static_cast<uint8_t>(bitWidth);
    encoding.max = getMaxSignedValue(encoding.bitWidth);

    /* Two's complement has one more negative number than positive ones. A sign
     * bit spends the pattern on minus zero instead. */
    encoding.min = (signMode == SignMode::twosComplement) ? -encoding.max - 1
                                                          : -encoding.max;
    encoding.midiMax = getMaxUnsignedValue(encoding.bitWidth);

    return (encoding);
}

int helpers_midiToSigned(lua_State *L)
{
    lua_settop(L, 3);

    const lua_Integer midiValue = luaL_checkinteger(L, 1);
    const SignedEncoding encoding = checkSignedEncoding(L);

    /* The E1 masks a value wider than its message down to the bits the
     * message has. A script that holds one has a bug, and a masked answer
     * would hide it. */
    luaL_argcheck(L,
                  (0 <= midiValue) && (midiValue <= encoding.midiMax),
                  1,
                  "failed: midiValue does not fit the bitWidth");

    lua_pushinteger(L,
                    translateMidiValueToValue(encoding.signMode,
                                              encoding.bitWidth,
                                              static_cast<uint16_t>(midiValue),
                                              0,
                                              encoding.midiMax,
                                              encoding.min,
                                              encoding.max));
    return (1);
}

int helpers_signedToMidi(lua_State *L)
{
    lua_settop(L, 3);

    const lua_Integer value = luaL_checkinteger(L, 1);
    const SignedEncoding encoding = checkSignedEncoding(L);

    /* Clamped, as the E1 clamps a displayed value the encoding cannot hold.
     * Done here as well so a number past int16_t does not wrap on the way in. */
    const lua_Integer held = (value < encoding.min)   ? encoding.min
                             : (value > encoding.max) ? encoding.max
                                                      : value;

    lua_pushinteger(L,
                    translateValueToMidiValue(encoding.signMode,
                                              encoding.bitWidth,
                                              static_cast<int16_t>(held),
                                              encoding.min,
                                              encoding.max,
                                              0,
                                              encoding.midiMax));
    return (1);
}
