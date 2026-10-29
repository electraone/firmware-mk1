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

#include "luaBytes.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace
{
    /**
     * @brief The bytes a caller handed in.
     *
     * A Lua string or an array of numbers. The same rule, and deliberately the
     * same wording of the failure, as sysexBlock's readByteRun().
     */
    bool readBytes(lua_State *L,
                   int arg,
                   const char *what,
                   std::vector<uint8_t> &bytes)
    {
        if (lua_type(L, arg) == LUA_TSTRING) {
            size_t length = 0;
            const char *text = lua_tolstring(L, arg, &length);

            bytes.assign(text, text + length);
            return (true);
        }

        if (lua_type(L, arg) == LUA_TTABLE) {
            const lua_Integer length = luaL_len(L, arg);

            for (lua_Integer i = 1; i <= length; i++) {
                lua_rawgeti(L, arg, i);

                if (!lua_isinteger(L, -1)) {
                    lua_pop(L, 1);
                    luaL_error(L, "%s: item %d is not a byte", what, (int)i);
                    return (false);
                }

                const lua_Integer value = lua_tointeger(L, -1);
                lua_pop(L, 1);

                if ((value < 0) || (value > 255)) {
                    luaL_error(L,
                               "%s: item %d is %d, which is not a byte",
                               what,
                               (int)i,
                               (int)value);
                    return (false);
                }

                bytes.push_back((uint8_t)value);
            }

            return (true);
        }

        luaL_error(L, "%s: expected a string or an array of bytes", what);
        return (false);
    }

    /** @brief An array of numbers, which is what every one of these answers. */
    void pushArray(lua_State *L, const std::vector<uint8_t> &bytes)
    {
        lua_createtable(L, (int)bytes.size(), 0);

        for (size_t i = 0; i < bytes.size(); i++) {
            lua_pushinteger(L, bytes[i]);
            lua_rawseti(L, -2, (lua_Integer)(i + 1));
        }
    }

    int hexDigit(char c)
    {
        if ((c >= '0') && (c <= '9')) {
            return (c - '0');
        }
        if ((c >= 'a') && (c <= 'f')) {
            return (10 + (c - 'a'));
        }
        if ((c >= 'A') && (c <= 'F')) {
            return (10 + (c - 'A'));
        }
        return (-1);
    }
} // namespace

int luaopen_bytes(lua_State *L)
{
    luaL_newlib(L, bytes_functions);
    return (1);
}

int bytes_toHex(lua_State *L)
{
    const int arguments = lua_gettop(L);

    lua_settop(L, 2);

    std::vector<uint8_t> bytes;

    if (!readBytes(L, 1, "bytes.toHex", bytes)) {
        return (0);
    }

    /* Nothing between the pairs unless asked. A separator is what makes the
     * result readable in a log; its absence is what makes it a value another
     * tool can read back. */
    const char *separator = (arguments >= 2) ? luaL_checkstring(L, 2) : "";

    static const char *digits = "0123456789ABCDEF";

    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);

    for (size_t i = 0; i < bytes.size(); i++) {
        if ((i > 0) && (separator[0] != '\0')) {
            luaL_addstring(&buffer, separator);
        }

        luaL_addchar(&buffer, digits[(bytes[i] >> 4) & 0x0f]);
        luaL_addchar(&buffer, digits[bytes[i] & 0x0f]);
    }

    luaL_pushresult(&buffer);
    return (1);
}

int bytes_fromHex(lua_State *L)
{
    lua_settop(L, 1);

    size_t length = 0;
    const char *text = luaL_checklstring(L, 1, &length);

    std::vector<uint8_t> bytes;
    int high = -1;

    /*
     * Anything that is not a hex digit separates: spaces, commas, colons,
     * newlines, "0x". A synth's manual prints its messages in all of those
     * and a preset author pastes what the manual says.
     */
    for (size_t i = 0; i < length; i++) {
        /* A "0x" prefix has to be passed over as a whole: its 0 is a hex
         * digit, and read as one it shifted every byte after it by half -
         * "0x10" was the digits 0, 1, 0. Only where a byte starts, so the 0
         * ending "A0" followed by "x" is still a digit. */
        if ((high < 0) && (text[i] == '0') && ((i + 1) < length)
            && ((text[i + 1] == 'x') || (text[i + 1] == 'X'))) {
            i++;
            continue;
        }

        const int digit = hexDigit(text[i]);

        if (digit < 0) {
            continue;
        }

        if (high < 0) {
            high = digit;
        } else {
            bytes.push_back((uint8_t)((high << 4) | digit));
            high = -1;
        }
    }

    if (high >= 0) {
        return (luaL_error(
            L, "bytes.fromHex: the text ends on half a byte"));
    }

    pushArray(L, bytes);
    return (1);
}

