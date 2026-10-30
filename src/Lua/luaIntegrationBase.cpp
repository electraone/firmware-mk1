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

#include "luaIntegrationBase.h"
#include "luaLazyGlobals.h"
#include "SysexBlock.h"
#include "System.h"
#include "Trace.h"

/**
 * @brief Records a module function lookup made on behalf of a real call.
 *
 * Every C++ to Lua call of the form module.function() resolves the function
 * through luaLE_getModuleFunction() first, so this one place covers all of
 * them. Whether the call will actually be made is already known here, which
 * is why a missing handler is recorded separately from a call.
 */
static void
    traceModuleLookup(lua_State *L, const char *module, const char *function)
{
#if defined(ELECTRA_TRACE) && ELECTRA_TRACE
    if (lua_isfunction(L, -1)) {
        TRACE_LUA_ENTER(module, function);
    } else {
        TRACE_LUA_MISSING(module, function);
    }
#else
    (void)L;
    (void)module;
    (void)function;
#endif
}

void luaLE_pushTableString(lua_State *L, const char *key, const char *value)
{
    lua_pushstring(L, key);
    lua_pushstring(L, value);
    lua_settable(L, -3);
}

void luaLE_pushTableInteger(lua_State *L, const char *key, int value)
{
    lua_pushstring(L, key);
    lua_pushinteger(L, value);
    lua_settable(L, -3);
}

void luaLE_pushTableObject(lua_State *L,
                           const char *key,
                           const char *objectName,
                           const void *object)
{
    lua_pushstring(L, key);
    luaLE_pushObject(L, objectName, object);
    lua_settable(L, -3);
}

void luaLE_pushArrayInteger(lua_State *L, int key, int value)
{
    lua_pushinteger(L, key);
    lua_pushinteger(L, value);
    lua_settable(L, -3);
}

void luaLE_pushArrayString(lua_State *L, int key, const char *value)
{
    lua_pushinteger(L, key);
    lua_pushstring(L, value);
    lua_settable(L, -3);
}

void luaLE_pushArrayObject(lua_State *L,
                           int key,
                           const char *objectName,
                           const void *object)
{
    lua_pushinteger(L, key);
    luaLE_pushObject(L, objectName, object);
    lua_settable(L, -3);
}

void luaLE_pushObject(lua_State *L, const char *objectName, const void *object)
{
    /* A type's metatable is made the first time an object of it is pushed,
     * the way a script's first use of a name makes it - see luaLazyGlobals.h.
     * Done before the userdata so that registering leaves the stack as it
     * found it underneath. */
    luaLE_ensureType(L, objectName);

    *reinterpret_cast<const void **>(lua_newuserdata(L, sizeof(void *))) =
        object;

    luaL_getmetatable(L, objectName);
    lua_setmetatable(L, -2);
}

bool luaLE_checkBoolean(lua_State *L, int idx)
{
    return (lua_toboolean(L, idx));
}

bool luaLE_optBoolean(lua_State *L, int idx, bool defaultValue)
{
    if (lua_isnoneornil(L, idx)) {
        return (defaultValue);
    }

    if (lua_isboolean(L, idx)) {
        return (lua_toboolean(L, idx));
    }

    const lua_Integer value = luaL_checkinteger(L, idx);

    if ((value < 0) || (value > 1)) {
        luaL_error(
            L, "flag is neither a boolean nor 0 or 1: value=%d", (int)value);
    }

    return (value != 0);
}

void luaLE_getModuleFunction(lua_State *L,
                             const char *module,
                             const char *function)
{
    lua_getglobal(L, module);
    lua_pushstring(L, function);
    lua_gettable(L, -2);
    lua_remove(L, -2);
    traceModuleLookup(L, module, function);
}

void luaLE_addObjectMethods(lua_State *L, const luaL_Reg *l)
{
    for (; l->name; l++) {
        lua_pushcfunction(L, l->func);
        lua_setfield(L, -(2), l->name);
    }
}

void luaLE_postFunctionCleanUp(lua_State *L)
{
    /** @attention: trigger the garbage collection.
     * For now, after each function call. This needs to be improved.
     */
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_settop(L, 0);
}

void luaLE_dumpstack(lua_State *L)
{
    int top = lua_gettop(L);

    System::logger.write(LOG_LUA, "stack size: %d", top);

    for (int i = 1; i <= top; i++) {
        System::logger.write(LOG_LUA, "%d -> %s   ", i, luaL_typename(L, i));

        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                System::logger.write(LOG_LUA, "number: %g", lua_tonumber(L, i));
                break;
            case LUA_TSTRING:
                System::logger.write(LOG_LUA, "string: %s", lua_tostring(L, i));
                break;
            case LUA_TBOOLEAN:
                System::logger.write(LOG_LUA,
                                     "bool: %s",
                                     (lua_toboolean(L, i) ? "true" : "false"));
                break;
            case LUA_TNIL:
                System::logger.write(LOG_LUA, "nil: %s", "nil");
                break;
            default:
                System::logger.write(
                    LOG_LUA, "pointer: %p", lua_topointer(L, i));
                break;
        }
    }
}

bool luaLE_functionExists(const char *module, const char *function)
{
    /** @attention the lookup is done here rather than through
     * luaLE_getModuleFunction() on purpose. This is a probe, not a call, and
     * it must not show up in the behavioural trace as one.
     */
    const int top = lua_gettop(L);

    /* Raw: a module the script has never named has not been made, and cannot
     * hold a function the script put there. Asking through _G would make it,
     * and every module with a callback in it would be made for every script. */
    lua_pushglobaltable(L);
    lua_pushstring(L, module);
    lua_rawget(L, -2);

    const bool exists = (lua_type(L, -1) == LUA_TTABLE)
                        && (lua_getfield(L, -1, function) == LUA_TFUNCTION);

    /* A probe leaves nothing behind. Every one of the eighteen MIDI handlers
     * is looked for this way when a preset loads, and the module table and
     * the field each used to stay on the stack - thirty-six values pushed
     * onto a stack that is only guaranteed twenty deep, with nothing at the
     * call site popping them. */
    lua_settop(L, top);

    return (exists);
}

void luaLE_getModuleFunction(const char *module, const char *function)
{
    lua_getglobal(L, module);
    lua_getfield(L, -1, function);
    traceModuleLookup(L, module, function);
}
