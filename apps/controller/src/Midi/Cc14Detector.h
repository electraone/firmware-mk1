#pragma once

#include <stdint.h>

struct MidiCc14Message {
    int deviceId;
    uint16_t parameterNumber;
    uint16_t value;
    bool isCc14;
};

class Cc14Detector
{
public:
    Cc14Detector();

    void reset(void);
    bool parseControllerMessage(int deviceId,
                                uint8_t controllerNumber,
                                uint8_t controllerValue,
                                MidiCc14Message &result);

private:
    struct DeviceState {
        DeviceState(void);
        bool handleController(int deviceId,
                              uint8_t controllerNumber,
                              uint8_t value,
                              MidiCc14Message &);
        void resetValue(void);
        bool sendIfReady(int deviceId, MidiCc14Message &);

        uint8_t parameter;
        uint8_t valueMSB;
        uint8_t valueLSB;
        bool isCc14;
        bool expectingLSB;
    };

    DeviceState devices[33]; // 32 + 1 for midiLearn
};
