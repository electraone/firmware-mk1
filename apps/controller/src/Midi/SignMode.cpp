#include "SignMode.h"
#include <string.h>
#include "helpers.h"
#include "Arduino.h"

SignMode translateSignMode(const char *signModeText)
{
    if (signModeText) {
        if (strcmp(signModeText, "twosComplement") == 0) {
            return (SignMode::twosComplement);
        } else if (strcmp(signModeText, "signBit") == 0) {
            return (SignMode::signBit);
        } else if (strcmp(signModeText, "signBit2") == 0) {
            return (SignMode::signBit2);
        } else if (strcmp(signModeText, "binOffset") == 0) {
            return (SignMode::binOffset);
        }
    }

    return (SignMode::noSign);
}

uint16_t getTwosComplementMidiValue(uint8_t bitWidth,
                                    int16_t value,
                                    int16_t minValue,
                                    int16_t maxValue)
{
    uint16_t midiValue = 0;
    int16_t range = getRange(bitWidth);

    if (value < 0) {
        midiValue = range - abs(value);
    } else {
        midiValue = value;
    }

    //logMessage ("twosComplement: value=%d results in midiValue=%d", value, midiValue);
    return (midiValue);
}

uint16_t getSignBitMidiValue(uint8_t bitWidth,
                             int16_t value,
                             int16_t minValue,
                             int16_t maxValue)
{
    uint16_t midiValue = 0;
    int16_t signBit = getRange(bitWidth) / 2;

    if (value < 0) {
        midiValue = signBit + abs(value);
    } else {
        midiValue = value;
    }

    //logMessage ("signBit: value=%d results in midiValue=%d", value, midiValue);

    return (midiValue);
}

uint16_t getUnsignedMidiValue(int16_t value,
                              int16_t minValue,
                              int16_t maxValue,
                              uint16_t midiMin,
                              uint16_t midiMax)
{
    float fMidiValue = map((float)value,
                           (float)minValue,
                           (float)maxValue,
                           (float)midiMin,
                           (float)midiMax);

    int16_t midiValue = round(fMidiValue);

    /* Make sure the output midiValue is within the range given by midiMin and
	 * midiMax
	 */
    midiValue = constrain(midiValue, midiMin, midiMax);
#ifdef DEBUG
    System::logger.write(
        ERROR,
        "getUnsignedMidiValue: value=%d, minValue=%d, maxValue=%d, midiMin=%d,"
        " midiMax=%d results in midiValue=%d (%f)",
        value,
        minValue,
        maxValue,
        midiMin,
        midiMax,
        midiValue,
        fMidiValue);
#endif
    return (midiValue);
}

uint16_t translateValueToMidiValue(SignMode signMode,
                                   uint8_t bitWidth,
                                   int16_t value,
                                   int16_t minValue,
                                   int16_t maxValue,
                                   uint16_t midiMin,
                                   uint16_t midiMax)
{
    uint16_t midiValue = 0;

    if (signMode == SignMode::twosComplement) {
        midiValue =
            getTwosComplementMidiValue(bitWidth, value, minValue, maxValue);
    } else if (signMode == SignMode::signBit) {
        midiValue = getSignBitMidiValue(bitWidth, value, minValue, maxValue);
    } else {
        midiValue =
            getUnsignedMidiValue(value, minValue, maxValue, midiMin, midiMax);
    }

    return (midiValue);
}

int16_t getTwosComplementValue(uint8_t bitWidth,
                               uint16_t midiValue,
                               int16_t minValue,
                               int16_t maxValue)
{
    int16_t value = 0;

    uint16_t range = getRange(bitWidth);

    if (midiValue >= (range / 2)) {
        value = midiValue - range;
    } else {
        value = midiValue;
    }

    return (value);
}

int16_t getSignBitValue(uint8_t bitWidth,
                        uint16_t midiValue,
                        int16_t minValue,
                        int16_t maxValue)
{
    int16_t value = 0;

    int16_t signBit = getRange(bitWidth) / 2;

    if (midiValue >= signBit) {
        value = signBit - midiValue;
    } else {
        value = midiValue;
    }

    return (value);
}

int16_t getUnsignedValue(uint16_t midiValue,
                         uint16_t midiMin,
                         uint16_t midiMax,
                         int16_t minValue,
                         int16_t maxValue)
{
    /* First make sure the midiValue is within boundaries given by midiMin and
	 * midiMax. This is needed, because the MIDI values from outside can be outside
	 * the permitted range
	 */
    midiValue = constrain(midiValue, midiMin, midiMax);

    float fValue = map((float)midiValue,
                       (float)midiMin,
                       (float)midiMax,
                       (float)minValue,
                       (float)maxValue);

    int16_t value = round(fValue);

#ifdef DEBUG
    System::logger.write(
        ERROR,
        "getUnsignedValue: midiValue=%d, minValue=%d, maxValue=%d, midiMin=%d,"
        " midiMax=%d results in value=%d (%f)",
        midiValue,
        minValue,
        maxValue,
        midiMin,
        midiMax,
        value,
        fValue);
#endif

    return (value);
}

int16_t translateMidiValueToValue(SignMode signMode,
                                  uint8_t bitWidth,
                                  uint16_t midiValue,
                                  uint16_t midiMin,
                                  uint16_t midiMax,
                                  int16_t minValue,
                                  int16_t maxValue)
{
    int16_t value = 0;

    /* TODO: for signed values, check that the midi value is within the range
	 * given by the bitWidth
	 */
    if (signMode == SignMode::twosComplement) {
        value = getTwosComplementValue(bitWidth, midiValue, minValue, maxValue);
    } else if (signMode == SignMode::signBit) {
        value = getSignBitValue(bitWidth, midiValue, minValue, maxValue);
    } else {
        value =
            getUnsignedValue(midiValue, midiMin, midiMax, minValue, maxValue);
    }

    // And make sure the resulting value is also within the min and max range
    value = constrain(value, minValue, maxValue);

    return (value);
}
