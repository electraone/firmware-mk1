#pragma once

#include "Config/Config.h"
#include "CommandManager.h"
#include "MainDelegate.h"

class MidiApi
{
public:
    MidiApi(const MidiControls &newMidiControls,
            MainDelegate &newDelegate,
            CommandManager &newCommandManager);
    virtual ~MidiApi() = default;

    void process(const MidiMessage &midiMessage);

private:
    void processBankSelect(uint8_t bankNumber);
    void processProgramChange(uint8_t programNumber);
    bool processMidiControl(MidiMessage::Type type, uint8_t data1);

    enum ProgramChangeMode { Presets, Snapshots };

    const MidiControls &midiControls;
    MainDelegate &delegate;
    CommandManager &commandManager;
    ProgramChangeMode programChangeMode;
};
