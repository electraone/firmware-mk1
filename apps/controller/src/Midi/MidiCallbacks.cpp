#include "MidiCallbacks.h"
#include "luaMidi.h"
#include "luaIntegration.h"
#include "System.h"

void assignLuaCallbacks(void)
{
    if (luaLE_functionExists("midi", "onClock")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onClock");
        MidiInputCallback::onMidiClockCallback = &onMidiClock;
    }

    if (luaLE_functionExists("midi", "onStart")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onStart");
        MidiInputCallback::onMidiStartCallback = &onMidiStart;
    }

    if (luaLE_functionExists("midi", "onStop")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onStop");
        MidiInputCallback::onMidiStopCallback = &onMidiStop;
    }

    if (luaLE_functionExists("midi", "onContinue")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onContinue");
        MidiInputCallback::onMidiContinueCallback = &onMidiContinue;
    }

    if (luaLE_functionExists("midi", "onActiveSensing")) {
        System::logger.write(LOG_ERROR,
                             "lua callback assigned: onActiveSensing");
        MidiInputCallback::onMidiActiveSensingCallback = &onMidiActiveSensing;
    }

    if (luaLE_functionExists("midi", "onSystemReset")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onSystemReset");
        MidiInputCallback::onMidiSystemResetCallback = &onMidiSystemReset;
    }

    if (luaLE_functionExists("midi", "onTuneRequest")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onTuneRequest");
        MidiInputCallback::onMidiTuneRequestCallback = &onMidiTuneRequest;
    }

    if (luaLE_functionExists("midi", "onProgramChange")) {
        System::logger.write(LOG_ERROR,
                             "lua callback assigned: onProgramChange");
        MidiInputCallback::onMidiProgramChangeCallback = &onMidiProgramChange;
    }

    if (luaLE_functionExists("midi", "onAfterTouchChannel")) {
        System::logger.write(LOG_ERROR,
                             "lua callback assigned: onAfterTouchChannel");
        MidiInputCallback::onMidiAfterTouchChannelCallback =
            &onMidiAfterTouchChannel;
    }

    if (luaLE_functionExists("midi", "onPitchBend")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onPitchBend");
        MidiInputCallback::onMidiPitchBendCallback = &onMidiPitchBend;
    }

    if (luaLE_functionExists("midi", "onSongSelect")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onSongSelect");
        MidiInputCallback::onMidiSongSelectCallback = &onMidiSongSelect;
    }

    if (luaLE_functionExists("midi", "onSongPosition")) {
        System::logger.write(LOG_ERROR,
                             "lua callback assigned: onSongPosition");
        MidiInputCallback::onMidiSongPositionCallback = &onMidiSongPosition;
    }

    if (luaLE_functionExists("midi", "onControlChange")) {
        System::logger.write(LOG_ERROR,
                             "lua callback assigned: onControlChange");
        MidiInputCallback::onMidiControlChangeCallback = &onMidiControlChange;
    }

    if (luaLE_functionExists("midi", "onNoteOn")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onNoteOn");
        MidiInputCallback::onMidiNoteOnCallback = &onMidiNoteOn;
    }

    if (luaLE_functionExists("midi", "onNoteOff")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onNoteOff");
        MidiInputCallback::onMidiNoteOffCallback = &onMidiNoteOff;
    }

    if (luaLE_functionExists("midi", "onAfterTouchPoly")) {
        System::logger.write(LOG_ERROR,
                             "lua callback assigned: onAfterTouchPoly");
        MidiInputCallback::onMidiAfterTouchPolyCallback = &onMidiAfterTouchPoly;
    }

    if (luaLE_functionExists("midi", "onSysex")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onSysex");
        MidiInputCallback::onMidiSysexCallback = &onMidiSysex;
    }

    if (luaLE_functionExists("midi", "onMessage")) {
        System::logger.write(LOG_ERROR, "lua callback assigned: onMessage");
        MidiInputCallback::onMidiMessageCallback = &onMidiMessage;
    }
}

