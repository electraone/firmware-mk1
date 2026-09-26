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

#include "luaController.h"
#include "compat.h"
#include "settings.h"
#include <cstdlib>
#include <cstring>

namespace
{
    /** The name of this panel, as a script written for either model spells it. */
    const char *modelName = "mk1";

    /**
     * @brief A version as one comparable number, the way the firmware keeps it.
     *
     * FIRMWARE_SEQ is major * 100000000 + minor * 100000 + patch * 100, which
     * leaves room between releases for the builds in between. A version a
     * script asks for is turned into the same shape so that the two can simply
     * be compared.
     *
     * Accepts "3.5.4", "v3.5.4", "3.5" and "3", and stops at the first thing
     * that is not part of a version - so a release name with something after it
     * is read as far as it makes sense rather than rejected.
     */
    uint32_t versionToNumber(const char *version)
    {
        if (version == nullptr) {
            return (0);
        }

        if ((*version == 'v') || (*version == 'V')) {
            version++;
        }

        uint32_t parts[3] = { 0, 0, 0 };

        for (int part = 0; part < 3; part++) {
            char *end = nullptr;
            const unsigned long value = strtoul(version, &end, 10);

            if (end == version) {
                break;
            }

            parts[part] = (uint32_t)value;
            version = end;

            if (*version != '.') {
                break;
            }
            version++;
        }

        return ((parts[0] * 100000000) + (parts[1] * 100000)
                + (parts[2] * 100));
    }

    /**
     * @brief Whether this controller meets what a script asks for.
     *
     * The model is given either as a name or as one of the model numbers, so
     * that both controller.require("mk2", ...) and
     * controller.require(MODEL_MK2, ...) work. A model this is not is never
     * met, whatever the version: the point of the call is to stop a preset
     * that needs the other panel from half running on this one.
     */
    bool requirementsMet(lua_State *L, int modelIndex, int versionIndex)
    {
        if (!lua_isnoneornil(L, modelIndex)) {
            if (lua_type(L, modelIndex) == LUA_TSTRING) {
                if (strcmp(lua_tostring(L, modelIndex), modelName) != 0) {
                    return (false);
                }
            } else if ((int)luaL_checkinteger(L, modelIndex)
                       != (int)ControllerModel::mk1) {
                return (false);
            }
        }

        if (lua_isnoneornil(L, versionIndex)) {
            return (true);
        }

        uint32_t required = 0;

        if (lua_type(L, versionIndex) == LUA_TSTRING) {
            required = versionToNumber(lua_tostring(L, versionIndex));
        } else {
            required = (uint32_t)luaL_checkinteger(L, versionIndex);

            /* A bare number is read as the sequence the firmware reports, not
             * as a major version: telling the two apart by size would guess. */
        }

        return (FIRMWARE_SEQ >= required);
    }
} // namespace

int luaopen_controller(lua_State *L)
{
    luaL_newlib(L, controller_functions);
    return (1);
}

int controller_uptime(lua_State *L)
{
    lua_settop(L, 0);
    lua_pushnumber(L, millis());
    return (1);
}

int controller_getModel(lua_State *L)
{
    lua_settop(L, 0);
    lua_pushstring(L, modelName);
    return (1);
}

int controller_getNumModel(lua_State *L)
{
    lua_settop(L, 0);
    lua_pushinteger(L, (int)ControllerModel::mk1);
    return (1);
}

int controller_getFirmwareVersion(lua_State *L)
{
    lua_settop(L, 0);
    lua_pushstring(L, FIRMWARE_VERSION);
    return (1);
}

int controller_getFirmwareNumVersion(lua_State *L)
{
    lua_settop(L, 0);
    lua_pushinteger(L, FIRMWARE_SEQ);
    return (1);
}

int controller_require(lua_State *L)
{
    lua_settop(L, 2);

    if (!requirementsMet(L, 1, 2)) {
        return (luaL_error(L,
                           "this preset needs a different controller: have "
                           "%s %s",
                           modelName,
                           FIRMWARE_VERSION));
    }

    return (0);
}

int controller_isRequired(lua_State *L)
{
    lua_settop(L, 2);
    lua_pushboolean(L, requirementsMet(L, 1, 2));
    return (1);
}
