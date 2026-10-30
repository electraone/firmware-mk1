#pragma once

#include "MidiInputCallback.h"

// Optional callbacks
void onMidiClock(MidiInput midiInput);
void onMidiStart(MidiInput midiInput);
void onMidiStop(MidiInput midiInput);
void onMidiContinue(MidiInput midiInput);
void onMidiActiveSensing(MidiInput midiInput);
void onMidiSystemReset(MidiInput midiInput);
void onMidiTuneRequest(MidiInput midiInput);
void onMidiProgramChange(MidiInput midiInput,
                         uint8_t channel,
                         uint8_t programNumber);
void onMidiAfterTouchChannel(MidiInput midiInput,
                             uint8_t channel,
                             uint8_t pressure);
void onMidiSongSelect(MidiInput midiInput, uint8_t songNumber);
void onMidiPitchBend(MidiInput midiInput, uint8_t channel, int value);
void onMidiSongPosition(MidiInput midiInput, int position);
void onMidiControlChange(MidiInput midiInput,
                         uint8_t channel,
                         uint8_t controllerNumber,
                         uint8_t value);
void onMidiNoteOn(MidiInput midiInput,
                  uint8_t channel,
                  uint8_t noteNumber,
                  uint8_t velocity);
void onMidiNoteOff(MidiInput midiInput,
                   uint8_t channel,
                   uint8_t noteNumber,
                   uint8_t velocity);
void onMidiAfterTouchPoly(MidiInput midiInput,
                          uint8_t channel,
                          uint8_t noteNumber,
                          uint8_t pressure);
void onMidiSysex(MidiInput &midiInput, MidiMessage &midiMessage);
void onMidiMessage(MidiInput &midiInput, MidiMessage &midiMessage);

void assignLuaCallbacks(void);
void resetMidiCallbacks(void);
