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

#include "luaMessage.h"
#include "Message.h"
#include "ParameterMap.h"

static Message *getMessage(lua_State *L, uint8_t stackPosition)
{
    return (*reinterpret_cast<Message **>(
        luaL_checkudata(L, stackPosition, "Message")));
}

int message_create(lua_State *L)
{
    lua_settop(L, 1);
    int index = luaLE_checkValueIndex(L, 1);

    Message *message = nullptr; // on purpose. there is no getter

    if (message) {
        luaLE_pushObject(L, "Message", message);
        return (1);
    }
    return (luaL_error(L, "failed: message %d does not exist", index));
}

int message_delete(lua_State *L)
{
    lua_settop(L, 1);
    return (0);
}

int message_setType(lua_State *L)
{
    lua_settop(L, 2);

    Message *message = getMessage(L, 1);
    int messageType = luaLE_checkParameterType(L, 2);

    if (message) {
        parameterMap.removeDestination(message);
        message->setType((Message::Type)messageType);
        parameterMap.addDestination(message);
    }
    return (0);
}

int message_getType(lua_State *L)
{
    lua_settop(L, 1);
    int messageType = (int)Message::Type::none;

    if (Message *message = getMessage(L, 1)) {
        messageType = (int)message->getType();
    }

    lua_pushinteger(L, messageType);
    return (1);
}

int message_setParameterNumber(lua_State *L)
{
    lua_settop(L, 2);

    Message *message = getMessage(L, 1);
    int parameterNumber = luaLE_checkParameterNumber(L, 2);

    if (message) {
        parameterMap.removeDestination(message);
        message->setParameterNumber(parameterNumber);
        parameterMap.addDestination(message);
    }
    return (0);
}

int message_getParameterNumber(lua_State *L)
{
    lua_settop(L, 1);
    int parameterNumber = 0;

    if (Message *message = getMessage(L, 1)) {
        parameterNumber = message->getParameterNumber();
    }

    lua_pushinteger(L, parameterNumber);
    return (1);
}

int message_setDeviceId(lua_State *L)
{
    lua_settop(L, 2);

    Message *message = getMessage(L, 1);
    int deviceId = luaLE_checkDeviceId(L, 2);

    if (message) {
        parameterMap.removeDestination(message);
        message->setDeviceId(deviceId);
        parameterMap.addDestination(message);
    }

    return (0);
}

int message_getDeviceId(lua_State *L)
{
    lua_settop(L, 1);
    int deviceId = 0;

    if (Message *message = getMessage(L, 1)) {
        deviceId = message->getDeviceId();
    }

    lua_pushinteger(L, deviceId);
    return (1);
}

int message_setValue(lua_State *L)
{
    lua_settop(L, 2);

    Message *message = getMessage(L, 1);
    int midiValue = luaL_checkinteger(L, 2);

    if (message) {
        message->setValue(midiValue);
        parameterMap.setValue(message->getDeviceId(),
                              message->getType(),
                              message->getParameterNumber(),
                              midiValue,
                              Origin::lua);
    }
    return (0);
}

int message_getValue(lua_State *L)
{
    lua_settop(L, 1);
    int midiValue = 0;

    if (Message *message = getMessage(L, 1)) {
        midiValue = parameterMap.getValue(message->getDeviceId(),
                                          message->getType(),
                                          message->getParameterNumber());
    }

    lua_pushinteger(L, midiValue);
    return (1);
}

int message_setOnValue(lua_State *L)
{
    lua_settop(L, 2);

    Message *message = getMessage(L, 1);
    int onValue = luaLE_checkMidiValue(L, 2);

    if (message) {
        message->setOnValue(onValue);
    }
    return (0);
}

int message_getOnValue(lua_State *L)
{
    lua_settop(L, 1);
    int midiValue = 0;

    if (Message *message = getMessage(L, 1)) {
        midiValue = message->getOnValue();
    }

    lua_pushinteger(L, midiValue);
    return (1);
}

int message_setOffValue(lua_State *L)
{
    lua_settop(L, 2);

    Message *message = getMessage(L, 1);
    int offValue = luaLE_checkMidiValue(L, 2);

    if (message) {
        message->setOffValue(offValue);
    }
    return (0);
}

int message_getOffValue(lua_State *L)
{
    lua_settop(L, 1);
    int midiValue = 0;

    if (Message *message = getMessage(L, 1)) {
        midiValue = message->getOffValue();
    }

    lua_pushinteger(L, midiValue);
    return (1);
}

int message_setMin(lua_State *L)
{
    lua_settop(L, 2);

    Message *message = getMessage(L, 1);
    int minValue = luaLE_checkMidiValue(L, 2);

    if (message) {
        message->setMidiMin(minValue);
    }
    return (0);
}

int message_getMin(lua_State *L)
{
    lua_settop(L, 1);
    int minValue = 0;

    if (Message *message = getMessage(L, 1)) {
        minValue = message->getMidiMin();
    }

    lua_pushinteger(L, minValue);
    return (1);
}

int message_setMax(lua_State *L)
{
    lua_settop(L, 2);

    Message *message = getMessage(L, 1);
    int maxValue = luaLE_checkMidiValue(L, 2);
    if (message) {
        message->setMidiMax(maxValue);
    }
    return (0);
}

int message_getMax(lua_State *L)
{
    lua_settop(L, 1);
    int maxValue = 0;

    if (Message *message = getMessage(L, 1)) {
        maxValue = message->getMidiMax();
    }

    lua_pushinteger(L, maxValue);
    return (1);
}

int message_setRange(lua_State *L)
{
    lua_settop(L, 3);

    Message *message = getMessage(L, 1);
    int minValue = luaLE_checkMidiValue(L, 2);
    int maxValue = luaLE_checkMidiValue(L, 3);

    if (message) {
        message->setMidiMin(minValue);
        message->setMidiMax(maxValue);
    }
    return (0);
}

int message_print(lua_State *L)
{
    lua_settop(L, 1);

    if (Message *message = getMessage(L, 1)) {
        message->print(LOG_LUA);
    }

    return (0);
}

void message_register(lua_State *L)
{
    lua_register(L, "Message", message_create);
    luaL_newmetatable(L, "Message");

    lua_pushcfunction(L, message_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaLE_addObjectMethods(L, message_functions);
    lua_pop(L, 1);
}
