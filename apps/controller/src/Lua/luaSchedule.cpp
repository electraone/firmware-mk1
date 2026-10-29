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

#include "luaSchedule.h"
#include "luaExtension.h"
#include "ScheduledFunctions.h"
#include "NoteTriggers.h"
#include "HeldNotes.h"
#include "System.h"

namespace
{
    /* One of each: this model runs one preset, in one Lua state. */
    ScheduledFunctions scheduler;
    NoteTriggers noteTriggers;
    HeldNotes heldNotes;

    /* A device's held notes, one per note number, ascending. */
    NoteTriggers::HeldNote dispatchNotes[HeldNotes::Capacity];

    /* The registry references and the stored functions all belong to the
     * preset's state; with no preset there is nothing for them to run in. */
    void checkPreset(lua_State *L)
    {
        if (luaPreset == nullptr) {
            luaL_error(L, "schedule: this script has no preset to run in");
        }
    }

    /** @brief Shared by after() and every(). */
    int scheduleFunction(lua_State *L, bool repeating)
    {
        const lua_Integer ms = luaL_checkinteger(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);

        checkPreset(L);

        if (ms < 0) {
            return (luaL_error(L, "schedule: a delay cannot be negative"));
        }

        /* A repeating entry with no period would run for ever inside one pass
         * and never let the loop get back to the knobs. */
        const uint32_t period = (repeating && (ms < 1)) ? 1 : (uint32_t)ms;

        /* Whatever follows the function travels with it, as the arguments it
         * is called with. Kept in the registry as one table, so the entry
         * holds one reference for all of them. */
        int argumentsRef = LUA_NOREF;
        const int extra = lua_gettop(L) - 2;

        if (extra > 0) {
            lua_createtable(L, extra, 0);

            for (int i = 0; i < extra; i++) {
                lua_pushvalue(L, 3 + i);
                lua_seti(L, -2, i + 1);
            }

            argumentsRef = luaL_ref(L, LUA_REGISTRYINDEX);
        }

        lua_pushvalue(L, 2);
        const int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);

        const uint16_t handle = scheduler.add(
            millis(), period, repeating, functionRef, argumentsRef);

        if (handle == 0) {
            luaL_unref(L, LUA_REGISTRYINDEX, functionRef);

            if (argumentsRef != LUA_NOREF) {
                luaL_unref(L, LUA_REGISTRYINDEX, argumentsRef);
            }

            return (luaL_error(L,
                               "schedule: nothing more can be scheduled just "
                               "now (%d are queued)",
                               (int)ScheduledFunctions::MaxEntries));
        }

