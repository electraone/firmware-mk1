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
#include "Control.h"
#include "luaExtension.h"

ControlValue::ControlValue()
    : handle(0),
      index(0),
      overlayId(0),
      defaultValue(0),
      min(0),
      max(NOT_SET),
      formatter(0),
      function(0),
      control(nullptr),
      overlay(nullptr),
      relative(false),
      accelerated(false),
      value(defaultValue)
{
    label[0] = '\0';
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
      defaultValue(newDefaultValue),
      min(newMin),
      max(newMax),
      overlayId(newOverlayId),
      message(newMessage),
      formatter(newFormatter),
      function(newFunction),
      overlay(newOverlay),
      value(defaultValue)
{
    // translate the valueId to the numeric handle
    handle = translateId(newValueId);
    label[0] = '\0';
    relative = message.isRelative();
    accelerated = message.isAccelerated();
}

void ControlValue::setControl(Control *newControl)
{
    control = newControl;
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
    copyString(label, newLabel, MaxLabelLength);
}

void ControlValue::resetLabel(void)
{
    label[0] = '\0';
}

const char *ControlValue::getLabel(void) const
{
    return (label);
}

bool ControlValue::isLabelSet(void) const
{
    return (label[0] != '\0');
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
        runFormatter(luaPreset->luaFunctions[formatter].c_str(),
                     this,
                     value,
                     label,
                     MaxLabelLength);
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

int16_t ControlValue::sign(int16_t value)
{
    return ((value > 0) - (value < 0));
}

int16_t ControlValue::invertedSign(int16_t value)
{
    return ((value < 0) - (value > 0));
}