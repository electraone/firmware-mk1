/*
* Electra One MIDI Controller Firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

#include "ControlValue.h"
#include "ParameterMap.h"
#include "Control.h"
#include "luaExtension.h"
#include "Hardware.h"

#include <cmath>

char ControlValue::labelBuffer[ControlValue::MaxLabelLength + 1];

namespace
{
    /* The encoding of the sensitivity byte: five bits of mantissa and three of
     * power of ten, the power biased so that the middle of the range is unity.
     * See ControlValue::setSensitivity(). */
    constexpr uint8_t mantissaMask = 0x1f;
    constexpr uint8_t maxMantissa = 31;
    constexpr uint8_t exponentShift = 5;
    constexpr uint8_t numExponents = 8;
    constexpr int8_t exponentBias = 3;

    /* 10 to the power of (index - exponentBias), which is what the mantissa is
     * multiplied by. */
    const float powersOfTen[numExponents] = { 1e-3f, 1e-2f, 1e-1f, 1e0f,
                                              1e1f,  1e2f,  1e3f,  1e4f };
} // namespace

ControlValue::ControlValue()
    : handle(0),
      index(0),
      overlayId(0),
      sensitivity(0),
      defaultValue(0),
      min(0),
      max(NOT_SET),
      formatter(0),
      function(0),
      control(nullptr),
      overlay(nullptr),
      nextDestination(nullptr),
      relative(false),
      accelerated(false),
      labelSet(false),
      labelHandle(VramStore::invalidHandle),
      value(defaultValue)
{
}

ControlValue::ControlValue(Control *newControl,
                           const char *newValueId,
                           uint8_t newIndex,
                           int16_t newDefaultValue,
                           int16_t newMin,
                           int16_t newMax,
                           uint8_t newOverlayId,
                           Message(newMessage),
                           uint8_t newFormatter,
                           uint8_t newFunction,
                           Overlay *newOverlay)
    : control(newControl),
      index(newIndex),
      sensitivity(0),
      defaultValue(newDefaultValue),
      min(newMin),
      max(newMax),
      overlayId(newOverlayId),
      message(newMessage),
      formatter(newFormatter),
      function(newFunction),
      overlay(newOverlay),
      nextDestination(nullptr),
      labelSet(false),
      labelHandle(VramStore::invalidHandle),
      value(defaultValue)
{
    /* The valueId as a numeric handle. A Custom control's ids are whatever
     * the preset wrote, so there is no table to look them up in and the
     * value's position in the list is its handle - which is what the mk2
     * does, and what a script asking for value N of a custom control
     * means. */
    handle = (control && (control->getType() == Control::Type::Custom))
                 ? newIndex
                 : translateId(newValueId);
    relative = message.isRelative();
    accelerated = message.isAccelerated();
}

void ControlValue::setControl(Control *newControl)
{
    control = newControl;
}

ControlValue *ControlValue::getNextDestination(void) const
{
    return (nextDestination);
}

void ControlValue::setNextDestination(ControlValue *newNext)
{
    nextDestination = newNext;
}

Control *ControlValue::getControl(void) const
{
    return (control);
}

int16_t ControlValue::getHandle(void) const
{
    return (handle);
}

int16_t ControlValue::getIndex(void) const
{
    return (index);
}

const char *ControlValue::getId(void) const
{
    return (translateId(handle));
}

void ControlValue::setDefault(int16_t newDefaultValue)
{
    defaultValue = newDefaultValue;
}

int16_t ControlValue::getDefault(void) const
{
    return (defaultValue);
}

void ControlValue::setMin(int16_t newMin)
{
    min = newMin;
}

int16_t ControlValue::getMin(void) const
{
    return (min);
}

void ControlValue::setMax(int16_t newMax)
{
    max = newMax;
}

int16_t ControlValue::getMax(void) const
{
    return (max);
}

void ControlValue::setOverlayId(uint8_t newOverlayId)
{
    overlayId = newOverlayId;
}

