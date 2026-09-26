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

#include "luaMidi.h"
#include "MidiOutput.h"
#include "System.h"
#include "luaExtensionBase.h"

int luaopen_midi(lua_State *L)
{
    luaL_newlib(L, midi_functions);
    return 1;
}

/**
 * sends a MidiMessage out
 * @params a midiMessage table
 */
int midi_sendMessage(lua_State *L)
{
    MidiInterface::Type interface =
        static_cast<MidiInterface::Type>(LUA_MIDI_INTERFACE_MIDI_ALL);

    int portIndex = 1;
    int tableIndex = 2;

    if (lua_gettop(L) == 3) { // if number of parameters is 3
        interface = static_cast<MidiInterface::Type>(luaL_checkinteger(L, 1));
        portIndex = 2;
        tableIndex = 3;
    }

    int port = luaL_checkinteger(L, portIndex);

    // get the message table
    luaL_checktype(L, tableIndex, LUA_TTABLE);

    // parse type
    lua_getfield(L, tableIndex, "type");
    int type = luaL_checkinteger(L, -1);

    // process type specific parts
    if (type == (uint8_t)MidiMessage::Type::ControlChange) {
        lua_getfield(L, tableIndex, "channel");
        lua_getfield(L, tableIndex, "controllerNumber");
        lua_getfield(L, tableIndex, "value");

        int channel = luaL_checkinteger(L, -3);
        int data1 = luaL_checkinteger(L, -2);
        int data2 = luaL_checkinteger(L, -1);

        MidiOutput::sendControlChange(interface, port, channel, data1, data2);
    } else if (type == (uint8_t)MidiMessage::Type::ProgramChange) {
        lua_getfield(L, tableIndex, "channel");
        lua_getfield(L, tableIndex, "programNumber");

        int channel = luaL_checkinteger(L, -2);
        int data1 = luaL_checkinteger(L, -1);

        MidiOutput::sendProgramChange(interface, port, channel, data1);
    } else if (type == (uint8_t)MidiMessage::Type::NoteOn) {
        lua_getfield(L, tableIndex, "channel");
        lua_getfield(L, tableIndex, "noteNumber");
        lua_getfield(L, tableIndex, "velocity");

        int channel = luaL_checkinteger(L, -3);
        int data1 = luaL_checkinteger(L, -2);
        int data2 = luaL_checkinteger(L, -1);

        MidiOutput::sendNoteOn(interface, port, channel, data1, data2);
    } else if (type == (uint8_t)MidiMessage::Type::NoteOff) {
        lua_getfield(L, tableIndex, "channel");
        lua_getfield(L, tableIndex, "noteNumber");
        lua_getfield(L, tableIndex, "velocity");

        int channel = luaL_checkinteger(L, -3);
        int data1 = luaL_checkinteger(L, -2);
        int data2 = luaL_checkinteger(L, -1);

        MidiOutput::sendNoteOff(interface, port, channel, data1, data2);
    } else if (type == (uint8_t)MidiMessage::Type::PitchBend) {
        lua_getfield(L, tableIndex, "channel");
        lua_getfield(L, tableIndex, "value");

        int channel = luaL_checkinteger(L, -2);
        int value = luaL_checkinteger(L, -1);

        MidiOutput::sendPitchBend(interface, port, channel, value);
    } else if (type == (uint8_t)MidiMessage::Type::AfterTouchPoly) {
        lua_getfield(L, tableIndex, "channel");
        lua_getfield(L, tableIndex, "noteNumber");
        lua_getfield(L, tableIndex, "pressure");

        int channel = luaL_checkinteger(L, -3);
        int data1 = luaL_checkinteger(L, -2);
        int data2 = luaL_checkinteger(L, -1);

        MidiOutput::sendAfterTouchPoly(interface, port, channel, data1, data2);
    } else if (type == (uint8_t)MidiMessage::Type::AfterTouchChannel) {
        lua_getfield(L, tableIndex, "channel");
        lua_getfield(L, tableIndex, "pressure");

        int channel = luaL_checkinteger(L, -2);
        int data1 = luaL_checkinteger(L, -1);

        MidiOutput::sendAfterTouchChannel(interface, port, channel, data1);
    } else if (type == (uint8_t)MidiMessage::Type::Clock) {
        MidiOutput::sendClock(interface, port);
    } else if (type == (uint8_t)MidiMessage::Type::Start) {
        MidiOutput::sendStart(interface, port);
    } else if (type == (uint8_t)MidiMessage::Type::Stop) {
        MidiOutput::sendStop(interface, port);
    } else if (type == (uint8_t)MidiMessage::Type::Continue) {
        MidiOutput::sendContinue(interface, port);
    } else if (type == (uint8_t)MidiMessage::Type::ActiveSensing) {
        MidiOutput::sendActiveSensing(interface, port);
    } else if (type == (uint8_t)MidiMessage::Type::SystemReset) {
        MidiOutput::sendSystemReset(interface, port);
    } else if (type == (uint8_t)MidiMessage::Type::SongSelect) {
        lua_getfield(L, tableIndex, "songNumber");
        int songNumber = luaL_checkinteger(L, -1);

        MidiOutput::sendSongSelect(interface, port, songNumber);
    } else if (type == (uint8_t)MidiMessage::Type::SongPosition) {
        lua_getfield(L, tableIndex, "position");
        int position = luaL_checkinteger(L, -1);

        MidiOutput::sendSongPosition(interface, port, position);
    } else if (type == (uint8_t)MidiMessage::Type::TuneRequest) {
        MidiOutput::sendTuneRequest(interface, port);
    } else {
        return (luaL_error(L, "type %d is not supported", type));
    }

    return (0);
}

