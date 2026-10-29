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
 * @file luaSchedule.h
 *
 * @brief Functions a preset wants run later, or when a chord is played.
 *
 *     schedule.after(ms, fn, ...)          -- once, ms from now
 *     schedule.every(ms, fn, ...)          -- every ms, from ms from now
 *     schedule.cancel(handle)              -- true when it was waiting
 *     schedule.now()                       -- millis()
 *     schedule.stats()                     -- pending, ran, errors, ...
 *     schedule.whenNotes(notes, fn[, options])
 *
 * The mk2's module, with the mk2's names, arguments and answers. timer.onTick
 * stays exactly as it is; this is for everything a preset used to multiplex
 * into it by hand. The functions run in the main loop, a millisecond task
 * between the others, so a due one can wait behind a repaint or a SysEx
 * message for a few milliseconds. Unlike the mk2, nothing stops one that never
 * returns - as nothing stops a timer.onTick that never returns.
 */

#pragma once

#include "luaIntegration.h"
#include "MidiInterface.h"
#include "MidiMessage.h"

int luaopen_schedule(lua_State *L);

int schedule_after(lua_State *L);
int schedule_every(lua_State *L);
int schedule_cancel(lua_State *L);
int schedule_now(lua_State *L);
int schedule_stats(lua_State *L);
int schedule_whenNotes(lua_State *L);

static const luaL_Reg schedule_functions[] = { { "after", schedule_after },
                                               { "every", schedule_every },
                                               { "cancel", schedule_cancel },
                                               { "now", schedule_now },
                                               { "stats", schedule_stats },
                                               { "whenNotes",
                                                 schedule_whenNotes },
                                               { NULL, NULL } };

/** @brief Runs what has fallen due. The millisecond task calls it. */
void schedule_run(lua_State *L, uint32_t now);

/**
 * @brief Drops everything a preset scheduled or waits for, releasing the
 *        references. Before its Lua state is closed.
 */
void schedule_reset(lua_State *L);

/**
 * @brief One incoming MIDI message, for the held notes and whatever
 *        schedule.whenNotes() waits on. processMidi() calls it for every
 *        message, preset or none, so the held notes are always current.
 */
void schedule_onMidiMessage(MidiInput midiInput,
                            uint8_t channel,
                            MidiMessage::Type type,
                            uint8_t data1,
                            uint8_t data2);
