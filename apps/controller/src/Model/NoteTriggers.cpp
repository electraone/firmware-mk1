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

#include "NoteTriggers.h"

/*
 * ---------------------------------------------------------------------------
 * Matching
 * ---------------------------------------------------------------------------
 */

static int16_t findNote(const NoteTriggers::HeldNote *held,
                        uint8_t heldCount,
                        int16_t note)
{
    for (uint8_t i = 0; i < heldCount; i++) {
        if (held[i].note == note) {
            return ((int16_t)i);
        }

        if (held[i].note > note) {
            break;
        }
    }

    return (-1);
}

/** @brief The pattern moved by `transpose`, tried against the held notes. */
static bool matchAt(const NoteTriggers::Pattern &pattern,
                    const NoteTriggers::HeldNote *held,
                    uint8_t heldCount,
                    int16_t transpose,
                    NoteTriggers::Match &match)
{
    match.count = 0;

    if (!pattern.anyOctave) {
        if (pattern.exact && (heldCount != pattern.count)) {
            return (false);
        }

        for (uint8_t i = 0; i < pattern.count; i++) {
            const int16_t index = findNote(
                held, heldCount, (int16_t)(pattern.notes[i] + transpose));

            if (index < 0) {
                return (false);
            }

            match.indices[match.count++] = (uint8_t)index;
        }

        match.root = held[match.indices[0]].note;
    } else {
        /* By name: twelve bits, one per pitch class. */
        uint16_t wanted = 0;
        uint16_t present = 0;

        for (uint8_t i = 0; i < pattern.count; i++) {
            wanted |= (uint16_t)(1u << ((pattern.notes[i] + transpose) % 12));
        }

        for (uint8_t i = 0; i < heldCount; i++) {
            present |= (uint16_t)(1u << (held[i].note % 12));
        }

        if (((present & wanted) != wanted)
            || (pattern.exact && (present != wanted))) {
            return (false);
        }

        const uint8_t rootClass =
            (uint8_t)((pattern.notes[0] + transpose) % 12);
        bool rootFound = false;

        for (uint8_t i = 0; i < heldCount; i++) {
            const uint8_t pitchClass = held[i].note % 12;

            if (wanted & (1u << pitchClass)) {
                match.indices[match.count++] = i;

                if (!rootFound && (pitchClass == rootClass)) {
                    match.root = held[i].note;
                    rootFound = true;
                }
            }
        }
    }

    /* Every note that makes the match has to be in the velocity range. */
    for (uint8_t i = 0; i < match.count; i++) {
        const uint8_t velocity = held[match.indices[i]].velocity;

        if ((velocity < pattern.minVelocity)
            || (velocity > pattern.maxVelocity)) {
            return (false);
        }
    }

    /* And pressed close enough together. Differences from the first, so a
     * wrap of the millisecond clock between two presses changes nothing. */
    if (pattern.withinMs > 0) {
        const uint32_t first = held[match.indices[0]].timeMs;
        int32_t earliest = 0;
        int32_t latest = 0;

        for (uint8_t i = 1; i < match.count; i++) {
            const int32_t offset =
                (int32_t)(held[match.indices[i]].timeMs - first);

            earliest = (offset < earliest) ? offset : earliest;
            latest = (offset > latest) ? offset : latest;
        }

        if ((uint32_t)(latest - earliest) > pattern.withinMs) {
            return (false);
        }
    }

    match.transpose = transpose;

    return (true);
}

bool NoteTriggers::match(const Pattern &pattern,
                         const HeldNote *held,
                         uint8_t heldCount,
                         Match &match)
{
    if ((pattern.count == 0) || (heldCount == 0)) {
        return (false);
    }

    if (!pattern.anyKey) {
        return (matchAt(pattern, held, heldCount, 0, match));
    }

    /* Any key: the pattern's first note can only be on a held note, so each
     * held note names one place to try it - lowest first, so of two places
     * the match is reported at the lower. By name, places an octave apart are
     * the same place and are tried once. */
    uint16_t triedClasses = 0;

    for (uint8_t i = 0; i < heldCount; i++) {
        int16_t transpose = (int16_t)(held[i].note - pattern.notes[0]);

        if (pattern.anyOctave) {
            transpose = (int16_t)(((transpose % 12) + 12) % 12);

            if (triedClasses & (1u << transpose)) {
                continue;
            }
            triedClasses |= (uint16_t)(1u << transpose);
        } else if ((pattern.notes[pattern.count - 1] + transpose) > 127) {
            break;
        }

        if (matchAt(pattern, held, heldCount, transpose, match)) {
            return (true);
        }
    }

    return (false);
}

/*
 * ---------------------------------------------------------------------------
 * The entries
 * ---------------------------------------------------------------------------
 */

NoteTriggers::NoteTriggers() : count(0), nextHandle(FirstHandle)
{
    for (Entry &entry : entries) {
        entry.used = false;
        entry.once = false;
        entry.deviceId = AnyDevice;
        entry.pattern = Pattern{};
        entry.functionRef = LUA_NOREF;
        entry.releaseRef = LUA_NOREF;
        entry.handle = 0;
        entry.matched = 0;
    }
}

int32_t NoteTriggers::add(const Pattern &pattern,
                          uint8_t deviceId,
                          bool once,
                          int functionRef,
                          int releaseRef)
{
    for (Entry &entry : entries) {
        if (entry.used) {
            continue;
        }

        entry.used = true;
        entry.once = once;
        entry.deviceId = deviceId;
        entry.pattern = pattern;
        entry.functionRef = functionRef;
        entry.releaseRef = releaseRef;
        entry.matched = 0;

        /* Never reused while the handles last, and they last for two
         * billion triggers. */
        if (nextHandle < FirstHandle) {
            nextHandle = FirstHandle;
        }

        entry.handle = nextHandle++;
        count++;

        return (entry.handle);
    }

    return (0);
}

void NoteTriggers::release(lua_State *L, uint8_t index)
{
    Entry &entry = entries[index];

    if (!entry.used) {
        return;
    }

    if (L) {
        if (entry.functionRef != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, entry.functionRef);
        }

        if (entry.releaseRef != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, entry.releaseRef);
        }
    }

    entry.used = false;
    entry.functionRef = LUA_NOREF;
    entry.releaseRef = LUA_NOREF;
    entry.handle = 0;
    entry.matched = 0;

    if (count > 0) {
        count--;
    }
}

bool NoteTriggers::cancel(lua_State *L, int32_t handle)
{
    if (handle < FirstHandle) {
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

void NoteTriggers::clear(lua_State *L)
{
    for (uint8_t i = 0; i < MaxEntries; i++) {
        release(L, i);
    }

    count = 0;
}

uint8_t NoteTriggers::getCount(void) const
{
    return (count);
}

NoteTriggers::Entry *NoteTriggers::getEntry(uint8_t index)
{
    if ((index >= MaxEntries) || !entries[index].used) {
        return (nullptr);
    }

    return (&entries[index]);
}