uint8_t ControlValue::getOverlayId(void) const
{
    return (overlayId);
}

void ControlValue::setOverlay(Overlay *newOverlay)
{
    if (newOverlay) {
        overlayId = newOverlay->getId();
        max = newOverlay->getMaxIndex();
    }
    overlay = newOverlay;
}

Overlay *ControlValue::getOverlay(void) const
{
    return (overlay);
}

uint16_t ControlValue::getNumSteps(void) const
{
    return (max - min);
}

/* The MIDI span when it is narrower than the display range, which is the case
 * the round trip through the parameter map loses. Only for unsigned encodings:
 * the signed ones map a display value one to one onto a MIDI value, so they
 * have no narrower grid to step on, and their MIDI values are not ordered like
 * the display values - -1 in two's complement is the largest of them. */
static uint16_t coarseMidiSpan(const ControlValue &cv)
{
    const SignMode signMode = cv.message.getSignMode();

    if ((signMode == SignMode::twosComplement)
        || (signMode == SignMode::signBit)) {
        return (0);
    }

    const int32_t midiMin = cv.message.getMidiMin();
    const int32_t midiMax = cv.message.getMidiMax();
    const uint16_t midiSpan =
        (uint16_t)((midiMax > midiMin) ? (midiMax - midiMin)
                                       : (midiMin - midiMax));

    return ((midiSpan > 0) && (midiSpan < cv.getNumSteps()) ? midiSpan : 0);
}

uint16_t ControlValue::getPotSteps(void) const
{
    const uint16_t midiSpan = coarseMidiSpan(*this);

    return ((midiSpan > 0) ? midiSpan : getNumSteps());
}

int16_t ControlValue::turnedValue(int16_t displayValue, int16_t steps) const
{
    /* Start from the value the parameter map holds, not from the component's
     * copy of it. The copy is refreshed by the repaint pass, up to 25 ms after
     * the map changed, and a knob in a hand reports every millisecond or two:
     * every turn in between started from the same stale value and wrote the
     * same result, and on the bench nearly half of each turn was lost that
     * way. */
    const uint16_t liveMidiValue = parameterMap.getValue(
        message.getDeviceId(), message.getType(), message.getParameterNumber());
    const bool isLive = (liveMidiValue != MIDI_VALUE_DO_NOT_SEND);

    if (coarseMidiSpan(*this) == 0) {
        return ((isLive ? translateMidiValue(liveMidiValue) : displayValue)
                + steps);
    }

    /* Step the MIDI value and come back to the display value it stands for.
     * Both directions of the translation round to the nearest, and the display
     * range is the wider one, so the value emitted translates back to exactly
     * this MIDI value. */
    const int32_t midiMin = message.getMidiMin();
    const int32_t midiMax = message.getMidiMax();
    const int32_t direction = (midiMax >= midiMin) ? 1 : -1;
    const int32_t low = (midiMax >= midiMin) ? midiMin : midiMax;
    const int32_t high = (midiMax >= midiMin) ? midiMax : midiMin;

    int32_t midiValue =
        isLive ? liveMidiValue
               : translateValueToMidiValue(
                   message.getSignMode(),
                   message.getBitWidth(),
                   constrain(displayValue, getMin(), getMax()),
                   getMin(),
                   getMax(),
                   message.getMidiMin(),
                   message.getMidiMax());

    midiValue = constrain(midiValue + (direction * steps), low, high);

    return (translateMidiValue((uint16_t)midiValue));
}

bool ControlValue::isFunctionAssigned(void) const
{
    if (luaPreset) {
        return (!luaPreset->luaFunctions[function].empty());
    }
    return (false);
}

bool ControlValue::isFormatterAssigned(void) const
{
    if (luaPreset) {
        return (!luaPreset->luaFunctions[formatter].empty());
    }
    return (false);
}

bool ControlValue::hasLua(void) const
{
    return (isFunctionAssigned() || isFormatterAssigned());
}