/**
 * sends out a Continuous Controller MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param parameterNumber a numeric identifier of the continuous controller
 * @param value a numeric value to send
 * @returns status and error code if present
 */
int midi_sendControlChange(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 4);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int controllerNumber = luaL_checkinteger(L, nextIndex++);
    int value = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check7bit(controllerNumber, "controllerNumber");
    check7bit(value, "value");

    MidiOutput::sendControlChange(static_cast<MidiInterface::Type>(interface),
                                  port,
                                  channel,
                                  controllerNumber,
                                  value);

    return (0);
}

/**
 * sends out a Note On MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param noteNumber a numeric identifier of the MIDI note
 * @param velocity a velocity of the note
 * @returns status and error code if present
 */
int midi_sendNoteOn(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 4);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int noteNumber = luaL_checkinteger(L, nextIndex++);
    int velocity = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check7bit(noteNumber, "noteNumber");
    check7bit(velocity, "velocity");

    MidiOutput::sendNoteOn(static_cast<MidiInterface::Type>(interface),
                           port,
                           channel,
                           noteNumber,
                           velocity);

    return (0);
}

/**
 * sends out a Note Off MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param noteNumber a numeric identifier of the MIDI note
 * @param velocity a velocity of the note
 * @returns status and error code if present
 */
int midi_sendNoteOff(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 4);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int noteNumber = luaL_checkinteger(L, nextIndex++);
    int velocity = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check7bit(noteNumber, "noteNumber");
    check7bit(velocity, "velocity");

    MidiOutput::sendNoteOff(static_cast<MidiInterface::Type>(interface),
                            port,
                            channel,
                            noteNumber,
                            velocity);

    return (0);
}

/**
 * sends out a Pitch Bend MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param value a rate of the pitch bend applied
 * @returns status and error code if present
 */
int midi_sendPitchBend(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 3);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int value = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    checkPitchBend(value, "value");

    MidiOutput::sendPitchBend(static_cast<MidiInterface::Type>(interface),
                              port,
                              channel,
                              value + 8191);

    return (0);
}

/**
 * sends out a Polyphonic Aftertouch MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param noteNumber a numeric identifier of the MIDI note
 * @param pressure a velocity of the note
 * @returns status and error code if present
 */
