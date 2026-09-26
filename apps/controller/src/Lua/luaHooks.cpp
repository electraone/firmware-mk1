#include "luaHooks.h"
#include "luaCallbacks.h"
#include "luaControl.h"
#include "Control.h"
#include "System.h"
#include "Trace.h"

void runFormatter(const char *formatter,
                  const void *object,
                  int16_t value,
                  char *buffer,
                  int maxLength)
{
    int stackSize = lua_gettop(L);
    lua_getglobal(L, formatter);

    if (lua_isfunction(L, -1)) {
        TRACE_LUA_ENTER_VALUE("formatter", formatter, value);
        luaLE_pushObject(L, "ControlValue", object);
        lua_pushnumber(L, value);

        if (lua_pcall(L, 2, LUA_MULTRET, 0) != 0) {
            TRACE_LUA_ERROR("formatter", formatter, lua_tostring(L, -1));
            System::logger.write(LOG_LUA,
                                 "error running function 'runFormatter': %s",
                                 lua_tostring(L, -1));
        }

        // Check if the lua function returned at least one value
        int numReturnedValues = lua_gettop(L) - stackSize;

        if (numReturnedValues > 0) {
            const char *formattedValue = luaL_checkstring(L, -1);
            if (strlen(formattedValue) > 0) {
                copyString(buffer, luaL_checkstring(L, -1), maxLength - 1);
            }
            TRACE_LUA_RETURN_STR("formatter", formatter, formattedValue);
            lua_pop(L, 1);
        } else {
            TRACE_LUA_RETURN_STR("formatter", formatter, "<none>");
            System::logger.write(
                LOG_LUA, "function 'runFormatter' does not return value");
        }
    } else {
        TRACE_LUA_MISSING("formatter", formatter);
        // Remove entry inserted with the lua_getglobal
        lua_pop(L, 1);
    }

    luaLE_postFunctionCleanUp(L);
}

void runFunction(const char *function, const void *object, int16_t value)
{
    lua_getglobal(L, function);

    if (lua_isfunction(L, -1)) {
        TRACE_LUA_ENTER_VALUE("function", function, value);
        luaLE_pushObject(L, "ControlValue", object);
        lua_pushnumber(L, value);

        if (lua_pcall(L, 2, 0, 0) != 0) {
            TRACE_LUA_ERROR("function", function, lua_tostring(L, -1));
            System::logger.write(LOG_LUA,
                                 "error running function 'runFunction': %s",
                                 lua_tostring(L, -1));
        }
        TRACE_LUA_RETURN_VOID("function", function);
    } else {
        TRACE_LUA_MISSING("function", function);
        // Remove entry inserted with the lua_getglobal
        lua_pop(L, 1);
    }

    luaLE_postFunctionCleanUp(L);
}

uint8_t
    runTemplateFunction(const char *function, const void *object, int16_t value)
{
    int stackSize = lua_gettop(L);
    uint8_t dataOut = 0;

    lua_getglobal(L, function);

    if (lua_isfunction(L, -1)) {
        TRACE_LUA_ENTER_VALUE("template", function, value);
        luaLE_pushObject(L, "Device", object);
        lua_pushnumber(L, value);

        if (lua_pcall(L, 2, LUA_MULTRET, 0) != 0) {
            TRACE_LUA_ERROR("template", function, lua_tostring(L, -1));
            System::logger.write(
                LOG_LUA,
                "error running function 'runTemplateFunction': %s",
                lua_tostring(L, -1));
        }

        // Check if the lua function returned at least one value
        int numReturnedValues = lua_gettop(L) - stackSize;

        if (numReturnedValues > 0) {
            dataOut = luaL_checkinteger(L, -1);
            lua_pop(L, 1);
        } else {
            System::logger.write(
                LOG_ERROR,
                "function 'runTemplateFunction' does not return value");
        }
        TRACE_LUA_RETURN_NUM("template", function, dataOut);
    } else {
        TRACE_LUA_MISSING("template", function);
        // Remove entry inserted with the lua_getglobal
        lua_pop(L, 1);
    }

    luaLE_postFunctionCleanUp(L);

    return (dataOut);
}

