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

#include "luaTimer.h"
#include "luaExtensionBase.h"

int luaopen_timer(lua_State *L)
{
    luaL_newlib(L, timer_functions);
    return 1;
}

int timer_enable(lua_State *L)
{
    lua_settop(L, 0);
    System::tasks.enableTimer();

    return (0);
}

int timer_disable(lua_State *L)
{
    lua_settop(L, 0);
    System::tasks.disableTimer();

    return (0);
}

int timer_isEnabled(lua_State *L)
{
    lua_pushboolean(L, System::tasks.isTimerEnabled());
    return (1);
}

int timer_setPeriod(lua_State *L)
{
    lua_settop(L, 1);

    int period = luaL_checkinteger(L, 1);

    if (period < 10) {
        return (luaL_error(L, "Minimum timer period is 10 mSec"));
    }

    /* A minute. The interval is held in microseconds, so anything much longer
     * than this overflows the unsigned long it is kept in. */
    if (period > 60000) {
        return (luaL_error(L, "Maximum timer period is 60000 mSec"));
    }

    System::tasks.timerSetInterval(period * 1000);

    return (0);
}

int timer_getPeriod(lua_State *L)
{
    unsigned long interval = System::tasks.timerGetInterval();

    lua_pushinteger(L, interval / 1000);
    return (1);
}

int timer_setBpm(lua_State *L)
{
    lua_settop(L, 1);

    int bpm = luaL_checkinteger(L, 1);

    /* The lower bound is what was missing: zero divided the tick length by it,
     * and a negative one set an interval of some hundreds of seconds. */
    if (bpm < 1) {
        return (luaL_error(L, "Minimum timer BPM is 1"));
    }

    if (bpm > 6000) {
        return (luaL_error(L, "Maximum timer BPM is 6000"));
    }

    System::tasks.timerSetInterval(60000000 / bpm);

    return (0);
}

int timer_getBpm(lua_State *L)
{
    unsigned long interval = System::tasks.timerGetInterval();

    lua_pushinteger(L, (interval > 0) ? (60000000 / interval) : 0);
    return (1);
}

void timer_runCallback(lua_State *L)
{
    luaLE_getModuleFunction(L, "timer", "onTick");

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function 'timer.callback': %s",
                                 lua_tostring(L, -1));
        }
    }
}
