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
 * @file NoteTriggers.h
 *
 * @brief What schedule.whenNotes() waits for: a combination of held notes.
 */

#pragma once

#include "luaIntegrationBase.h"
#include <cstdint>

/**
 * @brief One preset's note triggers, and the matching they rest on.
 *
 * A trigger is a set of notes and a function. It fires on the change to a
 * device's held notes that makes them match, and not again until a change
 * has made them stop matching - so a chord held down fires once, however
 * many other keys come and go meanwhile, and fires again the next time it is
 * played.
 *
 * The matching is here, free of Lua, so it can be tested on its own. The
 * functions are the preset's, kept in its state's registry; the Lua side
 * (luaNotes.cpp) runs them.
 */
class NoteTriggers
{
public:
    static constexpr uint8_t MaxEntries = 32;
    static constexpr uint8_t MaxPatternNotes = 16;

    /** @brief A trigger's device when it watches every device of the preset. */
    static constexpr uint8_t AnyDevice = 0xFF;

    /**
     * @brief The first handle a trigger is given.
     *
     * Above every handle schedule.after() and schedule.every() give out,
     * which are sixteen bits, so schedule.cancel() can tell which one it was
     * handed.
     */
    static constexpr int32_t FirstHandle = 0x10000;

    /** @brief One held note of a device: by note number, one each. */
    struct HeldNote {
        uint8_t note;
        uint8_t velocity;
        uint32_t timeMs;
    };

    struct Pattern {
        uint8_t notes[MaxPatternNotes]; /**< ascending, no repeats */
        uint8_t count;
        bool exact; /**< nothing else may be held */
        bool anyOctave; /**< notes by name: C is C in every octave */
        bool anyKey; /**< the same shape on any root */
        uint8_t minVelocity;
        uint8_t maxVelocity;
        uint32_t withinMs; /**< 0: pressed any time apart */
    };

    struct Match {
        uint8_t indices[128]; /**< into the held notes, ascending */
        uint8_t count;
        int16_t transpose; /**< how far the pattern was moved to match */
        uint8_t root; /**< the held note playing the pattern's first note */
    };

    /**
     * @brief Whether the held notes match a pattern.
     *
     * @param held a device's held notes, ascending by note number, each note
     *             once.
     */
    static bool match(const Pattern &pattern,
                      const HeldNote *held,
                      uint8_t heldCount,
                      Match &match);

    struct Entry {
        bool used;
        bool once;
        uint8_t deviceId;
        Pattern pattern;
        int functionRef;
        int releaseRef;
        int32_t handle;
        uint64_t matched; /**< one bit per device id: matching right now */
    };

    NoteTriggers();
    ~NoteTriggers() = default;

    /**
     * @brief Adds a trigger whose functions are already in the registry.
     *
     * @returns its handle, or 0 when there is no room.
     */
    int32_t add(const Pattern &pattern,
                uint8_t deviceId,
                bool once,
                int functionRef,
                int releaseRef);

    /** @brief Removes one; false when nothing has that handle. */
    bool cancel(lua_State *L, int32_t handle);

    /**
     * @brief Drops every trigger, releasing their references. With a null
     *        state - closed, its references gone with it - just forgets.
     */
    void clear(lua_State *L);

    uint8_t getCount(void) const;

    /** @brief The entry in a slot, or nullptr when the slot is free. */
    Entry *getEntry(uint8_t index);

private:
    void release(lua_State *L, uint8_t index);

    Entry entries[MaxEntries];
    uint8_t count;
    int32_t nextHandle;
};
