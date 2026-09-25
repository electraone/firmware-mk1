#include "RpnDetector.h"
#include <cassert>

RpnDetector::RpnDetector()
{
}

bool RpnDetector::parseControllerMessage(int deviceId,
                                         uint8_t controllerNumber,
                                         uint8_t controllerValue,
                                         MidiRpnMessage &result)
{
    if (!((0 <= deviceId) && (deviceId <= 32))) {
        return (false);
    }

    return (devices[deviceId].handleController(
        deviceId, controllerNumber, controllerValue, result));
}

void RpnDetector::reset(void)
{
    for (int i = 0; i < 33; ++i) {
        devices[i].parameterMSB = 0xff;
        devices[i].parameterLSB = 0xff;
        devices[i].resetValue();
        devices[i].isNrpn = false;
    }
}

RpnDetector::DeviceState::DeviceState(void)
    : parameterMSB(0xff),
      parameterLSB(0xff),
      valueMSB(0xff),
      valueLSB(0xff),
      isNrpn(false)
{
}

bool RpnDetector::DeviceState::handleController(int deviceId,
                                                uint8_t controllerNumber,
                                                uint8_t value,
                                                MidiRpnMessage &result)
{
    switch (controllerNumber) {
        case 0x62:
            parameterLSB = value;
            resetValue();
            isNrpn = true;
            break;
        case 0x63:
            parameterMSB = value;
            resetValue();
            isNrpn = true;
            break;

        case 0x64:
            parameterLSB = value;
            resetValue();
            isNrpn = false;
            break;
        case 0x65:
            parameterMSB = value;
            resetValue();
            isNrpn = false;
            break;

        case 0x06:

            valueMSB = value;
            if (valueLSB < 0x80) {
                return (sendIfReady(deviceId, result));
            }
            break;

        case 0x26:
            valueLSB = value;
            if (valueMSB < 0x80) {
                return (sendIfReady(deviceId, result));
            }
            break;

        default:
            break;
    }

    return (false);
}

void RpnDetector::DeviceState::resetValue(void)
{
    valueMSB = 0xff;
    valueLSB = 0xff;
}

bool RpnDetector::DeviceState::sendIfReady(int deviceId, MidiRpnMessage &result)
{
    if (parameterMSB < 0x80 && parameterLSB < 0x80) {
        if (valueMSB < 0x80) {
            result.deviceId = deviceId;
            result.parameterNumber = (parameterMSB << 7) + parameterLSB;
            result.isNrpn = isNrpn;

            if (valueLSB < 0x80) {
#ifdef DEBUG
                System::logger.write(LOG_ERROR,
                                     "value 14bit (valueMsb=%d, valueLsb=%d)",
                                     valueMSB,
                                     valueLSB);
#endif
                result.value = (valueMSB << 7) + valueLSB;
                result.is14BitValue = true;
            } else {
#ifdef DEBUG
                System::logger.write(LOG_ERROR,
                                     "value 7bit (valueMsb=%d, valueLsb=%d)",
                                     valueMSB,
                                     valueLSB);
#endif
                result.value = valueMSB;
                result.is14BitValue = false;
            }

            return (true);
        }
    }

    return (false);
}
