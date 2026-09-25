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

#include "luaIntegration.h"

void luaLE_pushDevice(const Device &device)
{
    lua_newtable(L);
    luaLE_pushTableInteger(L, "id", device.getId());
    luaLE_pushTableInteger(L, "port", device.getPort());
    luaLE_pushTableInteger(L, "channel", device.getChannel());
}

int luaLE_checkDeviceId(lua_State *L, int idx)
{
    int deviceId = luaL_checkinteger(L, idx);

    if ((deviceId < 1) || (deviceId > 32)) {
        return (luaL_error(L, "invalid deviceId: %d", deviceId));
    }
    return (deviceId);
}

int luaLE_checkInterface(lua_State *L, int idx)
{
    int interface = luaL_checkinteger(L, idx);

    if ((interface < 0) || (interface > 2)) {
        return (luaL_error(L, "invalid interface: %d", interface));
    }
    return (interface);
}

int luaLE_checkPort(lua_State *L, int idx)
{
    int port = luaL_checkinteger(L, idx);

    if ((port < 0) || (port > 2)) {
        return (luaL_error(L, "invalid port: %d", port));
    }
    return (port);
}

int luaLE_checkChannel(lua_State *L, int idx)
{
    int channel = luaL_checkinteger(L, idx);

    if ((channel < 1) || (channel > 16)) {
        return (luaL_error(L, "invalid channel: %d", channel));
    }
    return (channel);
}

int luaLE_checkParameterType(lua_State *L, int idx)
{
    int type = luaL_checkinteger(L, idx);

    if ((type < 0) || (type > 16)) {
        return (luaL_error(L, "invalid parameter type: %d", type));
    }
    return (type);
}

int luaLE_checkParameterNumber(lua_State *L, int idx)
{
    int parameterNumber = luaL_checkinteger(L, idx);

    if ((parameterNumber < 0) || (parameterNumber > 16383)) {
        return (luaL_error(L, "invalid parameterNumber: %d", parameterNumber));
    }
    return (parameterNumber);
}

int luaLE_checkMidiValue(lua_State *L, int idx)
{
    int midiValue = luaL_checkinteger(L, idx);

    if ((midiValue < 0) || (midiValue > 16538)) {
        return (luaL_error(L, "invalid midiValue: %d", midiValue));
    }
    return (midiValue);
}

int luaLE_checkControlId(lua_State *L, int idx)
{
    int controlId = luaL_checkinteger(L, idx);

    if ((controlId < 1) || (controlId > 864)) {
        return (luaL_error(L, "invalid controlId: %d", controlId));
    }
    return (controlId);
}

int luaLE_checkDevicelId(lua_State *L, int idx)
{
    int deviceId = luaL_checkinteger(L, idx);

    if ((deviceId < 1) || (deviceId > 32)) {
        return (luaL_error(L, "invalid deviceId: %d", deviceId));
    }
    return (deviceId);
}

int luaLE_checkPageId(lua_State *L, int idx)
{
    int pageId = luaL_checkinteger(L, idx);

    if ((pageId < 1) || (pageId > 12)) {
        return (luaL_error(L, "invalid pageId: %d", pageId));
    }
    return (pageId);
}

int luaLE_checkControlSetId(lua_State *L, int idx)
{
    int controlSetId = luaL_checkinteger(L, idx);

    if ((controlSetId < 1) || (controlSetId > 3)) {
        return (luaL_error(L, "invalid controlSetId: %d", controlSetId));
    }
    return (controlSetId);
}

int luaLE_checkGroupId(lua_State *L, int idx)
{
    int groupId = luaL_checkinteger(L, idx);

    if ((groupId < 1) || (groupId > 864)) {
        return (luaL_error(L, "invalid groupId: %d", groupId));
    }
    return (groupId);
}

int luaLE_checkValueIndex(lua_State *L, int idx)
{
    int index = luaL_checkinteger(L, idx);

    return (index);
}

int luaLE_checkEvents(lua_State *L, int idx)
{
    int events = luaL_checkinteger(L, idx);

    if ((events < 0) || (events > 127)) {
        return (luaL_error(L, "invalid events: %d", events));
    }
    return (events);
}

int luaLE_checkListId(lua_State *L, int idx)
{
    int listId = luaL_checkinteger(L, idx);

    if ((listId < 1) || (listId > 255)) {
        return (luaL_error(L, "invalid listId: %d", listId));
    }
    return (listId);
}