#pragma once

#include "MidiInterface.h"
#include "MidiMessage.h"
#include "Preset.h"
#include "Message.h"
#include "Cc14Detector.h"
#include "RpnDetector.h"

class MidiLearn
{
public:
    MidiLearn(const Preset &newPreset) : preset(newPreset)
    {
    }

    virtual ~MidiLearn() = default;

    void process(const MidiInput &midiInput, const MidiMessage &midiMessage);

private:
    const Preset &preset;
    RpnDetector rpnDetector;
    Cc14Detector cc14Detector;

    static constexpr uint8_t MidiLearnDeviceId = 32;
};
