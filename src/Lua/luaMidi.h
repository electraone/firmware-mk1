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
 * @file luaMidi.h
 *
 * @brief An implementation of E1 Lua MIDI library. 
 */

#pragma once

#include "MidiInterface.h"
#include "MidiMessage.h"
#include "helpers.h"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#define LUA_MAX_SYSEX_SIZE 60 // must be a multiply of 3
#define LUA_NR_OF_MIDI_INTERFACES 4 // includes MidiAll
#define LUA_NR_OF_MIDI_PORTS 3
#define LUA_MIDI_INTERFACE_MIDI_ALL 3

/*
 * midiMessage format
 *
 midiMessage = {
    channel = 16,
    type = "cc",
    data1 = 10,
    data2 = 20
  }
*/

/*
 * midiInput format
 *
 midiInput = {
    interface = "USB dev",
    port = 0
  }
*/

int luaopen_midi(lua_State *L);

int midi_sendMessage(lua_State *L);
int midi_sendControlChange(lua_State *L);
int midi_sendNoteOn(lua_State *L);
int midi_sendNoteOff(lua_State *L);
int midi_sendClock(lua_State *L);
int midi_sendStart(lua_State *L);
int midi_sendStop(lua_State *L);
int midi_sendContinue(lua_State *L);
int midi_sendTuneRequest(lua_State *L);
int midi_sendProgramChange(lua_State *L);
int midi_sendPitchBend(lua_State *L);
int midi_sendAfterTouchPoly(lua_State *L);
int midi_sendAfterTouchChannel(lua_State *L);
int midi_sendActiveSensing(lua_State *L);
int midi_sendSystemReset(lua_State *L);
int midi_sendSongSelect(lua_State *L);
int midi_sendSongPosition(lua_State *L);
int midi_sendSysEx(lua_State *L);
int midi_sendNrpn(lua_State *L);
int midi_sendRpn(lua_State *L);
int midi_sendControlChange14Bit(lua_State *L);
int midi_flush(lua_State *L);
void midi_onSingleByte(lua_State *L,
                       const char *module,
                       const char *function,
                       MidiInput midiInput);
void midi_onTwoBytes(lua_State *L,
                     const char *module,
                     const char *function,
                     MidiInput midiInput,
                     int data1);
void midi_onTwoBytesWithChannel(lua_State *L,
                                const char *module,
                                const char *function,
                                MidiInput midiInput,
                                uint8_t channel,
                                int data1);
void midi_onThreeBytesWithChannel(lua_State *L,
                                  const char *module,
                                  const char *function,
                                  MidiInput midiInput,
                                  uint8_t channel,
                                  uint8_t data1,
                                  uint8_t data2);
void midi_onPitchBend(lua_State *L,
                      const char *module,
                      const char *function,
                      MidiInput midiInput,
                      uint8_t channel,
                      int data1);
void midi_onMidiSysex(lua_State *L,
                      MidiInput &midiInput,
                      SysexBlock &sysexBlock);

static const luaL_Reg midi_functions[] = {
    { "sendMessage", midi_sendMessage },
    { "sendControlChange", midi_sendControlChange },
    { "sendNoteOn", midi_sendNoteOn },
    { "sendNoteOff", midi_sendNoteOff },
    { "sendClock", midi_sendClock },
    { "sendStart", midi_sendStart },
    { "sendStop", midi_sendStop },
    { "sendContinue", midi_sendContinue },
    { "sendTuneRequest", midi_sendTuneRequest },
    { "sendProgramChange", midi_sendProgramChange },
    { "sendPitchBend", midi_sendPitchBend },
    { "sendAfterTouchPoly", midi_sendAfterTouchPoly },
    { "sendAfterTouchChannel", midi_sendAfterTouchChannel },
    { "sendActiveSensing", midi_sendActiveSensing },
    { "sendSystemReset", midi_sendSystemReset },
    { "sendSongSelect", midi_sendSongSelect },
    { "sendSongPosition", midi_sendSongPosition },
    { "sendSysex", midi_sendSysEx },
    { "sendNrpn", midi_sendNrpn },
    { "sendRpn", midi_sendRpn },
    { "sendControlChange14Bit", midi_sendControlChange14Bit },
    /* The name the released documentation gives it. Both are kept: presets
     * written against either one have to load. */
    { "sendControlChange14", midi_sendControlChange14Bit },
    { "flush", midi_flush },
    { NULL, NULL }
};

#define checkInterface(interface)                                              \
    if ((interface < 0) || (interface > (LUA_NR_OF_MIDI_INTERFACES - 1))) {    \
        return (luaL_error(L, "interface %d is out of range", interface));     \
    }

#define checkPort(port)                                                        \
    if ((port < 0) || (port > (LUA_NR_OF_MIDI_PORTS - 1))) {                   \
        return (luaL_error(L, "port %d is out of range", port));               \
    }

#define checkChannel(channel)                                                  \
    if ((channel < 1) || (channel > 16)) {                                     \
        return (luaL_error(L, "channel %d is out of range", channel));         \
    }

#define check7bit(number, var)                                                 \
    if ((number < 0) || (number > 127)) {                                      \
        return (                                                               \
            luaL_error(L, "%s is out of 7bit range: value=%d", var, number));  \
    }

#define check14bit(number, var)                                                \
    if ((number < 0) || (number > 16383)) {                                    \
        return (                                                               \
            luaL_error(L, "%s is out of 14bit range: value=%d", var, number)); \
    }

#define checkPitchBend(number, var)                                            \
    if ((number < -8191) || (number > 8192)) {                                 \
        return (luaL_error(                                                    \
            L, "%s is out of pitchBend range: value=%d", var, number));        \
    }

/**
 * @brief Reads the optional leading interface argument.
 *
 * Every send function takes either (port, ...) or (interface, port, ...), and
 * which one it was given is decided by how many arguments arrived:
 * @p numMandatory is how many always follow the interface, so one more than
 * that means an interface came first.
 *
 * The count has to come from the caller because it differs per function -
 * midi.sendClock() has one, midi.sendControlChange() has four. It used to be
 * hardwired to five here, which is right for the four-argument messages and
 * wrong for every other one: midi.sendClock(PORT_1) read the port as an
 * interface, and midi.sendNrpn(PORT_1, 1, 100, 200) read its own port argument
 * back as resetRpn.
 *
 * @p nextIndex is left holding an absolute stack index - 1 or 2 - so that a
 * function reading an argument that was not given asks for an index past the
 * top, which luaL_optinteger() answers with the default. Walking backwards
 * from the top, as this used to, made that index 0 or 1 instead: never absent,
 * and sometimes an earlier argument read a second time.
 *
 * A trailing flag is what tells the one ambiguous case apart. Only the two
 * functions with optional flags can meet it, and only when the flag is given
 * as a boolean - which is what the argument's name asks for, and what
 * luaLE_optBoolean() now accepts.
 */
int getInterface(lua_State *L, int *nextIndex, int numMandatory);