int bytes_pack7(lua_State *L)
{
    lua_settop(L, 1);

    std::vector<uint8_t> bytes;

    if (!readBytes(L, 1, "bytes.pack7", bytes)) {
        return (0);
    }

    /*
     * The MIDI convention: seven data bytes are sent as eight, the first
     * carrying the top bit of each of the seven that follow, low bit first.
     * A SysEx message may not contain a byte with bit 7 set, which is why
     * every instrument that sends more than seven bits of anything does this.
     */
    std::vector<uint8_t> packed;

    for (size_t i = 0; i < bytes.size(); i += 7) {
        const size_t run = ((bytes.size() - i) < 7) ? (bytes.size() - i) : 7;
        uint8_t tops = 0;

        for (size_t j = 0; j < run; j++) {
            if (bytes[i + j] & 0x80) {
                tops |= (uint8_t)(1 << j);
            }
        }

        packed.push_back(tops);

        for (size_t j = 0; j < run; j++) {
            packed.push_back((uint8_t)(bytes[i + j] & 0x7f));
        }
    }

    pushArray(L, packed);
    return (1);
}

int bytes_unpack7(lua_State *L)
{
    lua_settop(L, 1);

    std::vector<uint8_t> bytes;

    if (!readBytes(L, 1, "bytes.unpack7", bytes)) {
        return (0);
    }

    std::vector<uint8_t> unpacked;

    for (size_t i = 0; i < bytes.size(); i += 8) {
        const uint8_t tops = bytes[i];
        const size_t run =
            ((bytes.size() - i - 1) < 7) ? (bytes.size() - i - 1) : 7;

        for (size_t j = 0; j < run; j++) {
            uint8_t value = (uint8_t)(bytes[i + 1 + j] & 0x7f);

            if (tops & (1 << j)) {
                value |= 0x80;
            }

            unpacked.push_back(value);
        }
    }

    pushArray(L, unpacked);
    return (1);
}

int bytes_toNibbles(lua_State *L)
{
    const int arguments = lua_gettop(L);

    lua_settop(L, 2);

    std::vector<uint8_t> bytes;

    if (!readBytes(L, 1, "bytes.toNibbles", bytes)) {
        return (0);
    }

    /* High nibble first unless told otherwise, which is what most
     * instruments send; the ones that do not are why the flag exists. */
    const bool highFirst = (arguments >= 2) ? lua_toboolean(L, 2) : true;

    std::vector<uint8_t> nibbles;

    for (const uint8_t byte : bytes) {
        const uint8_t high = (uint8_t)((byte >> 4) & 0x0f);
        const uint8_t low = (uint8_t)(byte & 0x0f);

        nibbles.push_back(highFirst ? high : low);
        nibbles.push_back(highFirst ? low : high);
    }

    pushArray(L, nibbles);
    return (1);
}

int bytes_fromNibbles(lua_State *L)
{
    const int arguments = lua_gettop(L);

    lua_settop(L, 2);

    std::vector<uint8_t> nibbles;

    if (!readBytes(L, 1, "bytes.fromNibbles", nibbles)) {
        return (0);
    }

    if ((nibbles.size() % 2) != 0) {
        return (luaL_error(
            L, "bytes.fromNibbles: an odd number of nibbles makes no bytes"));
    }

    const bool highFirst = (arguments >= 2) ? lua_toboolean(L, 2) : true;

    std::vector<uint8_t> bytes;

    for (size_t i = 0; i < nibbles.size(); i += 2) {
        const uint8_t first = (uint8_t)(nibbles[i] & 0x0f);
        const uint8_t second = (uint8_t)(nibbles[i + 1] & 0x0f);

        bytes.push_back(highFirst ? (uint8_t)((first << 4) | second)
                                  : (uint8_t)((second << 4) | first));
    }

    pushArray(L, bytes);
    return (1);
}

int bytes_checksum(lua_State *L)
{
    const int arguments = lua_gettop(L);

    lua_settop(L, 2);

    std::vector<uint8_t> bytes;

    if (!readBytes(L, 1, "bytes.checksum", bytes)) {
        return (0);
    }

    const char *kind = (arguments >= 2) ? luaL_checkstring(L, 2) : "roland";

    uint32_t sum = 0;
    uint8_t answer = 0;

    if (strcmp(kind, "roland") == 0) {
        /* The one most instruments use, Roland's among them: the seven bit
         * value that makes the running total come out a multiple of 128. */
        for (const uint8_t byte : bytes) {
            sum += byte;
        }
        answer = (uint8_t)((128 - (sum & 0x7f)) & 0x7f);
    } else if (strcmp(kind, "sum") == 0) {
        for (const uint8_t byte : bytes) {
            sum += byte;
        }
        answer = (uint8_t)(sum & 0x7f);
    } else if (strcmp(kind, "xor") == 0) {
        for (const uint8_t byte : bytes) {
            answer ^= byte;
        }
        answer &= 0x7f;
    } else {
        return (luaL_error(L,
                           "bytes.checksum: unknown kind '%s' - "
                           "expected roland, sum or xor",
                           kind));
    }

    lua_pushinteger(L, answer);
    return (1);
}

int bytes_toString(lua_State *L)
{
    lua_settop(L, 1);

    std::vector<uint8_t> bytes;

    if (!readBytes(L, 1, "bytes.toString", bytes)) {
        return (0);
    }

    /* The other direction from an array, for the calls that want a string -
     * midi.sendSysex takes one, and so does anything doing string.find over
     * a reply. */
    lua_pushlstring(L, (const char *)bytes.data(), bytes.size());
    return (1);
}
