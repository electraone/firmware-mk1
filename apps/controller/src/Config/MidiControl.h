#pragma once

#include "MidiMessage.h"
#include "UiCommand.h"

/**
 * @brief One incoming MIDI message, and what the instrument does about it.
 *
 * What used to be a short list of named events is now any command the
 * instrument has - see Commands.inc. The nine names the configuration could
 * carry before are all commands, so a configuration written for the old list
 * still reads.
 */
struct MidiControl {
    MidiControl(UiCommand::Type newCommand,
                int16_t newParameter,
                MidiMessage::Type newMidiMessageType,
                uint8_t newParameterNumber)
        : midiMessageType(newMidiMessageType),
          parameterNumber(newParameterNumber),
          command(newCommand),
          parameter(newParameter)
    {
    }

    MidiMessage::Type midiMessageType;
    uint8_t parameterNumber;

    UiCommand::Type command;
    int16_t parameter;
};