int midi_sendAfterTouchPoly(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 4);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int noteNumber = luaL_checkinteger(L, nextIndex++);
    int pressure = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check7bit(noteNumber, "noteNumber");
    check7bit(pressure, "pressure");

    MidiOutput::sendAfterTouchPoly(static_cast<MidiInterface::Type>(interface),
                                   port,
                                   channel,
                                   noteNumber,
                                   pressure);

    return (0);
}

/**
 * sends out a Channel Aftertouch MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param pressure a velocity of the note
 * @returns status and error code if present
 */
int midi_sendAfterTouchChannel(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 3);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int pressure = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check7bit(pressure, "pressure");

    MidiOutput::sendAfterTouchChannel(
        static_cast<MidiInterface::Type>(interface), port, channel, pressure);

    return (0);
}

/**
 * sends out a MIDI RealTime Clock message
 * @param port an identifier of the port number
 * @returns status and error code if present
 */
int midi_sendClock(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 1);
    int port = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);

    MidiOutput::sendClock(static_cast<MidiInterface::Type>(interface), port);

    return (0);
}

/**
 * sends out a MIDI RealTime Start message
 * @param port an identifier of the port number
 * @returns status and error code if present
 */
int midi_sendStart(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 1);
    int port = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);

    MidiOutput::sendStart(static_cast<MidiInterface::Type>(interface), port);

    return (0);
}

/**
 * sends out a MIDI RealTime Stop message
 * @param port an identifier of the port number
 * @returns status and error code if present
 */
int midi_sendStop(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 1);
    int port = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);

    MidiOutput::sendStop(static_cast<MidiInterface::Type>(interface), port);

    return (0);
}

/**
 * sends out a MIDI RealTime Continue message
 * @param port an identifier of the port number
 * @returns status and error code if present
 */
int midi_sendContinue(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 1);
    int port = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);

    MidiOutput::sendContinue(static_cast<MidiInterface::Type>(interface), port);

    return (0);
}

/**
 * sends out a MIDI RealTime Active Sensing message
 * @param port an identifier of the port number
 * @returns status and error code if present
 */
int midi_sendActiveSensing(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 1);
    int port = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);

    MidiOutput::sendActiveSensing(static_cast<MidiInterface::Type>(interface),
                                  port);

    return (0);
}

/**
 * sends out a MIDI RealTime System Reset message
 * @param port an identifier of the port number
 * @returns status and error code if present
 */
int midi_sendSystemReset(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 1);
    int port = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);

    MidiOutput::sendSystemReset(static_cast<MidiInterface::Type>(interface),
                                port);

    return (0);
}

/**
 * sends out a MIDI Song Select message
 * @param port an identifier of the port number
 * @param songNumber an identifier of the song
 * @returns status and error code if present
 */
int midi_sendSongSelect(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 2);
    int port = luaL_checkinteger(L, nextIndex++);
    int song = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    check7bit(song, "song");

    MidiOutput::sendSongSelect(
        static_cast<MidiInterface::Type>(interface), port, song);

    return (0);
}

/**
 * sends out a MIDI Song Select message
 * @param port an identifier of the port number
 * @param songNumber an identifier of the song
 * @returns status and error code if present
 */
int midi_sendSongPosition(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 2);
    int port = luaL_checkinteger(L, nextIndex++);
    int position = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    check14bit(position, "position");

    MidiOutput::sendSongPosition(
        static_cast<MidiInterface::Type>(interface), port, position);

    return (0);
}

/**
 * sends out a MIDI Tune Request message
 * @param port an identifier of the port number
 * @returns status and error code if present
 */
int midi_sendTuneRequest(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 1);
    int port = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);

    MidiOutput::sendTuneRequest(static_cast<MidiInterface::Type>(interface),
                                port);

    return (0);
}

/**
 * sends out a Program Change MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param programNumber a numeric identifier of the MIDI program
 * @returns status and error code if present
 */
