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

#include "ScheduledFunctions.h"
#include "System.h"

ScheduledFunctions::ScheduledFunctions()
    : count(0), nextHandle(1), stats{ 0, 0, 0 }
{
    for (Entry &entry : entries) {
        entry.used = false;
        entry.repeating = false;
        entry.due = 0;
        entry.period = 0;
        entry.functionRef = LUA_NOREF;
        entry.argumentsRef = LUA_NOREF;
        entry.handle = 0;
    }
}

/*
 * The next handle nothing live is already using.
 *
 * Handles are sixteen bits and only ever counted up, so a script that
 * schedules steadily - a sequencer asking for a few a beat - reaches 65536 in
 * a few hours and starts again at 1. A repeating task that has been running
 * since boot still holds handle 1, and cancel() finds by handle: the wrap made
 * the new task's handle cancel the old task instead. Skipping the handles that
 * are in use costs a walk of at most MaxEntries and makes the documented
 * promise - a handle is never reused while its task is alive - true.
 */
uint16_t ScheduledFunctions::takeHandle(void)
{
    for (uint32_t attempt = 0; attempt <= 0xffff; attempt++) {
        /* Never zero: zero is "no handle" to a script. */
        if (nextHandle == 0) {
            nextHandle = 1;
        }

        const uint16_t handle = nextHandle++;

        if (!handleInUse(handle)) {
            return (handle);
        }
    }

    /* Unreachable: the table holds far fewer than 65535 entries. */
    return (nextHandle++);
}

bool ScheduledFunctions::handleInUse(uint16_t handle) const
{
    for (const Entry &entry : entries) {
        if (entry.used && (entry.handle == handle)) {
            return (true);
        }
    }

    return (false);
}

uint16_t ScheduledFunctions::add(uint32_t now,
                                 uint32_t ms,
                                 bool repeating,
                                 int functionRef,
                                 int argumentsRef)
{
    for (Entry &entry : entries) {
        if (entry.used) {
            continue;
        }

        entry.used = true;
        entry.repeating = repeating;
        entry.due = now + ms;
        entry.period = ms;
        entry.functionRef = functionRef;
        entry.argumentsRef = argumentsRef;

        entry.handle = takeHandle();
        count++;

        return (entry.handle);
    }

    return (0);
}

void ScheduledFunctions::release(lua_State *L, uint8_t index)
{
    Entry &entry = entries[index];

    if (!entry.used) {
        return;
    }

    if (L) {
        if (entry.functionRef != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, entry.functionRef);
        }

        if (entry.argumentsRef != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, entry.argumentsRef);
        }
    }

    entry.used = false;
    entry.repeating = false;
    entry.functionRef = LUA_NOREF;
    entry.argumentsRef = LUA_NOREF;
    entry.handle = 0;

    if (count > 0) {
        count--;
    }
}

bool ScheduledFunctions::cancel(lua_State *L, uint16_t handle)
{
    if (handle == 0) {
        return (false);
    }

    for (uint8_t i = 0; i < MaxEntries; i++) {
        if (entries[i].used && (entries[i].handle == handle)) {
            release(L, i);
            return (true);
        }
    }

    return (false);
}

bool ScheduledFunctions::hasEntries(void) const
{
    return (count > 0);
}

bool ScheduledFunctions::needsAttention(uint32_t now) const
{
    if (count == 0) {
        return (false);
    }

    for (const Entry &entry : entries) {
        /* Subtraction rather than a comparison, so a wrap of the millisecond
         * counter does not make everything look overdue. */
        if (entry.used && ((int32_t)(now - entry.due) >= 0)) {
            return (true);
        }
    }

    return (false);
}

uint8_t ScheduledFunctions::getPendingCount(void) const
{
    return (count);
}

const ScheduledFunctions::Stats &ScheduledFunctions::getStats(void) const
{
    return (stats);
}

void ScheduledFunctions::run(lua_State *L, uint32_t now)
{
    if (!L || (count == 0)) {
        return;
    }

    for (uint8_t i = 0; i < MaxEntries; i++) {
        Entry &entry = entries[i];

        if (!entry.used || ((int32_t)(now - entry.due) < 0)) {
            continue;
        }

        const uint32_t overrun = now - entry.due;

        if (overrun > stats.worstOverrunMs) {
            stats.worstOverrunMs = overrun;
        }

        /* The handle it fired under. The callback may cancel this very entry,
         * or schedule something that lands in this slot; what happens to the
         * slot afterwards depends on whether it is still the same entry. */
        const uint16_t fired = entry.handle;

        lua_rawgeti(L, LUA_REGISTRYINDEX, entry.functionRef);

        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            release(L, i);
            continue;
        }

        int argumentCount = 0;

        if (entry.argumentsRef != LUA_NOREF) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, entry.argumentsRef);

            if (lua_istable(L, -1)) {
                const lua_Integer length = luaL_len(L, -1);

                for (lua_Integer j = 1; j <= length; j++) {
                    lua_geti(L, -(int)j, j);
                }

                /* The table itself is below the arguments now; take it out. */
                lua_remove(L, -(int)length - 1);
                argumentCount = (int)length;
            } else {
                lua_pop(L, 1);
            }
        }

        const int status = lua_pcall(L, argumentCount, 0, 0);

        if (status != LUA_OK) {
            stats.errors++;

            /* Rate limited, and at LOG_ERROR: LOG_LUA bypasses the logger's
             * enabled flag, and a callback that raises every millisecond must
             * not be able to flood the port. See timer_runCallback. */
            static uint32_t lastReport = 0;

            if ((lastReport == 0) || ((now - lastReport) >= 2000)) {
                lastReport = now;
                System::logger.write(LOG_ERROR,
                                     "error running a scheduled function: %s",
                                     lua_tostring(L, -1));
            }

            lua_pop(L, 1);

            /* Dropped rather than left to raise again on every pass. */
            if (entry.used && (entry.handle == fired)) {
                release(L, i);
            }

            continue;
        }

        stats.ran++;

        if (!entry.used || (entry.handle != fired)) {
            continue; /* it cancelled itself, or the slot was reused */
        }

        if (entry.repeating) {
            entry.due = now + entry.period;
        } else {
            release(L, i);
        }
    }
}

void ScheduledFunctions::clear(lua_State *L)
{
    for (uint8_t i = 0; i < MaxEntries; i++) {
        release(L, i);
    }

    count = 0;
}