/* -------------------------------------------------------------------------
 * The callbacks a Custom control runs
 *
 * Each one is a function the script handed to the control, kept in the
 * registry. The control's own Lua object is the first argument, so a script
 * reads the fields it set on the control from inside the callback; the rest
 * is a table describing what happened.
 * ------------------------------------------------------------------------- */

namespace
{
    /* The function, and the control it belongs to, on the stack. Answers
     * false and leaves the stack as it found it when there is no callback. */
    bool pushCallback(const Control &control, int ref)
    {
        if (!L || (ref == LUA_REFNIL)) {
            return (false);
        }

        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            return (false);
        }

        /* The instance, not a bare object: Control's __index reads the
         * uservalue table first, and a userdata without one takes the callback
         * down at its first method call. */
        luaLE_pushControlInstance(L, control);
        return (true);
    }

    void call(const char *name, int arguments)
    {
        if (lua_pcall(L, arguments, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 name,
                                 lua_tostring(L, -1));
        }

        luaLE_postFunctionCleanUp(L);
    }

    /* The valueId of the input that knob arrives on, which is what tells one
     * knob of a twelve-knob Custom control from another. */
    const char *valueIdForPot(const Control &control, uint8_t potId)
    {
        for (const auto &input : control.inputs) {
            if (input.getPotId() == potId) {
                return (control.translateValueId(input.getValueId()));
            }
        }
        return ("value");
    }

    void runKnobCallback(const Control &control,
                         int ref,
                         const char *name,
                         LuaPotEvent event,
                         uint8_t potId,
                         int16_t delta)
    {
        if (!pushCallback(control, ref)) {
            return;
        }

        lua_newtable(L);
        luaLE_pushTableInteger(L, "type", (int)event);
        /* Zero based, as the mk2's is: scripts written against the release
         * that shipped it index from zero. */
        luaLE_pushTableInteger(L, "id", potId);
        luaLE_pushTableString(L, "valueId", valueIdForPot(control, potId));
        luaLE_pushTableInteger(L, "delta", delta);

        call(name, 2);
    }
} // namespace

void runPaintCallback(const Control &control)
{
    const ControlCallbacks *entry = findControlCallbacks(control.getId());

    if (!entry || !pushCallback(control, entry->paint)) {
        return;
    }

    call("runPaintCallback", 1);
}

void runTouchCallback(const Control &control, const TouchEvent &touchEvent)
{
    const ControlCallbacks *entry = findControlCallbacks(control.getId());

    if (!entry || !pushCallback(control, entry->touch)) {
        return;
    }

    lua_newtable(L);
    luaLE_pushTableInteger(L, "type", (int)touchEvent.getEvent());
    luaLE_pushTableInteger(L, "id", touchEvent.getId());
    luaLE_pushTableInteger(L, "x", touchEvent.getX());
    luaLE_pushTableInteger(L, "y", touchEvent.getY());
    luaLE_pushTableInteger(L, "touchDownX", touchEvent.getTouchDownX());
    luaLE_pushTableInteger(L, "touchDownY", touchEvent.getTouchDownY());

    call("runTouchCallback", 2);
}

void runPotCallback(const Control &control,
                    const PotEvent &potEvent,
                    LuaPotEvent event)
{
    const ControlCallbacks *entry = findControlCallbacks(control.getId());

    if (!entry) {
        return;
    }

    runKnobCallback(
        control,
        entry->pot,
        "runPotCallback",
        event,
        potEvent.getPotId(),
        (event == LuaPotEvent::change) ? potEvent.getAcceleratedChange() : 0);
}

void runPotTouchCallback(const Control &control,
                         const PotEvent &potEvent,
                         LuaPotEvent event)
{
    const ControlCallbacks *entry = findControlCallbacks(control.getId());

    if (!entry) {
        return;
    }

    runKnobCallback(control,
                    entry->potTouch,
                    "runPotTouchCallback",
                    event,
                    potEvent.getPotId(),
                    0);
}

void runSwitchCallback(const Control &control, uint8_t potId, bool pressed)
{
    const ControlCallbacks *entry = findControlCallbacks(control.getId());

    if (!entry || !pushCallback(control, entry->switchPress)) {
        return;
    }

    lua_pushinteger(L, potId);
    lua_pushboolean(L, pressed);

    call("runSwitchCallback", 3);
}