int midi_sendProgramChange(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 3);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int programNumber = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check7bit(programNumber, "programNumber");

    MidiOutput::sendProgramChange(static_cast<MidiInterface::Type>(interface),
                                  port,
                                  channel,
                                  programNumber);

    return (0);
}

/**
 * @brief midi.sendSysex([interface,] port, data)
 *
 * The data is either an array of bytes, without the leading 0xF0 and the
 * trailing 0xF7, or a SysexBlock - the object a script is handed by
 * midi.onSysex() and patch.onResponse(), which is what makes answering a
 * device's response with a message derived from it possible without copying
 * the whole thing through a Lua table.
 */
int midi_sendSysEx(lua_State *L)
{
    int interface = LUA_MIDI_INTERFACE_MIDI_ALL;
    int portIndex = 1;
    int dataIndex = 2;

    if (lua_gettop(L) == 3) { // if number of parameters is 3
        interface = luaL_checkinteger(L, 1);
        portIndex = 2;
        dataIndex = 3;
    }

    int port = luaL_checkinteger(L, portIndex);

    checkInterface(interface);
    checkPort(port);

    if (void *block = luaL_testudata(L, dataIndex, "SysexBlock")) {
        SysexBlock *sysexBlock = *reinterpret_cast<SysexBlock **>(block);

        if (sysexBlock == nullptr) {
            return (luaL_error(L, "sendSysex: invalid SysexBlock"));
        }

        MidiOutput::sendSysEx(
            static_cast<MidiInterface::Type>(interface), port, *sysexBlock);
        return (0);
    }

    luaL_checktype(L, dataIndex, LUA_TTABLE);

    /* expects a true array of integers, with no keys and no gaps */
    const int length = (int)lua_rawlen(L, dataIndex);

    /* One byte more than a chunk: a message whose last data byte exactly
     * fills a chunk still has the end byte appended to it rather than sent in
     * a packet of its own. */
    uint8_t buffer[LUA_MAX_SYSEX_SIZE + 1];

    /* The SysEx start byte, then as much of the message as fits, then the end
     * byte: the chunks in between carry data only. */
    buffer[0] = 0xf0;

    int bufferIndex = 1;

    for (int item = 1; item <= length; item++) {
        lua_rawgeti(L, dataIndex, item);

        /* Converted rather than just tested for being a number: a fraction is
         * a number and is not a byte, and lua_tointeger() would have turned it
         * into a silent zero in the middle of the message. */
        int isByte = 0;
        const lua_Integer byte = lua_tointegerx(L, -1, &isByte);

        lua_pop(L, 1);

        if (isByte == 0) {
            /* The message is already on its way out, so it has to be closed
             * before the error is raised. Leaving it open locks up whatever is
             * listening until it gives up on the message. */
            buffer[bufferIndex++] = 0xf7;
            MidiOutput::sendSysExPartial(
                static_cast<MidiInterface::Type>(interface),
                port,
                buffer,
                bufferIndex,
                true);
            return (luaL_error(
                L, "sendSysex: data[%d] is not a whole number", item));
        }

        if ((byte < 0) || (byte > 127)) {
            buffer[bufferIndex++] = 0xf7;
            MidiOutput::sendSysExPartial(
                static_cast<MidiInterface::Type>(interface),
                port,
                buffer,
                bufferIndex,
                true);
            return (
                luaL_error(L,
                           "sendSysex: data[%d] is out of 7bit range: value=%d",
                           item,
                           (int)byte));
        }

        buffer[bufferIndex++] = (uint8_t)byte;

        /* Full, and there is more to come. A chunk that turns out to be the
         * last is held back so that the end byte goes out with it rather than
         * in a packet of its own. */
        if ((bufferIndex == LUA_MAX_SYSEX_SIZE) && (item < length)) {
            MidiOutput::sendSysExPartial(
                static_cast<MidiInterface::Type>(interface),
                port,
                buffer,
                LUA_MAX_SYSEX_SIZE,
                false);
            bufferIndex = 0;
        }
    }

    /* Room for the end byte is what the odd size of the buffer buys: a full
     * chunk is LUA_MAX_SYSEX_SIZE, and the byte that closes the message goes
     * after it. */
    buffer[bufferIndex++] = 0xf7;

    MidiOutput::sendSysExPartial(static_cast<MidiInterface::Type>(interface),
                                 port,
                                 buffer,
                                 bufferIndex,
                                 true);

    return (0);
}

