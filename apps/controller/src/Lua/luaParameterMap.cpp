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
    parameterMap.clear();
    return (0);
}

int parameterMap_resetDevice(lua_State *L)
{
    lua_settop(L, 1);
    int deviceId = luaLE_checkDeviceId(L, -1);
    parameterMap.resetDeviceValues(deviceId);
    return (0);
}

int parameterMap_set(lua_State *L)
{
    lua_settop(L, 4);
    int deviceId = luaLE_checkDeviceId(L, -4);
    int type = luaLE_checkParameterType(L, -3);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, -2);
    uint16_t midiValue = luaLE_checkMidiValue(L, -1);

    parameterMap.setValue(
        deviceId, (Message::Type)type, parameterNumber, midiValue, Origin::lua);
    return (0);
}

int parameterMap_apply(lua_State *L)
{
    lua_settop(L, 4);

    int deviceId = luaLE_checkDeviceId(L, -4);
    int type = luaLE_checkParameterType(L, -3);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, -2);
    uint16_t midiValue = luaLE_checkMidiValue(L, -1);

    parameterMap.applyToValue(
        deviceId, (Message::Type)type, parameterNumber, midiValue, Origin::lua);
    return (0);
}

int parameterMap_modulate(lua_State *L)
{
    lua_settop(L, 5);

    int deviceId = luaLE_checkDeviceId(L, -5);
    int type = luaLE_checkParameterType(L, -4);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, -3);
    float modulationValue = luaL_checknumber(L, -2);
    int depth = luaL_checkinteger(L, -1);

    parameterMap.modulateValue(
        deviceId, (Message::Type)type, parameterNumber, modulationValue, depth);

    return (0);
}

int parameterMap_send(lua_State *L)
{
    lua_settop(L, 3);
    int deviceId = luaLE_checkDeviceId(L, -3);
    int type = luaLE_checkParameterType(L, -2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, -1);

    System::logger.write(LOG_LUA, "parameterMap_send is deprecated");
    return (0);
}

int parameterMap_get(lua_State *L)
{
    lua_settop(L, 3);

    int deviceId = luaLE_checkDeviceId(L, -3);
    int type = luaLE_checkParameterType(L, -2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, -1);

    uint16_t midiValue =
        parameterMap.getValue(deviceId, (Message::Type)type, parameterNumber);

    lua_pushnumber(L, midiValue);
    return (1);
}

int parameterMap_getValues(lua_State *L)
{
    lua_settop(L, 3);

    int deviceId = luaLE_checkDeviceId(L, -3);
    int type = luaLE_checkParameterType(L, -2);
    uint16_t parameterNumber = luaLE_checkParameterNumber(L, -1);

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
    lua_pushcclosure(
        L,
        [](lua_State *L) -> int {
            if (lua_isnumber(L, 1)) {
                int newMidiValue = lua_tointeger(L, 1);
                lua_remove(L, 1);

                lua_pushvalue(L, lua_upvalueindex(1)); // deviceId
                lua_pushvalue(L, lua_upvalueindex(2)); // type
                lua_pushvalue(L, lua_upvalueindex(3)); // parameterNumber
                lua_pushinteger(L, newMidiValue); // midiValue

                return (parameterMap_set(L));
            } else {
                lua_pushvalue(L, lua_upvalueindex(1)); // deviceId
                lua_pushvalue(L, lua_upvalueindex(2)); // type
                lua_pushvalue(L, lua_upvalueindex(3)); // parameterNumber

                return (parameterMap_get(L));
            }
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
    luaLE_getModuleFunction(L, "parameterMap", "onChange");

    if (lua_isfunction(L, -1)) {
        int i = 1;

        if (entry->hasDestinations()) {
            lua_newtable(L);

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
    } else {
        lua_pop(L, 1);
    }
}
