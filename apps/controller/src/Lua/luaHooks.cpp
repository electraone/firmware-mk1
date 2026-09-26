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
            /* A failed call leaves its error message where a returned value
             * would be. Counting it as one used to put the error text on the
             * control, in place of the value, and then hand it to a checked
             * conversion that raises from outside any protected call. */
            TRACE_LUA_ERROR("formatter", formatter, lua_tostring(L, -1));
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 formatter,
                                 lua_tostring(L, -1));
            lua_pop(L, 1);
            luaLE_postFunctionCleanUp(L);
            return;
        }

        // Check if the lua function returned at least one value
        int numReturnedValues = lua_gettop(L) - stackSize;

        if (numReturnedValues > 0) {
            /* lua_tostring(), not luaL_checkstring(): a formatter returning a
             * table or a boolean is a mistake in the preset, not a reason to
             * raise where there is nothing to catch it. Numbers and strings are
             * both accepted, which is what a formatter tends to return. */
            const char *formattedValue = lua_tostring(L, -1);

            if (formattedValue == nullptr) {
                System::logger.write(
                    LOG_LUA,
                    "function '%s' returned a %s, which is not a value",
                    formatter,
                    luaL_typename(L, -1));
            } else if (strlen(formattedValue) > 0) {
                copyString(buffer, formattedValue, maxLength - 1);
            }

            TRACE_LUA_RETURN_STR("formatter",
                                 formatter,
                                 (formattedValue != nullptr) ? formattedValue
                                                             : "<invalid>");
        } else {
            TRACE_LUA_RETURN_STR("formatter", formatter, "<none>");
            System::logger.write(
                LOG_LUA, "function '%s' does not return a value", formatter);
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
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
            lua_pop(L, 1);
            luaLE_postFunctionCleanUp(L);
            return (0);
        }

        // Check if the lua function returned at least one value
        int numReturnedValues = lua_gettop(L) - stackSize;

        if (numReturnedValues > 0) {
            /* Converted rather than checked, for the same reason as the
             * formatter above: a template function returning something that is
             * not a number must not raise from here, where there is no
             * protected call to catch it. */
            int isNumber = 0;
            const lua_Integer returned = lua_tointegerx(L, -1, &isNumber);

            if (isNumber != 0) {
                dataOut = (uint8_t)returned;
            } else {
                System::logger.write(
                    LOG_LUA,
                    "function '%s' returned a %s, which is not a byte",
                    function,
                    luaL_typename(L, -1));
            }
        } else {
            System::logger.write(
                LOG_LUA, "function '%s' does not return a value", function);
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

/*
 * Both of these walk preset.userFunctions and leave the stack as they found
 * it. The entry is named after the knob that offers it - pot1 to pot12 - which
 * is the mk2's spelling, so a preset written for one runs on the other.
 */
namespace
{
    /** Pushes preset.userFunctions.pot<n>, or nothing. */
    bool pushUserFunction(uint8_t functionId)
    {
        if ((L == nullptr) || (functionId >= numUserFunctions)) {
            return (false);
        }

        lua_getglobal(L, "preset");

        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return (false);
        }

        lua_getfield(L, -1, "userFunctions");

        if (!lua_istable(L, -1)) {
            lua_pop(L, 2);
            return (false);
        }

        /* "pot" and two digits. */
        char potKey[8];

        snprintf(potKey, sizeof(potKey), "pot%d", functionId + 1);
        lua_getfield(L, -1, potKey);

        if (!lua_istable(L, -1)) {
            lua_pop(L, 3);
            return (false);
        }

        return (true);
    }

    /** Removes what pushUserFunction() left: the entry, the table, preset. */
    void popUserFunction(void)
    {
        lua_pop(L, 3);
    }
} // namespace

bool getUserFunctionName(uint8_t functionId, char *buffer, size_t size)
{
    if ((buffer == nullptr) || (size == 0)) {
        return (false);
    }

    buffer[0] = '\0';

    if (!pushUserFunction(functionId)) {
        return (false);
    }

    bool named = false;

    lua_getfield(L, -1, "name");

    if (lua_isstring(L, -1)) {
        copyString(buffer, lua_tostring(L, -1), size - 1);
        named = (buffer[0] != '\0');
    }

    lua_pop(L, 1);
    popUserFunction();

    return (named);
}

bool runUserFunction(uint8_t functionId)
{
    if (!pushUserFunction(functionId)) {
        return (false);
    }

    lua_getfield(L, -1, "call");

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            System::logger.write(LOG_LUA,
                                 "error running preset function %d: %s",
                                 functionId + 1,
                                 lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        /* Whatever "call" held, so that the entry is on top again. */
        lua_pop(L, 1);
    }

    bool shouldClose = false;

    lua_getfield(L, -1, "close");

    if (lua_isboolean(L, -1)) {
        shouldClose = (lua_toboolean(L, -1) != 0);
    }

    lua_pop(L, 1);
    popUserFunction();

    return (shouldClose);
}
