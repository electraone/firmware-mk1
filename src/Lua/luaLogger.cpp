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

#include "luaLogger.h"
#include "System.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace
{
    /** @brief Longer than a log message can be; the logger cuts it anyway. */
    constexpr size_t TextSize = 256;

    /** @brief The longest conversion spec kept: %, flags, width, precision. */
    constexpr size_t SpecSize = 32;

    /** @brief Appends to a fixed buffer, dropping whatever does not fit. */
    struct Text {
        char buffer[TextSize];
        size_t length = 0;

        Text()
        {
            buffer[0] = '\0';
        }

        size_t room(void) const
        {
            return (sizeof(buffer) - length);
        }

        void addCharacter(char character)
        {
            if (room() > 1) {
                buffer[length++] = character;
                buffer[length] = '\0';
            }
        }

        /** @brief Advances past what snprintf wrote, or up to the end. */
        void advance(int written)
        {
            if (written > 0) {
                length += std::min(static_cast<size_t>(written), room() - 1);
            }
        }
    };

    /**
     * @brief Copies one conversion spec, from '%' up to its letter.
     *
     * The flags, width and precision string.format accepts: "-+ #0", then up
     * to two digits of width, then '.' and up to two digits of precision.
     * Length modifiers are not accepted - a Lua number is not a C type, and
     * the conversion letter alone says how it is read.
     *
     * @return the conversion letter, with @p spec holding the spec without it.
     */
    char readSpec(lua_State *L, const char *&cursor, char *spec)
    {
        size_t length = 0;

        spec[length++] = '%';

        while ((*cursor != '\0') && (strchr("-+ #0", *cursor) != nullptr)) {
            if (length >= 6) {
                luaL_error(L, "logger.write: invalid format (repeated flags)");
            }
            spec[length++] = *cursor++;
        }

        for (int digits = 0; (digits < 2) && isdigit((unsigned char)*cursor);
             digits++) {
            spec[length++] = *cursor++;
        }

        if (*cursor == '.') {
            spec[length++] = *cursor++;

            for (int digits = 0;
                 (digits < 2) && isdigit((unsigned char)*cursor);
                 digits++) {
                spec[length++] = *cursor++;
            }
        }

        if (isdigit((unsigned char)*cursor)) {
            luaL_error(L, "logger.write: invalid format (width or precision "
                          "too long)");
        }

        spec[length] = '\0';

        return (*cursor);
    }

    /** @brief The spec with @p modifier and @p conversion appended. */
    void finishSpec(char *spec, const char *modifier, char conversion)
    {
        size_t length = strlen(spec);

        strcpy(spec + length, modifier);
        length += strlen(modifier);
        spec[length++] = conversion;
        spec[length] = '\0';
    }
} // namespace

int luaopen_logger(lua_State *L)
{
    luaL_newlib(L, logger_functions);
    return (1);
}

int logger_write(lua_State *L)
{
    const char *cursor = luaL_checkstring(L, 1);
    const int arguments = lua_gettop(L);
    int argument = 1;
    Text text;

    while (*cursor != '\0') {
        if (*cursor != '%') {
            text.addCharacter(*cursor++);
            continue;
        }

        cursor++;

        if (*cursor == '%') {
            text.addCharacter(*cursor++);
            continue;
        }

        char spec[SpecSize];
        const char conversion = readSpec(L, cursor, spec);

        if (conversion == '\0') {
            return (luaL_error(L, "logger.write: invalid format (ends in '%%')"));
        }

        cursor++;
        argument++;

        /* Refused rather than read as nil, as string.format refuses it. */
        if (argument > arguments) {
            return (luaL_argerror(L, argument, "no value"));
        }

        switch (conversion) {
            case 'd':
            case 'i': {
                const long long value = luaL_checkinteger(L, argument);
                finishSpec(spec, "ll", conversion);
                text.advance(snprintf(
                    text.buffer + text.length, text.room(), spec, value));
                break;
            }

            case 'u':
            case 'o':
            case 'x':
            case 'X': {
                /* Through lua_Unsigned, so -1 is the width of a Lua
                 * integer in ones - 32 bits on this build - as
                 * string.format writes it, and not 64. */
                const unsigned long long value = static_cast<lua_Unsigned>(
                    luaL_checkinteger(L, argument));
                finishSpec(spec, "ll", conversion);
                text.advance(snprintf(
                    text.buffer + text.length, text.room(), spec, value));
                break;
            }

            case 'c': {
                const int value =
                    static_cast<int>(luaL_checkinteger(L, argument));
                finishSpec(spec, "", conversion);
                text.advance(snprintf(
                    text.buffer + text.length, text.room(), spec, value));
                break;
            }

            case 'a':
            case 'A':
            case 'e':
            case 'E':
            case 'f':
            case 'F':
            case 'g':
            case 'G': {
                const double value = luaL_checknumber(L, argument);
                finishSpec(spec, "", conversion);
                text.advance(snprintf(
                    text.buffer + text.length, text.room(), spec, value));
                break;
            }

            case 's': {
                /* Anything, as print() takes anything: tostring() and a
                 * __tostring metamethod both apply. */
                const char *value = luaL_tolstring(L, argument, nullptr);
                finishSpec(spec, "", conversion);
                text.advance(snprintf(
                    text.buffer + text.length, text.room(), spec, value));
                lua_pop(L, 1);
                break;
            }

            default:
                return (luaL_error(
                    L,
                    "logger.write: invalid conversion '%%%c' to format",
                    conversion));
        }
    }

    System::logger.write(LOG_LUA, "lua: %s", text.buffer);

    return (0);
}
