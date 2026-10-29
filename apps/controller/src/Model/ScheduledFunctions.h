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

/**
 * @file ScheduledFunctions.h
 *
 * @brief What a preset's schedule.after() and schedule.every() have queued.
 *
 * One for the running preset, beside its timer. The timer is one periodic callback
 * with one period, and everything a preset wanted to happen later - an LFO, a
 * delayed request, a message sent after a knob stops moving - was multiplexed
 * into it by hand. This is the router's scheduler, on the preset side, with
 * the router's names and semantics: a bounded table of entries, each holding
 * its function and arguments in the state's registry, run from the
 * application thread's loop when they fall due.
 */

#pragma once

#include "luaIntegrationBase.h"
#include <cstdint>

class ScheduledFunctions
{
public:
    static constexpr uint8_t MaxEntries = 32;

    struct Stats {
        uint32_t ran; /**< callbacks that ran to the end */
        uint32_t errors; /**< callbacks that raised */
        uint32_t worstOverrunMs; /**< the latest a callback ever ran */
    };

    ScheduledFunctions();
    ~ScheduledFunctions() = default;

    /**
     * @brief Queues a function held in the state's registry.
     *
     * @param now the clock the due time is measured from
     * @return a handle for cancel(), or 0 when there is no slot left
     */
    uint16_t add(uint32_t now,
                 uint32_t ms,
                 bool repeating,
                 int functionRef,
                 int argumentsRef);

    /** @brief Removes one entry; false when no entry has that handle. */
    bool cancel(lua_State *L, uint16_t handle);

    /** @brief Whether anything is queued at all. */
    bool hasEntries(void) const;

    /** @brief Whether anything is due at `now`. */
    bool needsAttention(uint32_t now) const;

    uint8_t getPendingCount(void) const;
    const Stats &getStats(void) const;

    /**
     * @brief Runs whatever has fallen due, from the main loop.
     *
     * A repeating entry that raises is dropped rather than left to raise on
     * every pass; one that ran is rearmed from `now`, not from when it was
     * due, so a callback that fell behind does not fire a burst to catch up.
     */
    void run(lua_State *L, uint32_t now);

    /**
     * @brief Drops every entry, releasing their registry references.
     *
     * With a null state - the state is already closed, and the references
     * died with it - the table is simply forgotten.
     */
    void clear(lua_State *L);

private:
    struct Entry {
        bool used;
        bool repeating;
        uint32_t due;
        uint32_t period;
        int functionRef;
        int argumentsRef;
        uint16_t handle;
    };

    void release(lua_State *L, uint8_t index);

    Entry entries[MaxEntries];
    uint8_t count;
    /** @brief A handle no live entry holds. */
    uint16_t takeHandle(void);

    /** @brief Whether a live entry already answers to this handle. */
    bool handleInUse(uint16_t handle) const;

    uint16_t nextHandle;
    Stats stats;
};
