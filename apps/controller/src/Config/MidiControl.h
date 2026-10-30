#pragma once

#include "MidiMessage.h"
#include "Enums.h"

struct MidiControl {
    MidiControl(AppEventType newEventType,
                uint8_t newEventParameter,
                MidiMessage::Type newMidiMessageType,
                uint8_t newParameterNumber)
        : command(false),
          eventType(newEventType),
          eventParameter1(newEventParameter),
          eventParameter2(0),
          eventParameter3(0),
          midiMessageType(newMidiMessageType),
          parameterNumber(newParameterNumber)
    {
    }

    MidiControl(AppEventType newEventType,
                uint8_t newEventParameter1,
                uint8_t newEventParameter2,
                MidiMessage::Type newMidiMessageType,
                uint8_t newParameterNumber)
        : command(true),
          eventType(newEventType),
          eventParameter1(newEventParameter1),
          eventParameter2(newEventParameter2),
          eventParameter3(0),
          midiMessageType(newMidiMessageType),
          parameterNumber(newParameterNumber)
    {
    }

    MidiMessage::Type midiMessageType;
    uint8_t parameterNumber;

    bool command;
    AppEventType eventType;
    uint8_t eventParameter1;
    uint8_t eventParameter2;
    uint8_t eventParameter3;
};