const char *ControlValue::ControlValue::getFunction(void) const
{
    if (luaPreset) {
        return (luaPreset->luaFunctions[function].c_str());
    }
    return ("");
}

const std::string ControlValue::getFormatter(void) const
{
    if (luaPreset) {
        if (formatter < luaPreset->luaFunctions.size()) {
            return (luaPreset->luaFunctions[formatter]);
        }
    }
    return std::string();
}

void ControlValue::setLabel(const char *newLabel)
{
    if ((newLabel == nullptr) || (newLabel[0] == '\0')) {
        resetLabel();
        return;
    }

    /* The space is taken on first use, so a value that never gets a label,
     * which is most of them, costs nothing in the store. */
    if (labelHandle == VramStore::invalidHandle) {
        labelHandle = Hardware::memory.stringPool.reserve(MaxLabelLength + 1);

        if (labelHandle == VramStore::invalidHandle) {
            return;
        }
    }

    char text[MaxLabelLength + 1];

    copyString(text, newLabel, MaxLabelLength);
    labelSet = Hardware::memory.stringPool.updateString(labelHandle, text);
}

void ControlValue::resetLabel(void)
{
    /* The record itself is kept. Clearing the flag is enough to make the
     * label gone as far as anything that reads it is concerned, and the space
     * would not come back anyway. */
    labelSet = false;
}

const char *ControlValue::getLabel(void) const
{
    if (!labelSet) {
        labelBuffer[0] = '\0';
        return (labelBuffer);
    }

    return (Hardware::memory.stringPool.loadString(
        labelHandle, labelBuffer, sizeof(labelBuffer)));
}

bool ControlValue::isLabelSet(void) const
{
    return (labelSet);
}

void ControlValue::setValue(int16_t newValue)
{
    value = newValue;
}

int16_t ControlValue::getValue(void) const
{
    return (value);
}

void ControlValue::callFormatter(int16_t value)
{
    /* @todo this need some clean up */
    if ((L != nullptr) // Lua is initialized
        && luaPreset // preset is loaded
        && (formatter
            < luaPreset->luaFunctions.size()) // function Id is within the range
        && !luaPreset->luaFunctions[formatter]
                .empty()) { // function really exists
        /* The formatter writes into the buffer only when it returns
         * something, so an empty one afterwards means the label is to stay
         * as it was. Starting from empty keeps that behaviour without having
         * to fetch the current label first. */
        char formatted[MaxLabelLength + 1] = {};

        runFormatter(luaPreset->luaFunctions[formatter].c_str(),
                     this,
                     value,
                     formatted,
                     MaxLabelLength);

        if (formatted[0] != '\0') {
            setLabel(formatted);
        }
    }
}

void ControlValue::callFunction(int16_t value) const
{
    /* @todo this need some clean up */
    if ((L != nullptr) // Lua is initialized
        && luaPreset // preset is loaded
        && (value != MIDI_VALUE_DO_NOT_SEND) // MIDI value is valid
        && (function
            < luaPreset->luaFunctions.size()) // function Id is within the range
        && !luaPreset->luaFunctions[function]
                .empty()) { // function really exists
        runFunction(luaPreset->luaFunctions[function].c_str(), this, value);
    }
}

const char *ControlValue::translateId(uint8_t id) const
{
    switch (control->getType()) {
        case Control::Type::Fader:
        case Control::Type::List:
        case Control::Type::Pad:
            return ("value");
            break;

        case Control::Type::Adsr:
            if (id < 4) {
                return (valueIdsAdsr[id]);
            }
            break;

        case Control::Type::Adr:
            if (id < 3) {
                return (valueIdsAdr[id]);
            }
            break;

        case Control::Type::Dx7envelope:
            if (id < 8) {
                return (valueIdsDx7Env[id]);
            }
            break;

        default:
            break;
    }
    return ("value");
}

uint8_t ControlValue::translateId(const char *handle) const
{
    return (Control::translateValueToId(control->getType(), handle));
}

