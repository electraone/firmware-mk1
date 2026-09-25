#include "luaHooks.h"
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