/**
 * sends out a NRPN MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param parameterNumber a numeric identifier of the NPRN parameter
 * @param value a numeric value to send
 * @param lsbFirst a switch to swap an order of LSB and MSB bytes
 * @returns status and error code if present
 */
int midi_sendNrpn(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 4);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int parameterNumber = luaL_checkinteger(L, nextIndex++);
    int value = luaL_checkinteger(L, nextIndex++);
    bool lsbFirst = luaLE_optBoolean(L, nextIndex++, false);
    bool resetRpn = luaLE_optBoolean(L, nextIndex, true);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check14bit(parameterNumber, "parameterNumber");
    check14bit(value, "value");

    MidiOutput::sendNrpn(static_cast<MidiInterface::Type>(interface),
                         port,
                         channel,
                         parameterNumber,
                         value,
                         lsbFirst,
                         resetRpn);

    return (0);
}

/**
 * sends out a RPN MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param parameterNumber a numeric identifier of the PRN parameter
 * @param value a numeric value to send
 * @returns status and error code if present
 */
int midi_sendRpn(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 4);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int parameterNumber = luaL_checkinteger(L, nextIndex++);
    int value = luaL_checkinteger(L, nextIndex);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check14bit(parameterNumber, "parameterNumber");
    check14bit(value, "value");

    MidiOutput::sendRpn(static_cast<MidiInterface::Type>(interface),
                        port,
                        channel,
                        parameterNumber,
                        value);

    return (0);
}

/**
 * sends out a 14-bit Continuous Controller MIDI message
 * @param port an identifier of the port number
 * @param channel a MIDI channel
 * @param parameterNumber a numeric identifier of the CC14 parameter
 * @param value a numeric value to send
 * @param lsbFirst a switch to swap an order of LSB and MSB bytes
 * @returns status and error code if present
 */
int midi_sendControlChange14Bit(lua_State *L)
{
    int nextIndex = 0;
    int interface = getInterface(L, &nextIndex, 4);
    int port = luaL_checkinteger(L, nextIndex++);
    int channel = luaL_checkinteger(L, nextIndex++);
    int controllerNumber = luaL_checkinteger(L, nextIndex++);
    int value = luaL_checkinteger(L, nextIndex++);
    bool lsbFirst = luaLE_optBoolean(L, nextIndex, false);

    checkInterface(interface);
    checkPort(port);
    checkChannel(channel);
    check14bit(controllerNumber, "controllerNumber");
    check14bit(value, "value");

    MidiOutput::sendControlChange14Bit(
        static_cast<MidiInterface::Type>(interface),
        port,
        channel,
        controllerNumber,
        value,
        lsbFirst);

    return (0);
}

int midi_flush(lua_State *L)
{
    lua_settop(L, 0);
    System::tasks.flushMidi();

    return (0);
}