int16_t ControlValue::translateMidiValue(uint16_t midiValue) const
{
    if (control->getType() == Control::Type::List) {
        if (overlay) {
            return (overlay->getIndexByValue(midiValue));
        } else {
            return (0);
        }
    } else if (control->getType() == Control::Type::Pad) {
        return (midiValue == message.getOnValue());
    }
    return (translateMidiValueToValue(message.getSignMode(),
                                      message.getBitWidth(),
                                      midiValue,
                                      message.getMidiMin(),
                                      message.getMidiMax(),
                                      getMin(),
                                      getMax()));
}

void ControlValue::print(uint8_t logLevel) const
{
    System::logger.write(
        logLevel, "        --[Value]------------------------------------");
    System::logger.write(logLevel, "        address: 0x%08x", this);
    System::logger.write(logLevel, "        id: %s", getId());
    System::logger.write(logLevel, "        handle: %d", getHandle());
    System::logger.write(logLevel, "        index: %d", getIndex());
    System::logger.write(logLevel, "        default: %d", getDefault());
    System::logger.write(logLevel, "        min: %d", getMin());
    System::logger.write(logLevel, "        max: %d", getMax());
    System::logger.write(logLevel, "        overlayId: %d", getOverlayId());

    /* As thousandths, because the logger has no float. A value that declares
     * none prints 1000, which is the rate it moves at. */
    System::logger.write(logLevel,
                         "        sensitivity: %d/1000 (declared: %d)",
                         (int)(getSensitivity() * 1000.0f),
                         (int)hasSensitivity());
    System::logger.write(logLevel, "        function: %s", getFunction());
    System::logger.write(
        logLevel, "        formatter: %s", getFormatter().c_str());
    System::logger.write(logLevel, "        overlay: %x", getOverlay());
    System::logger.write(
        logLevel, "        control address: 0x%08x", getControl());
    message.print(logLevel);
}

bool ControlValue::isRelative(void) const
{
    return (relative);
}

bool ControlValue::isAccelerated(void) const
{
    return (accelerated);
}

int16_t ControlValue::calculateRelativeDelta(int16_t value) const
{
    return (accelerated ? value : sign(value));
}

/*
 * A byte rather than a float, because there is one of these for every value of
 * every control and the byte is free where a float is not - see the note on the
 * member. The mantissa is taken at the finest power of ten that still fits in
 * five bits, so that the significant figures a preset wrote are the ones kept:
 * 0.25 is stored as 25 thousandths of ten, not as 3 tenths.
 */
void ControlValue::setSensitivity(float newSensitivity)
{
    /* Written this way round so that a NaN, which compares false with
     * everything, falls to the default rather than through both clamps. */
    if (!(newSensitivity > 0.0f)) {
        sensitivity = 0;
        return;
    }

    if (newSensitivity < MinSensitivity) {
        newSensitivity = MinSensitivity;
    } else if (newSensitivity > MaxSensitivity) {
        newSensitivity = MaxSensitivity;
    }

    for (uint8_t exponent = 0; exponent < numExponents; exponent++) {
        const long mantissa =
            std::lroundf(newSensitivity / powersOfTen[exponent]);

        if ((mantissa >= 1) && (mantissa <= maxMantissa)) {
            sensitivity =
                (uint8_t)mantissa | (uint8_t)(exponent << exponentShift);
            return;
        }
    }

    /* Unreachable for anything inside the clamps above: the coarsest power of
     * ten covers a hundred with a mantissa of ten. */
    sensitivity = 0;
}

float ControlValue::getSensitivity(void) const
{
    if (sensitivity == 0) {
        return (DefaultSensitivity);
    }

    return ((float)(sensitivity & mantissaMask)
            * powersOfTen[sensitivity >> exponentShift]);
}

bool ControlValue::hasSensitivity(void) const
{
    return (sensitivity != 0);
}

int16_t ControlValue::sign(int16_t value)
{
    return ((value > 0) - (value < 0));
}

int16_t ControlValue::invertedSign(int16_t value)
{
    return ((value < 0) - (value > 0));
}