void resetMidiCallbacks(void)
{
    MidiInputCallback::onMidiClockCallback = nullptr;
    MidiInputCallback::onMidiStartCallback = nullptr;
    MidiInputCallback::onMidiStopCallback = nullptr;
    MidiInputCallback::onMidiContinueCallback = nullptr;
    MidiInputCallback::onMidiActiveSensingCallback = nullptr;
    MidiInputCallback::onMidiSystemResetCallback = nullptr;
    MidiInputCallback::onMidiTuneRequestCallback = nullptr;
    MidiInputCallback::onMidiProgramChangeCallback = nullptr;
    MidiInputCallback::onMidiAfterTouchChannelCallback = nullptr;
    MidiInputCallback::onMidiPitchBendCallback = nullptr;
    MidiInputCallback::onMidiSongSelectCallback = nullptr;
    MidiInputCallback::onMidiSongPositionCallback = nullptr;
    MidiInputCallback::onMidiControlChangeCallback = nullptr;
    MidiInputCallback::onMidiNoteOnCallback = nullptr;
    MidiInputCallback::onMidiNoteOffCallback = nullptr;
    MidiInputCallback::onMidiAfterTouchPolyCallback = nullptr;
    MidiInputCallback::onMidiSysexCallback = nullptr;
    MidiInputCallback::onMidiMessageCallback = nullptr;
}

/*
 * Optional callbacks
 */
void onMidiClock(MidiInput midiInput)
{
    midi_onSingleByte(L, "midi", "onClock", midiInput);
}

void onMidiStart(MidiInput midiInput)
{
    midi_onSingleByte(L, "midi", "onStart", midiInput);
}

void onMidiStop(MidiInput midiInput)
{
    midi_onSingleByte(L, "midi", "onStop", midiInput);
}

void onMidiContinue(MidiInput midiInput)
{
    midi_onSingleByte(L, "midi", "onContinue", midiInput);
}

void onMidiActiveSensing(MidiInput midiInput)
{
    midi_onSingleByte(L, "midi", "onActiveSensing", midiInput);
}

void onMidiSystemReset(MidiInput midiInput)
{
    midi_onSingleByte(L, "midi", "onSystemReset", midiInput);
}

void onMidiTuneRequest(MidiInput midiInput)
{
    midi_onSingleByte(L, "midi", "onTuneRequest", midiInput);
}

void onMidiProgramChange(MidiInput midiInput,
                         uint8_t channel,
                         uint8_t programNumber)
{
    midi_onTwoBytesWithChannel(
        L, "midi", "onProgramChange", midiInput, channel, programNumber);
}

void onMidiAfterTouchChannel(MidiInput midiInput,
                             uint8_t channel,
                             uint8_t pressure)
{
    midi_onTwoBytesWithChannel(
        L, "midi", "onAfterTouchChannel", midiInput, channel, pressure);
}

void onMidiSongSelect(MidiInput midiInput, uint8_t songNumber)
{
    midi_onTwoBytes(L, "midi", "onSongSelect", midiInput, songNumber);
}

void onMidiPitchBend(MidiInput midiInput, uint8_t channel, int value)
{
    midi_onTwoBytesWithChannel(
        L, "midi", "onPitchBend", midiInput, channel, value);
}

void onMidiSongPosition(MidiInput midiInput, int position)
{
    midi_onTwoBytes(L, "midi", "onSongPosition", midiInput, position);
}

void onMidiControlChange(MidiInput midiInput,
                         uint8_t channel,
                         uint8_t controllerNumber,
                         uint8_t value)
{
    midi_onThreeBytesWithChannel(L,
                                 "midi",
                                 "onControlChange",
                                 midiInput,
                                 channel,
                                 controllerNumber,
                                 value);
}

void onMidiNoteOn(MidiInput midiInput,
                  uint8_t channel,
                  uint8_t noteNumber,
                  uint8_t velocity)
{
    midi_onThreeBytesWithChannel(
        L, "midi", "onNoteOn", midiInput, channel, noteNumber, velocity);
}

void onMidiNoteOff(MidiInput midiInput,
                   uint8_t channel,
                   uint8_t noteNumber,
                   uint8_t velocity)
{
    midi_onThreeBytesWithChannel(
        L, "midi", "onNoteOff", midiInput, channel, noteNumber, velocity);
}

void onMidiAfterTouchPoly(MidiInput midiInput,
                          uint8_t channel,
                          uint8_t noteNumber,
                          uint8_t pressure)
{
    midi_onThreeBytesWithChannel(L,
                                 "midi",
                                 "onAfterTouchPoly",
                                 midiInput,
                                 channel,
                                 noteNumber,
                                 pressure);
}

