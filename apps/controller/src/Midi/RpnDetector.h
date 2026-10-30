#pragma once

#include <stdint.h>

struct MidiRpnMessage {
    int deviceId;
    uint16_t parameterNumber;
    uint16_t value;
    bool isNrpn;
    bool is14BitValue;
};

class RpnDetector
{
public:
    RpnDetector();

    void reset();
    bool parseControllerMessage(int deviceId,
                                uint8_t controllerNumber,
                                uint8_t controllerValue,
                                MidiRpnMessage &result);

private:
    struct DeviceState {
        DeviceState();
        bool handleController(int deviceId,
                              uint8_t controllerNumber,
                              uint8_t value,
                              MidiRpnMessage &);
        void resetValue();
        bool sendIfReady(int deviceId, MidiRpnMessage &);

        uint8_t parameterMSB;
        uint8_t parameterLSB;
        uint8_t valueMSB;
        uint8_t valueLSB;
        bool isNrpn;
    };

    DeviceState devices[33]; // 32 + 1 for midiLearn
};