void midi_onSingleByte(lua_State *L,
                       const char *module,
                       const char *function,
                       MidiInput midiInput)
{
    LuaStackGuard guard(L);

    luaLE_getModuleFunction(L, module, function);

    if (lua_isfunction(L, -1)) {
        lua_newtable(L);
        luaLE_pushTableInteger(
            L, "interface", (uint8_t)midiInput.getInterfaceType());
        luaLE_pushTableInteger(L, "port", midiInput.getPort());

        if (lua_pcall(L, 1, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
}

void midi_onTwoBytes(lua_State *L,
                     const char *module,
                     const char *function,
                     MidiInput midiInput,
                     int data1)
{
    LuaStackGuard guard(L);

    luaLE_getModuleFunction(L, module, function);

    if (lua_isfunction(L, -1)) {
        lua_newtable(L);
        luaLE_pushTableInteger(
            L, "interface", (uint8_t)midiInput.getInterfaceType());
        luaLE_pushTableInteger(L, "port", midiInput.getPort());
        lua_pushnumber(L, data1);

        if (lua_pcall(L, 2, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
}

void midi_onTwoBytesWithChannel(lua_State *L,
                                const char *module,
                                const char *function,
                                MidiInput midiInput,
                                uint8_t channel,
                                int data1)
{
    LuaStackGuard guard(L);

    luaLE_getModuleFunction(L, module, function);

    if (lua_isfunction(L, -1)) {
        lua_newtable(L);
        luaLE_pushTableInteger(
            L, "interface", (uint8_t)midiInput.getInterfaceType());
        luaLE_pushTableInteger(L, "port", midiInput.getPort());
        lua_pushnumber(L, channel);
        lua_pushnumber(L, data1);

        if (lua_pcall(L, 3, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
}

void midi_onThreeBytesWithChannel(lua_State *L,
                                  const char *module,
                                  const char *function,
                                  MidiInput midiInput,
                                  uint8_t channel,
                                  uint8_t data1,
                                  uint8_t data2)
{
    LuaStackGuard guard(L);

    luaLE_getModuleFunction(L, module, function);

    if (lua_isfunction(L, -1)) {
        lua_newtable(L);
        luaLE_pushTableInteger(
            L, "interface", (uint8_t)midiInput.getInterfaceType());
        luaLE_pushTableInteger(L, "port", midiInput.getPort());
        lua_pushnumber(L, channel);
        lua_pushnumber(L, data1);
        lua_pushnumber(L, data2);

        if (lua_pcall(L, 4, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
}

void midi_onPitchBend(lua_State *L,
                      const char *module,
                      const char *function,
                      MidiInput midiInput,
                      uint8_t channel,
                      int data1)
{
    LuaStackGuard guard(L);

    luaLE_getModuleFunction(L, module, function);

    if (lua_isfunction(L, -1)) {
        lua_newtable(L);
        luaLE_pushTableInteger(
            L, "interface", (uint8_t)midiInput.getInterfaceType());
        luaLE_pushTableInteger(L, "port", midiInput.getPort());
        lua_pushnumber(L, channel);
        lua_pushnumber(L, data1 - 8191);

        if (lua_pcall(L, 3, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function '%s': %s",
                                 function,
                                 lua_tostring(L, -1));
        }
    }
}

void midi_onMidiSysex(lua_State *L,
                      MidiInput &midiInput,
                      SysexBlock &sysexBlock)
{
    LuaStackGuard guard(L);

    luaLE_getModuleFunction(L, "midi", "onSysex");

    if (lua_isfunction(L, -1)) {
        lua_newtable(L);
        luaLE_pushTableInteger(
            L, "interface", (uint8_t)midiInput.getInterfaceType());
        luaLE_pushTableInteger(L, "port", midiInput.getPort());
        luaLE_pushObject(L, "SysexBlock", &sysexBlock);

        if (lua_pcall(L, 2, 0, 0) != 0) {
            System::logger.write(LOG_LUA,
                                 "error running function 'onSysex': %s",
                                 lua_tostring(L, -1));
        }
    }
}

int getInterface(lua_State *L, int *nextIndex, int numMandatory)
{
    const int numParameters = lua_gettop(L);
    int interface = LUA_MIDI_INTERFACE_MIDI_ALL;

    *nextIndex = 1;

    if (numParameters <= numMandatory) {
        return (interface);
    }

    /* The one ambiguous shape: as many arguments as the interface form needs,
     * but the extra one is a flag rather than a port. Only sendNrpn() and
     * sendControlChange14Bit() have flags to be confused with. */
    if (lua_isboolean(L, numMandatory + 1)) {
        return (interface);
    }

    interface = luaL_checkinteger(L, 1);
    *nextIndex = 2;

    return (interface);
}
