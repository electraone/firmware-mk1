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
#include "luaControl.h"
#include "Presets.h"
#include "MainDelegate.h"
#include "luaExtension.h"
#include "ParameterMap.h"

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

/*
 * setSensitivity(factor) - how far a knob turns to move the value, as a factor
 * on the usual rate: 0.1 is a tenth of it, so ten sweeps of the knob end to
 * end. Written straight to the value rather than through the delegate - nothing
 * on the screen depends on it, and every turn reads it afresh, so the next one
 * already moves at the new rate.
 */
int value_setSensitivity(lua_State *L)
{
    lua_settop(L, 2);

    ControlValue *value = getControlValue(L, 1);
    const lua_Number factor = luaL_checknumber(L, 2);

    luaL_argcheck(L,
                  (factor >= ControlValue::MinSensitivity)
                      && (factor <= ControlValue::MaxSensitivity),
                  2,
                  "sensitivity out of range");

    if (value) {
        value->setSensitivity(static_cast<float>(factor));
    } else {
        return (luaL_error(L, "failed: not a valid value"));
    }
    return (0);
}

int value_getSensitivity(lua_State *L)
{
    lua_settop(L, 1);

    if (ControlValue *value = getControlValue(L, 1)) {
        lua_pushnumber(L, value->getSensitivity());
        return (1);
    }
    return (luaL_error(L, "failed: not a valid value"));
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
            /* The cached instance, not a fresh object: the fields a script
             * set on the control are kept on it, so value:getControl().x
             * has to find the x the script put there. */
            luaLE_pushControlInstance(L, *control);
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

/** @brief The display value as the MIDI value it is sent as. */
namespace
{
    /* The MIDI value the parameter map holds for the value's message, or
     * MIDI_VALUE_DO_NOT_SEND when nothing has set it yet. */
    uint16_t currentMidiValue(const ControlValue &value)
    {
        const Message &message = value.message;

        return (parameterMap.getValue(message.getDeviceId(),
                                      message.getType(),
                                      message.getParameterNumber()));
    }
} // namespace

/**
 * @brief value:isSet() - whether the value has been given one yet.
 *
 * A value is unset until MIDI, a knob, a preset's default or a script gives it
 * one. The mk2's call, and what a script reading the value checks first.
 */
int value_isSet(lua_State *L)
{
    lua_settop(L, 1);

    if (ControlValue *value = getControlValue(L, 1)) {
        lua_pushboolean(L, currentMidiValue(*value) != MIDI_VALUE_DO_NOT_SEND);
        return (1);
    }
    return (luaL_error(L, "failed: not a valid value"));
}

/**
 * @brief value:getValue() - the value as the control displays it.
 *
 * The default while the value is unset, which is what the control shows then.
 */
int value_getValue(lua_State *L)
{
    lua_settop(L, 1);

    if (ControlValue *value = getControlValue(L, 1)) {
        const uint16_t midiValue = currentMidiValue(*value);

        lua_pushinteger(L,
                        (midiValue == MIDI_VALUE_DO_NOT_SEND)
                            ? value->getDefault()
                            : value->translateMidiValue(midiValue));
        return (1);
    }
    return (luaL_error(L, "failed: not a valid value"));
}

/**
 * @brief value:setValue(displayValue) - sets it as a knob would.
 *
 * Held to the display range, sent as MIDI and drawn, the way message:setValue
 * does it for a MIDI value; this takes the number the control shows.
 */
int value_setValue(lua_State *L)
{
    lua_settop(L, 2);

    ControlValue *value = getControlValue(L, 1);
    const lua_Integer displayValue = luaL_checkinteger(L, 2);

    if (value == nullptr) {
        return (luaL_error(L, "failed: not a valid value"));
    }

    /* Held before it is narrowed, for the reason value_toMidi() gives. */
    const lua_Integer held = (displayValue < value->getMin())   ? value->getMin()
                             : (displayValue > value->getMax()) ? value->getMax()
                                                                : displayValue;
    const Message &message = value->message;

    parameterMap.setValue(message.getDeviceId(),
                          message.getType(),
                          message.getParameterNumber(),
                          value->translateValue((int16_t)held),
                          Origin::lua);
    return (0);
}

int value_toMidi(lua_State *L)
{
    lua_settop(L, 2);

    ControlValue *value = getControlValue(L, 1);
    const lua_Integer displayValue = luaL_checkinteger(L, 2);

    if (value == nullptr) {
        return (luaL_error(L, "failed: not a valid value"));
    }

    /* Held to the display range here, before it is narrowed to the int16_t the
     * translation takes. Narrowed first, 99999 wrapped to a negative number and
     * came back as the minimum instead of the maximum. */
    const lua_Integer held = (displayValue < value->getMin())   ? value->getMin()
                             : (displayValue > value->getMax()) ? value->getMax()
                                                                : displayValue;

    lua_pushinteger(L, value->translateValue((int16_t)held));

    return (1);
}

/** @brief The MIDI value as a display value, the inverse of toMidi(). */
int value_fromMidi(lua_State *L)
{
    lua_settop(L, 2);

    ControlValue *value = getControlValue(L, 1);
    const lua_Integer midiValue = luaL_checkinteger(L, 2);

    if (value == nullptr) {
        return (luaL_error(L, "failed: not a valid value"));
    }

    /* A MIDI value is never negative and never wider than 14 bits; outside
     * that it would wrap on the way into the uint16_t the translation takes. */
    const lua_Integer held = (midiValue < 0)       ? 0
                             : (midiValue > 16383) ? 16383
                                                   : midiValue;

    lua_pushinteger(L, value->translateMidiValue((uint16_t)held));

    return (1);
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