void onMidiSysex(MidiInput &midiInput, MidiMessage &midiMessage)
{
    luaLE_getModuleFunction("midi", "onSysex");

    if (lua_isfunction(L, -1)) {
        SysexBlock sysexBlock = midiMessage.getSysExBlock();
        lua_newtable(L);
        luaLE_pushTableInteger(
            L, "interface", (uint8_t)midiInput.getInterfaceType());
        luaLE_pushTableInteger(L, "port", midiInput.getPort());
        luaLE_pushObject(L, "SysexBlock", &sysexBlock);

        if (lua_pcall(L, 2, 0, 0) != 0) {
            System::logger.write(LOG_ERROR,
                                 "error running function 'onSysex': %s",
                                 lua_tostring(L, -1));
        }
    }
}

void onMidiMessage(MidiInput &midiInput, MidiMessage &midiMessage)
{
    luaLE_getModuleFunction("midi", "onMessage");

    if (lua_isfunction(L, -1)) {
        if (midiMessage.isSysEx()) {
            SysexBlock sysexBlock = midiMessage.getSysExBlock();
            MidiMessage::Type type = MidiMessage::Type::SystemExclusive;

            lua_newtable(L);
            luaLE_pushTableInteger(
                L, "interface", (uint8_t)midiInput.getInterfaceType());
            luaLE_pushTableInteger(L, "port", midiInput.getPort());

            lua_newtable(L);
            luaLE_pushTableInteger(L, "type", (uint8_t)type);
            luaLE_pushTableObject(L, "sysexBlock", "SysexBlock", &sysexBlock);

            if (lua_pcall(L, 2, 0, 0) != 0) {
                System::logger.write(
                    LOG_ERROR,
                    "error running function 'midi.onMessage': %s",
                    lua_tostring(L, -1));
            }
        } else {
            lua_newtable(L);
            luaLE_pushTableInteger(
                L, "interface", (uint8_t)midiInput.getInterfaceType());
            luaLE_pushTableInteger(L, "port", midiInput.getPort());

            MidiMessage::Type type = midiMessage.getType();

            lua_newtable(L);
            luaLE_pushTableInteger(L, "channel", midiMessage.getChannel());
            luaLE_pushTableInteger(L, "type", (uint8_t)type);
            luaLE_pushTableInteger(L, "data1", midiMessage.getData1());
            luaLE_pushTableInteger(L, "data2", midiMessage.getData2());

            if (type == MidiMessage::Type::ControlChange) {
                luaLE_pushTableInteger(
                    L, "controllerNumber", midiMessage.getData1());
                luaLE_pushTableInteger(L, "value", midiMessage.getData2());
            } else if (type == MidiMessage::Type::NoteOn) {
                luaLE_pushTableInteger(L, "noteNumber", midiMessage.getData1());
                luaLE_pushTableInteger(L, "velocity", midiMessage.getData2());
            } else if (type == MidiMessage::Type::NoteOff) {
                luaLE_pushTableInteger(L, "noteNumber", midiMessage.getData1());
                luaLE_pushTableInteger(L, "velocity", midiMessage.getData2());
            } else if (type == MidiMessage::Type::AfterTouchPoly) {
                luaLE_pushTableInteger(L, "noteNumber", midiMessage.getData1());
                luaLE_pushTableInteger(L, "pressure", midiMessage.getData2());
            } else if (type == MidiMessage::Type::ProgramChange) {
                luaLE_pushTableInteger(
                    L, "programNumber", midiMessage.getData1());
            } else if (type == MidiMessage::Type::SongSelect) {
                luaLE_pushTableInteger(L, "songNumber", midiMessage.getData1());
            } else if (type == MidiMessage::Type::AfterTouchChannel) {
                luaLE_pushTableInteger(L, "pressure", midiMessage.getData1());
            } else if (type == MidiMessage::Type::PitchBend) {
                luaLE_pushTableInteger(L,
                                       "value",
                                       midiMessage.getData1()
                                           | midiMessage.getData2() << 7);
            } else if (type == MidiMessage::Type::SongPosition) {
                luaLE_pushTableInteger(L,
                                       "position",
                                       midiMessage.getData1()
                                           | midiMessage.getData2() << 7);
            }

            if (lua_pcall(L, 2, 0, 0) != 0) {
                System::logger.write(
                    LOG_ERROR,
                    "error running function 'midi.onMessage': %s",
                    lua_tostring(L, -1));
            }
        }
    }
}
