#include "Cc14Detector.h"
#include <cassert>

Cc14Detector::Cc14Detector()
{
}

bool Cc14Detector::parseControllerMessage(int deviceId,
                                          uint8_t controllerNumber,
                                          uint8_t controllerValue,
                                          MidiCc14Message &result)
{
    if (!((0 <= deviceId) && (deviceId <= 32))) {
        return (false);
    }
    return (devices[deviceId].handleController(
        deviceId, controllerNumber, controllerValue, result));
}

void Cc14Detector::reset(void)
{
    for (int i = 0; i < 33; ++i) {
        devices[i].parameter = 0xff;
        devices[i].resetValue();
        devices[i].isCc14 = false;
        devices[i].expectingLSB = false;
    }
}

Cc14Detector::DeviceState::DeviceState(void)
    : parameter(0xff), valueMSB(0xff), valueLSB(0xff), isCc14(false)
{
}

bool Cc14Detector::DeviceState::handleController(int deviceId,
                                                 uint8_t controllerNumber,
                                                 uint8_t value,
                                                 MidiCc14Message &result)
{
    if (controllerNumber < 32) {
        parameter = controllerNumber;
        valueMSB = value;
        expectingLSB = true;
    } else {
        if ((controllerNumber == (parameter + 32)) && (expectingLSB == true)) {
            valueLSB = value;
            return (sendIfReady(deviceId, result));
        }

        expectingLSB = false;
    }

    return (false);
}

void Cc14Detector::DeviceState::resetValue(void)
{
    valueMSB = 0xff;
    valueLSB = 0xff;
}

bool Cc14Detector::DeviceState::sendIfReady(int deviceId,
                                            MidiCc14Message &result)
{
    result.deviceId = deviceId;
    result.parameterNumber = parameter;
    result.value = (valueMSB << 7) + valueLSB;

    return (true);
}