        lua_pushinteger(L, handle);
        return (1);
    }

    /*
     * Whether a held note is this device's: it is when the preset hands an
     * incoming message from there to this device - the same look-up the
     * parameter map uses, so the two never disagree about whose note it is.
     */
    bool isDevicesNote(const Device &device, const HeldNotes::Note &note)
    {
        const Device &owner = luaPreset->getDevice(note.port, note.channel);

        return (owner.isValid() && (owner.getId() == device.getId()));
    }

    /** @returns how many notes it wrote to `out`. */
    uint8_t collectNotes(const Device &device, NoteTriggers::HeldNote *out)
    {
        uint8_t count = 0;

        for (uint8_t i = 0; i < heldNotes.size(); i++) {
            const HeldNotes::Note &note = heldNotes.at(i);

            if (!isDevicesNote(device, note)) {
                continue;
            }

            /* One entry per note number, whichever input it came in on. The
             * set is in the order notes were pressed, so a later one is the
             * newer press and its velocity is the one that counts. */
            uint8_t at = 0;

            while ((at < count) && (out[at].note != note.note)) {
                at++;
            }

            out[at] = NoteTriggers::HeldNote{ .note = note.note,
                                              .velocity = note.velocity,
                                              .timeMs = note.timeMs };

            if (at == count) {
                count++;
            }
        }

        /* Ascending by note. A handful of notes: insertion sort. */
        for (uint8_t i = 1; i < count; i++) {
            const NoteTriggers::HeldNote moving = out[i];
            uint8_t j = i;

            while ((j > 0) && (out[j - 1].note > moving.note)) {
                out[j] = out[j - 1];
                j--;
            }

            out[j] = moving;
        }

        return (count);
    }

    void pushNote(lua_State *L, const NoteTriggers::HeldNote &note)
    {
        lua_createtable(L, 0, 3);
        luaLE_pushTableInteger(L, "note", note.note);
        luaLE_pushTableInteger(L, "velocity", note.velocity);
        luaLE_pushTableInteger(L, "time", (int)note.timeMs);
    }

    void pushNotes(lua_State *L,
                   const NoteTriggers::HeldNote *notes,
                   uint8_t count)
    {
        lua_createtable(L, count, 0);

        for (uint8_t i = 0; i < count; i++) {
            pushNote(L, notes[i]);
            lua_rawseti(L, -2, i + 1);
        }
    }

    /* What a trigger's function is handed about the match it fired on. */
    void pushMatch(lua_State *L,
                   const NoteTriggers::HeldNote *held,
                   const NoteTriggers::Match &match)
    {
        lua_createtable(L, 0, 5);

        lua_createtable(L, match.count, 0);

        uint32_t velocities = 0;
        const uint32_t first = held[match.indices[0]].timeMs;
        int32_t earliest = 0;
        int32_t latest = 0;

        for (uint8_t i = 0; i < match.count; i++) {
            const NoteTriggers::HeldNote &note = held[match.indices[i]];
            const int32_t offset = (int32_t)(note.timeMs - first);

            pushNote(L, note);
            lua_rawseti(L, -2, i + 1);

            velocities += note.velocity;
            earliest = (offset < earliest) ? offset : earliest;
            latest = (offset > latest) ? offset : latest;
        }

        lua_setfield(L, -2, "notes");

        luaLE_pushTableInteger(L, "root", match.root);
        luaLE_pushTableInteger(L, "transpose", match.transpose);
        luaLE_pushTableInteger(
            L, "velocity", (int)((velocities + match.count / 2) / match.count));
        luaLE_pushTableInteger(L, "spread", (int)(latest - earliest));
    }

    /* Rate limited, as ScheduledFunctions::run is: a function that raises on every
     * note must not flood the port. */
    void reportError(lua_State *L, const char *what)
    {
        static uint32_t lastReport = 0;
        const uint32_t now = millis();

        if ((lastReport == 0) || ((now - lastReport) >= 2000)) {
            lastReport = now;
            System::logger.write(
                LOG_ERROR, "error running %s: %s", what, lua_tostring(L, -1));
        }
    }

    /*
     * The preset's triggers, against one device's notes after a change. A
     * trigger fires on the change that makes the notes match, and its
     * onRelease on the one that makes them stop.
     */
    void runTriggers(lua_State *L,
                     const Device &device,
                     const NoteTriggers::HeldNote *held,
                     uint8_t count)
    {
        const uint8_t deviceId = device.getId();
        const uint64_t bit = 1ull << (deviceId & 63);
        NoteTriggers::Match match;

        for (uint8_t i = 0; i < NoteTriggers::MaxEntries; i++) {
            NoteTriggers::Entry *entry = noteTriggers.getEntry(i);

            if ((entry == nullptr)
                || ((entry->deviceId != NoteTriggers::AnyDevice)
                    && (entry->deviceId != deviceId))) {
                continue;
            }

            const bool matches =
                NoteTriggers::match(entry->pattern, held, count, match);
            const bool matched = (entry->matched & bit) != 0;

            if (matches == matched) {
                continue;
            }

            entry->matched =
                matches ? (entry->matched | bit) : (entry->matched & ~bit);

            const int ref = matches ? entry->functionRef : entry->releaseRef;

            if (ref == LUA_NOREF) {
                continue;
            }

            /* The handle it fired under: the function may cancel this very
             * trigger, or set one that lands in this slot. */
            const int32_t handle = entry->handle;
            const bool once = matches && entry->once;

            lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

            /* Gone before it runs, so a once trigger is gone even if its
             * function raises; the function itself is on the stack. */
            if (once) {
                noteTriggers.cancel(L, handle);
            }

            luaLE_pushObject(L, "Device", &device);
            pushNotes(L, held, count);

            int arguments = 2;

            if (matches) {
                pushMatch(L, held, match);
                arguments = 3;
            }

            const bool raised = (lua_pcall(L, arguments, 0, 0) != LUA_OK);

            if (raised) {
                reportError(L, "a schedule.whenNotes function");
            }

            luaLE_postFunctionCleanUp(L);

            if (raised) {
                /* Dropped, as a repeating scheduled function that raises is. */
                NoteTriggers::Entry *same = noteTriggers.getEntry(i);

                if (same && (same->handle == handle)) {
                    noteTriggers.cancel(L, handle);
                }
            }
        }
    }

    void dispatchDevice(lua_State *L,
                        const Device &device,
                        bool isNoteOff,
                        uint8_t releasedNote)
    {
        const uint8_t count = collectNotes(device, dispatchNotes);

        /* Released on one input and still held on another: nothing changed
         * for the device. */
        if (isNoteOff) {
            for (uint8_t i = 0; i < count; i++) {
                if (dispatchNotes[i].note == releasedNote) {
                    return;
                }
            }
        }

        runTriggers(L, device, dispatchNotes, count);
    }

    /** @brief Reads the note list into the pattern: ascending, each once. */
    void checkPatternNotes(lua_State *L, NoteTriggers::Pattern &pattern)
    {
        auto add = [L, &pattern](lua_Integer note) {
            if ((note < 0) || (note > 127)) {
                luaL_error(L, "schedule.whenNotes: a note is 0 to 127");
            }

            uint8_t at = 0;

            while ((at < pattern.count) && (pattern.notes[at] < note)) {
                at++;
            }

            if ((at < pattern.count) && (pattern.notes[at] == note)) {
                return;
            }

            if (pattern.count >= NoteTriggers::MaxPatternNotes) {
                luaL_error(L,
                           "schedule.whenNotes: at most %d notes",
                           (int)NoteTriggers::MaxPatternNotes);
            }

            for (uint8_t i = pattern.count; i > at; i--) {
                pattern.notes[i] = pattern.notes[i - 1];
            }

            pattern.notes[at] = (uint8_t)note;
            pattern.count++;
        };

        if (lua_isinteger(L, 1)) {
            add(lua_tointeger(L, 1));
            return;
        }

        luaL_checktype(L, 1, LUA_TTABLE);

        const lua_Integer length = luaL_len(L, 1);

        for (lua_Integer i = 1; i <= length; i++) {
            lua_rawgeti(L, 1, i);

            if (!lua_isinteger(L, -1)) {
                luaL_error(L, "schedule.whenNotes: notes are note numbers");
            }

            add(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }

        if (pattern.count == 0) {
            luaL_error(L, "schedule.whenNotes: no notes to wait for");
        }
    }

    /** @brief An optional boolean field of the options table. */
    bool optionFlag(lua_State *L, const char *name)
    {
        lua_getfield(L, 3, name);
        const bool flag = lua_toboolean(L, -1) != 0;
        lua_pop(L, 1);

        return (flag);
    }

    /** @brief An optional whole-number field, within bounds. */
    lua_Integer optionNumber(lua_State *L,
                             const char *name,
                             lua_Integer fallback,
                             lua_Integer lowest,
                             lua_Integer highest)
    {
        lua_Integer value = fallback;

        lua_getfield(L, 3, name);

        if (!lua_isnil(L, -1)) {
            if (!lua_isinteger(L, -1)) {
                luaL_error(L, "schedule.whenNotes: %s is a whole number", name);
            }

            value = lua_tointeger(L, -1);

            if ((value < lowest) || (value > highest)) {
                luaL_error(L,
                           "schedule.whenNotes: %s is %d to %d",
                           name,
                           (int)lowest,
                           (int)highest);
            }
        }

        lua_pop(L, 1);

        return (value);
    }
} // namespace

