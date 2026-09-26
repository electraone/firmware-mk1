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

#include "luaParameterMap.h"
#include "luaExtension.h"

int luaopen_parameterMap(lua_State *L)
{
    luaL_newlib(L, parameterMap_functions);
    return 1;
}

int parameterMap_resetAll([[maybe_unused]] lua_State *L)
{
    /* Resetting is setting every value to zero, which is what resetDevice()
     * does for one device. It used to call clear(), which throws the map away
     * along with every control's link into it: after one call the preset's
     * faders moved nothing, and nothing said why. */
    parameterMap.resetAllValues();
    return (0);
}

int parameterMap_resetDevice(lua_State *L)
{
    lua_settop(L, 1);
    int deviceId = luaLE_checkDeviceId(L, 1);
    parameterMap.resetDeviceValues(deviceId);
    return (0);
}

int parameterMap_set(lua_State *L)
{
    lua_settop(L, 4);
    int deviceId = luaLE_checkDeviceId(L, 1);
    int type = luaLE_checkParameterType(L, 2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, 3);
    uint16_t midiValue = luaLE_checkMidiValue(L, 4);

    parameterMap.setValue(
        deviceId, (Message::Type)type, parameterNumber, midiValue, Origin::lua);
    return (0);
}

int parameterMap_apply(lua_State *L)
{
    lua_settop(L, 4);

    int deviceId = luaLE_checkDeviceId(L, 1);
    int type = luaLE_checkParameterType(L, 2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, 3);
    uint16_t midiValue = luaLE_checkMidiValue(L, 4);

    parameterMap.applyToValue(
        deviceId, (Message::Type)type, parameterNumber, midiValue, Origin::lua);
    return (0);
}

int parameterMap_modulate(lua_State *L)
{
    lua_settop(L, 5);

    int deviceId = luaLE_checkDeviceId(L, 1);
    int type = luaLE_checkParameterType(L, 2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, 3);
    float modulationValue = luaL_checknumber(L, 4);
    int depth = luaL_checkinteger(L, 5);

    parameterMap.modulateValue(
        deviceId, (Message::Type)type, parameterNumber, modulationValue, depth);

    return (0);
}

int parameterMap_send(lua_State *L)
{
    lua_settop(L, 3);

    /* Checked and then dropped: the arguments are still validated so that a
     * preset calling this is told about a bad device or parameter, but nothing
     * is sent. Kept only so that such a preset still loads. */
    (void)luaLE_checkDeviceId(L, 1);
    (void)luaLE_checkParameterType(L, 2);
    (void)luaLE_checkParameterNumber(L, 3);

    System::logger.write(LOG_LUA, "parameterMap.send is deprecated");
    return (0);
}

/**
 * @brief parameterMap.updateValue(deviceId, type, parameterNumber, midiValue)
 *
 * Stores the value and repaints whatever shows it, but sends no MIDI and calls
 * no Lua back - which is what a script answering a device's own report needs,
 * so that the report does not come straight back out as a command.
 */
int parameterMap_updateValue(lua_State *L)
{
    lua_settop(L, 4);

    int deviceId = luaLE_checkDeviceId(L, 1);
    int type = luaLE_checkParameterType(L, 2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, 3);
    uint16_t midiValue = luaLE_checkMidiValue(L, 4);

    parameterMap.setValueSimple(
        deviceId, (Message::Type)type, parameterNumber, midiValue);
    return (0);
}

int parameterMap_get(lua_State *L)
{
    lua_settop(L, 3);

    int deviceId = luaLE_checkDeviceId(L, 1);
    int type = luaLE_checkParameterType(L, 2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, 3);

    uint16_t midiValue =
        parameterMap.getValue(deviceId, (Message::Type)type, parameterNumber);

    lua_pushnumber(L, midiValue);
    return (1);
}

int parameterMap_getValues(lua_State *L)
{
    lua_settop(L, 3);

    int deviceId = luaLE_checkDeviceId(L, 1);
    int type = luaLE_checkParameterType(L, 2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, 3);

    LookupEntry *entry =
        parameterMap.get(deviceId, (Message::Type)type, parameterNumber);

    if (entry) {
        int i = 1;

        if (entry->hasDestinations()) {
            lua_newtable(L);

            for (auto value : entry->getDestinations()) {
                if (value) {
                    luaLE_pushArrayObject(L, i, "ControlValue", value);
                    i++;
                }
            }
            return (1);
        }
    }

    return (luaL_error(L, "failed: empty parameterMap entry"));
}

int parameterMap_map(lua_State *L)
{
    /* Exactly the three the closure captures. Without this a call with a
     * fourth argument made that argument the parameter number, because
     * lua_pushcclosure() takes the upvalues off the top of the stack. */
    lua_settop(L, 3);

    (void)luaLE_checkDeviceId(L, 1);
    (void)luaLE_checkParameterType(L, 2);
    (void)luaLE_checkParameterNumber(L, 3);

    lua_pushcclosure(
        L,
        [](lua_State *L) -> int {
            const bool setting = lua_isnumber(L, 1);
            const lua_Integer newMidiValue = setting ? lua_tointeger(L, 1) : 0;

            /* Whatever the closure was called with goes, so that the arguments
             * handed on are only the ones built here. The two functions below
             * read their arguments by position from the bottom of the stack. */
            lua_settop(L, 0);

            lua_pushvalue(L, lua_upvalueindex(1)); // deviceId
            lua_pushvalue(L, lua_upvalueindex(2)); // type
            lua_pushvalue(L, lua_upvalueindex(3)); // parameterNumber

            if (setting) {
                lua_pushinteger(L, newMidiValue); // midiValue
                return (parameterMap_set(L));
            }

            return (parameterMap_get(L));
        },
        3);

    return (1);
}

int parameterMap_keep(lua_State *L)
{
    lua_settop(L, 0);

    parameterMap.keep();
    return (0);
}

int parameterMap_recall(lua_State *L)
{
    lua_settop(L, 0);

    parameterMap.recall();
    return (0);
}

int parameterMap_forget(lua_State *L)
{
    lua_settop(L, 0);

    parameterMap.forget();
    return (0);
}

int parameterMap_print(lua_State *L)
{
    lua_settop(L, 0);

    parameterMap.print(LOG_LUA);
    return (0);
}

void parameterMap_onChange(LookupEntry *entry, Origin origin)
{
    LuaStackGuard guard(L);

    luaLE_getModuleFunction(L, "parameterMap", "onChange");

    if (lua_isfunction(L, -1)) {
        int i = 1;

        /* The table goes on unconditionally. An entry with no destinations -
         * a parameter the script drives that no control shows - pushed only
         * two arguments and then called with three, which took the function
         * itself off the stack as one of them and called whatever was under
         * it. */
        lua_newtable(L);

        if (entry->hasDestinations()) {
            for (auto value : entry->getDestinations()) {
                if (value) {
                    luaLE_pushArrayObject(L, i, "ControlValue", value);
                    i++;
                }
            }
        }

        lua_pushnumber(L, (uint8_t)origin);
        lua_pushnumber(L, entry->getMidiValue());

        if (lua_pcall(L, 3, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function 'onChange': %s",
                                 lua_tostring(L, -1));
        }
    }
}
