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

#include "luaDevice.h"
#include "Preset.h"

extern Preset *luaPreset;

static const luaL_Reg devices_functions[] = { { "get", device_create },
                                              { "create", device_createNew },
                                              { "getByPortChannel",
                                                device_getByPortChannel },
                                              { NULL, NULL } };

static Device *getDevice(lua_State *L, uint8_t stackPosition)
{
    return (*reinterpret_cast<Device **>(
        luaL_checkudata(L, stackPosition, "Device")));
}

int luaopen_devices(lua_State *L)
{
    luaL_newlib(L, devices_functions);
    return 1;
}

int device_create(lua_State *L)
{
    lua_settop(L, 1);
    int deviceId = luaLE_checkDeviceId(L, 1);

    const Device &device = luaPreset->getDevice(deviceId);

    if (device.isValid()) {
        luaLE_pushObject(L, "Device", &device);
        return (1);
    }
    return (luaL_error(L, "failed: device %d does not exist", deviceId));
}

int device_createNew(lua_State *L)
{
    lua_settop(L, 4);
    int deviceId = luaLE_checkDeviceId(L, 1);
    const char *name = luaL_checkstring(L, 2);
    int port = luaLE_checkPort(L, 3);
    int channel = luaLE_checkChannel(L, 4);

    const Device &device = luaPreset->addDevice(deviceId, name, port, channel);

    if (device.isValid()) {
        luaLE_pushObject(L, "Device", &device);
        return (1);
    }
    return (luaL_error(L, "failed: device %d does not exist", deviceId));
}

int device_getByPortChannel(lua_State *L)
{
    lua_settop(L, 2);

    int port = luaLE_checkPort(L, 1);
    int channel = luaLE_checkChannel(L, 2);

    const Device &device = luaPreset->getDevice(port, channel);

    if (device.isValid()) {
        luaLE_pushObject(L, "Device", &device);
        return (1);
    }
    return (luaL_error(L,
                       "failed: device does not exist: port=%d, channel=%d",
                       port,
                       channel));
}

int device_delete(lua_State *L)
{
    lua_settop(L, 1);
    return (0);
}

int device_getId(lua_State *L)
{
    lua_settop(L, 1);
    int id = 0;

    if (Device *device = getDevice(L, 1)) {
        id = device->getId();
    }

    lua_pushinteger(L, id);
    return (1);
}

int device_setName(lua_State *L)
{
    lua_settop(L, 2);

    Device *device = getDevice(L, 1);
    const char *name = luaL_checkstring(L, 2);

    luaL_argcheck(L, name != nullptr, 2, "failed: name must not be nil");

    if (device) {
        device->setName(name);
    } else {
        return (luaL_error(L, "failed: not a valid device"));
    }
    return (0);
}

int device_getName(lua_State *L)
{
    lua_settop(L, 1);

    if (Device *device = getDevice(L, 1)) {
        const char *name = device->getName();
        lua_pushstring(L, name);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid device"));
}

int device_setRate(lua_State *L)
{
    lua_settop(L, 2);

    Device *device = getDevice(L, 1);
    int rate = luaL_checkinteger(L, 2);

    luaL_argcheck(L,
                  10 <= rate && rate <= 1000,
                  2,
                  "failed: rate must be between 10 and 1000");

    if (device) {
        device->setRate(rate);
    } else {
        return (luaL_error(L, "failed: not a valid device"));
    }

    return (0);
}

int device_getRate(lua_State *L)
{
    lua_settop(L, 1);
    int rate = 0;

    if (Device *device = getDevice(L, 1)) {
        rate = device->getRate();
    }

    lua_pushinteger(L, rate);
    return (1);
}

int device_setPort(lua_State *L)
{
    lua_settop(L, 2);

    Device *device = getDevice(L, 1);
    int port = luaL_checkinteger(L, 2);

    luaL_argcheck(
        L, 0 <= port && port <= 2, 2, "failed: port must be between 0 and 2");

    if (device) {
        device->setPort(port);
    } else {
        return (luaL_error(L, "failed: not a valid device"));
    }

    return (0);
}

int device_getPort(lua_State *L)
{
    lua_settop(L, 1);
    int port = 0;

    if (Device *device = getDevice(L, 1)) {
        port = device->getPort();
    }

    lua_pushinteger(L, port);
    return (1);
}

int device_setChannel(lua_State *L)
{
    lua_settop(L, 2);

    Device *device = getDevice(L, 1);
    int channel = luaL_checkinteger(L, 2);

    luaL_argcheck(L,
                  1 <= channel && channel <= 16,
                  2,
                  "failed: channel must be between 1 and 16");

    if (device) {
        device->setChannel(channel);
    } else {
        return (luaL_error(L, "failed: not a valid device"));
    }

    return (0);
}

int device_getChannel(lua_State *L)
{
    lua_settop(L, 1);
    int channel = 0;

    if (Device *device = getDevice(L, 1)) {
        channel = device->getChannel();
    }

    lua_pushinteger(L, channel);
    return (1);
}

int device_print(lua_State *L)
{
    lua_settop(L, 1);

    if (Device *device = getDevice(L, 1)) {
        device->print(LOG_LUA);
    }

    return (0);
}

void device_register(lua_State *L)
{
    lua_register(L, "Device", device_create);
    luaL_newmetatable(L, "Device");

    lua_pushcfunction(L, device_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaLE_addObjectMethods(L, device_functions);

    lua_pop(L, 1);
}