int luaopen_schedule(lua_State *L)
{
    luaL_newlib(L, schedule_functions);
    return (1);
}

int schedule_after(lua_State *L)
{
    return (scheduleFunction(L, false));
}

int schedule_every(lua_State *L)
{
    return (scheduleFunction(L, true));
}

int schedule_cancel(lua_State *L)
{
    lua_settop(L, 1);

    const lua_Integer handle = luaL_checkinteger(L, 1);

    checkPreset(L);

    /* A whenNotes() trigger: its handles start above every sixteen-bit one
     * after() and every() give out. */
    if (handle >= NoteTriggers::FirstHandle) {
        lua_pushboolean(L,
                        (handle <= 0x7fffffff)
                            ? noteTriggers.cancel(L, (int32_t)handle)
                            : 0);
        return (1);
    }

    lua_pushboolean(L,
                    ((handle > 0) && (handle <= 0xffff))
                        ? scheduler.cancel(L, (uint16_t)handle)
                        : 0);
    return (1);
}

int schedule_now(lua_State *L)
{
    lua_settop(L, 0);
    lua_pushinteger(L, (lua_Integer)millis());
    return (1);
}

int schedule_stats(lua_State *L)
{
    lua_settop(L, 0);

    checkPreset(L);

    const ScheduledFunctions::Stats &stats = scheduler.getStats();

    lua_createtable(L, 0, 6);

    lua_pushinteger(L, noteTriggers.getCount());
    lua_setfield(L, -2, "whenNotes");

    lua_pushinteger(L, scheduler.getPendingCount());
    lua_setfield(L, -2, "pending");
    lua_pushinteger(L, (lua_Integer)stats.ran);
    lua_setfield(L, -2, "ran");
    lua_pushinteger(L, (lua_Integer)stats.errors);
    lua_setfield(L, -2, "errors");
    lua_pushinteger(L, (lua_Integer)stats.worstOverrunMs);
    lua_setfield(L, -2, "worstOverrun");
    lua_pushinteger(L, ScheduledFunctions::MaxEntries);
    lua_setfield(L, -2, "capacity");

    return (1);
}

