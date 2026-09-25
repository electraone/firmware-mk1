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

#include "luaValue.h"
#include "Presets.h"
#include "MainDelegate.h"
#include "luaExtension.h"

static ControlValue *getControlValue(lua_State *L, uint8_t stackPosition)
{
    return (*reinterpret_cast<ControlValue **>(
        luaL_checkudata(L, stackPosition, "ControlValue")));
}

int value_create(lua_State *L)
{
    lua_settop(L, 1);
    int index = luaLE_checkValueIndex(L, 1);

    ControlValue *value = nullptr; // on purpose. there is no getter

    if (value) {
        luaLE_pushObject(L, "ControlValue", value);
        return (1);
    }
    return (luaL_error(L, "failed: value %d does not exist", index));
}

int value_delete(lua_State *L)
{
    lua_settop(L, 1);
    return (0);
}

int value_getId(lua_State *L)
{
    lua_settop(L, 1);
    const char *valueId = "value";

    if (ControlValue *value = getControlValue(L, 1)) {
        valueId = value->getId();
    }

    lua_pushstring(L, valueId);
    return (1);
}

int value_setDefault(lua_State *L)
{
    lua_settop(L, 2);

    ControlValue *value = getControlValue(L, 1);
    int valueDefault = luaL_checkinteger(L, 2);

    if (value) {
        value->setDefault(valueDefault);
    }
    return (0);
}

int value_getDefault(lua_State *L)
{
    lua_settop(L, 1);
    int valueDefault = 0;

    if (ControlValue *value = getControlValue(L, 1)) {
        valueDefault = value->getDefault();
    }

    lua_pushinteger(L, valueDefault);
    return (1);
}

int value_setMin(lua_State *L)
{
    lua_settop(L, 2);

    ControlValue *value = getControlValue(L, 1);
    int valueMin = luaL_checkinteger(L, 2);

    if (value) {
        Control *control = value->getControl();
        if (control) {
            luaDelegate->setControlValueMin(
                control->getId(), value->getHandle(), valueMin);
        }
    } else {
        return (luaL_error(L, "failed: not a valid value"));
    }
    return (0);
}

int value_getMin(lua_State *L)
{
    lua_settop(L, 1);
    int valueMin = 0;

    if (ControlValue *value = getControlValue(L, 1)) {
        valueMin = value->getMin();
    }

    lua_pushinteger(L, valueMin);
    return (1);
}

int value_setMax(lua_State *L)
{
    lua_settop(L, 2);

    ControlValue *value = getControlValue(L, 1);
    int valueMax = luaL_checkinteger(L, 2);

    if (value) {
        Control *control = value->getControl();
        if (control) {
            luaDelegate->setControlValueMax(
                control->getId(), value->getHandle(), valueMax);
        }
    } else {
        return (luaL_error(L, "failed: not a valid value"));
    }
    return (0);
}

int value_getMax(lua_State *L)
{
    lua_settop(L, 1);
    int valueMax = 0;

    if (ControlValue *value = getControlValue(L, 1)) {
        valueMax = value->getMax();
    }

    lua_pushinteger(L, valueMax);
    return (1);
}

int value_setRange(lua_State *L)
{
    lua_settop(L, 5);

    ControlValue *value = getControlValue(L, 1);
    int valueMin = luaL_checkinteger(L, 2);
    int valueMax = luaL_checkinteger(L, 3);
    int valueDefault = luaL_checkinteger(L, 4);
    bool updateMessage = luaLE_checkBoolean(L, 5);

    if (value) {
        Control *control = value->getControl();
        if (control) {
            luaDelegate->setControlValueRange(control->getId(),
                                              value->getHandle(),
                                              valueMin,
                                              valueMax,
                                              valueDefault,
                                              updateMessage);
        }
    } else {
        return (luaL_error(L, "failed: not a valid value"));
    }
    return (0);
}

int value_setOverlayId(lua_State *L)
{
    lua_settop(L, 2);

    ControlValue *value = getControlValue(L, 1);
    int overlayId = luaL_checkinteger(L, 2);

    if (value) {
        Control *control = value->getControl();
        if (control) {
            luaDelegate->setControlValueOverlay(
                control->getId(), value->getHandle(), overlayId);
        }
    } else {
        return (luaL_error(L, "failed: not a valid value"));
    }
    return (0);
}

int value_getOverlayId(lua_State *L)
{
    lua_settop(L, 1);
    int overlayId = 0;

    if (ControlValue *value = getControlValue(L, 1)) {
        overlayId = value->getOverlayId();
    }

    lua_pushinteger(L, overlayId);
    return (1);
}

int value_getMessage(lua_State *L)
{
    lua_settop(L, 1);

    if (ControlValue *value = getControlValue(L, 1)) {
        Message *message = &(value->message);

        if (message) {
            luaLE_pushObject(L, "Message", message);
            return (1);
        }
    }
    return (luaL_error(L, "failed: not a valid value"));
}

int value_getControl(lua_State *L)
{
    lua_settop(L, 1);

    if (ControlValue *value = getControlValue(L, 1)) {
        Control *control = value->getControl();

        if (control) {
            luaLE_pushObject(L, "Control", control);
            return (1);
        }
    }
    return (luaL_error(L, "failed: not a valid value"));
}

int value_overrideValue(lua_State *L)
{
    lua_settop(L, 2);
    ControlValue *value = getControlValue(L, 1);
    const char *textValue = luaL_checkstring(L, 2);

    if (value) {
        Control *control = value->getControl();
        if (control) {
            luaDelegate->setControlValueLabel(
                control->getId(), value->getHandle(), textValue);
        }
    } else {
        return (luaL_error(L, "failed: not a valid value"));
    }
    return (0);
}

int value_cancelOverride(lua_State *L)
{
    lua_settop(L, 1);
    ControlValue *value = getControlValue(L, 1);

    if (value) {
        Control *control = value->getControl();
        if (control) {
            luaDelegate->setControlValueLabel(
                control->getId(), value->getHandle(), "");
        }
    } else {
        return (luaL_error(L, "failed: not a valid value"));
    }
    return (0);
}

int value_print(lua_State *L)
{
    lua_settop(L, 1);

    if (ControlValue *value = getControlValue(L, 1)) {
        value->print(LOG_LUA);
    }

    return (0);
}

void value_register(lua_State *L)
{
    lua_register(L, "ControlValue", value_create);
    luaL_newmetatable(L, "ControlValue");

    lua_pushcfunction(L, value_delete);
    lua_setfield(L, -2, "__gc");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaLE_addObjectMethods(L, value_functions);
    lua_pop(L, 1);
}