int schedule_whenNotes(lua_State *L)
{
    lua_settop(L, 3);

    checkPreset(L);

    NoteTriggers::Pattern pattern{};

    pattern.maxVelocity = 127;

    checkPatternNotes(L, pattern);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    uint8_t deviceId = NoteTriggers::AnyDevice;
    bool once = false;
    bool hasRelease = false;

    if (!lua_isnil(L, 3)) {
        luaL_checktype(L, 3, LUA_TTABLE);

        lua_getfield(L, 3, "device");

        if (void *asDevice = luaL_testudata(L, -1, "Device")) {
            Device *device = *reinterpret_cast<Device **>(asDevice);

            if ((device == nullptr) || !device->isValid()) {
                return (
                    luaL_error(L, "schedule.whenNotes: not a valid device"));
            }

            deviceId = device->getId();
        } else if (lua_isinteger(L, -1)) {
            const lua_Integer id = lua_tointeger(L, -1);

            if ((id < 0) || (id >= 63)) {
                return (luaL_error(L,
                                   "schedule.whenNotes: device is a Device "
                                   "or a device id"));
            }

            deviceId = (uint8_t)id;
        } else if (!lua_isnil(L, -1)) {
            return (luaL_error(
                L, "schedule.whenNotes: device is a Device or a device id"));
        }

        lua_pop(L, 1);

        pattern.exact = optionFlag(L, "exact");
        pattern.anyOctave = optionFlag(L, "anyOctave");
        pattern.anyKey = optionFlag(L, "anyKey");
        once = optionFlag(L, "once");

        pattern.minVelocity =
            (uint8_t)optionNumber(L, "minVelocity", 0, 0, 127);
        pattern.maxVelocity =
            (uint8_t)optionNumber(L, "maxVelocity", 127, 0, 127);
        pattern.withinMs = (uint32_t)optionNumber(L, "within", 0, 0, 60000);

        lua_getfield(L, 3, "onRelease");

        if (!lua_isnil(L, -1)) {
            if (!lua_isfunction(L, -1)) {
                return (luaL_error(
                    L, "schedule.whenNotes: onRelease is a function"));
            }
            hasRelease = true;
        }

        lua_pop(L, 1);
    }

    if (pattern.minVelocity > pattern.maxVelocity) {
        return (luaL_error(
            L, "schedule.whenNotes: minVelocity is above maxVelocity"));
    }

    /* Everything checked: nothing below raises before the references are
     * either stored or given back. */
    lua_pushvalue(L, 2);
    const int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    int releaseRef = LUA_NOREF;

    if (hasRelease) {
        lua_getfield(L, 3, "onRelease");
        releaseRef = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    const int32_t handle =
        noteTriggers.add(pattern, deviceId, once, functionRef, releaseRef);

    if (handle == 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);

        if (releaseRef != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, releaseRef);
        }

        return (luaL_error(L,
                           "schedule.whenNotes: nothing more can be waited "
                           "for just now (%d are set)",
                           (int)NoteTriggers::MaxEntries));
    }

    lua_pushinteger(L, handle);

    return (1);
}

void schedule_run(lua_State *L, uint32_t now)
{
    if (L && scheduler.needsAttention(now)) {
        scheduler.run(L, now);
    }
}

void schedule_reset(lua_State *L)
{
    scheduler.clear(L);
    noteTriggers.clear(L);
}

void schedule_onMidiMessage(MidiInput midiInput,
                            uint8_t channel,
                            MidiMessage::Type type,
                            uint8_t data1,
                            uint8_t data2)
{
    const uint8_t port = midiInput.getPort();

    if (!heldNotes.process(midiInput.getInterfaceType(),
                           port,
                           type,
                           channel,
                           data1,
                           data2,
                           millis())) {
        return;
    }

    if (!L || !luaPreset || (noteTriggers.getCount() == 0)) {
        return;
    }

    /* A note's change concerns the one device it belongs to; a controller or
     * a reset that ends notes may end them for every device on the input. */
    if ((type == MidiMessage::Type::NoteOn)
        || (type == MidiMessage::Type::NoteOff)) {
        const Device &device = luaPreset->getDevice(port, channel);

        if (device.isValid()) {
            const bool isNoteOff =
                (type == MidiMessage::Type::NoteOff) || (data2 == 0);

            dispatchDevice(L, device, isNoteOff, data1);
        }
        return;
    }

    /* By id rather than by walking the map, because the functions run
     * meanwhile may add devices to it. */
    uint8_t ids[64];
    uint8_t found = 0;

    for (const auto &[id, device] : luaPreset->devices) {
        if (device.isValid() && (found < 64)) {
            ids[found++] = id;
        }
    }

    for (uint8_t i = 0; i < found; i++) {
        const Device &device = luaPreset->getDevice(ids[i]);

        if (device.isValid()) {
            dispatchDevice(L, device, false, 0);
        }
    }
}
