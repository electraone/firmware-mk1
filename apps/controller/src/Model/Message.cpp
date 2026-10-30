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

#include "Message.h"
#include "ControlValue.h"
#include <cstring>

Message::Message()
    : parameterNumber(0),
      value(NOT_SET),
      midiMin(0),
      midiMax(NOT_SET),
      controlValue(nullptr),
      deviceId(0),
      type((uint8_t)Message::Type::none),
      lsbFirst(false),
      resetRpn(true),
      signMode((uint8_t)SignMode::noSign),
      bitWidth(7),
      relative(false),
      relativeMode((uint8_t)RelativeMode::signBit),
      accelerated(false)
{
}

Message::Message(uint8_t newDeviceId,
                 Type newType,
                 uint16_t newParameterNumber,
                 uint16_t newMidiMin,
                 uint16_t newMidiMax,
                 uint16_t newValue,
                 std::vector<uint8_t> *newData,
                 bool newLsbFirst,
                 bool newResetRpn,
                 SignMode newSignMode,
                 uint8_t newbitWidth,
                 bool newRelative,
                 RelativeMode newRelativeMode,
                 bool newAccelerated)
    : data(newData),
      parameterNumber(newParameterNumber),
      value(newValue),
      midiMin(newMidiMin),
      midiMax(newMidiMax),
      deviceId(newDeviceId),
      type((uint8_t)newType),
      lsbFirst(newLsbFirst),
      resetRpn(newResetRpn),
      signMode((uint8_t)newSignMode),
      bitWidth(newbitWidth),
      relative(newRelative),
      relativeMode((uint8_t)newRelativeMode),
      accelerated(newAccelerated)
{
}

void Message::setDeviceId(uint8_t newDeviceId)
{
    deviceId = newDeviceId;
}

uint8_t Message::getDeviceId(void) const
{
    return (deviceId);
}

void Message::setType(Message::Type newType)
{
    type = (uint8_t)newType;
}

Message::Type Message::getType(void) const
{
    return ((Message::Type)type);
}

void Message::setMidiMin(uint16_t newMin)
{
    midiMin = newMin;
}

uint16_t Message::getMidiMin(void) const
{
    return (midiMin);
}

void Message::setMidiMax(uint16_t newMax)
{
    midiMax = newMax;
}

uint16_t Message::getMidiMax(void) const
{
    return (midiMax);
}

void Message::setMidiOff(uint16_t newMin)
{
    midiMin = newMin;
}

uint16_t Message::getMidiOff(void) const
{
    return (midiMin);
}

void Message::setMidiOn(uint16_t newMax)
{
    midiMax = newMax;
}

uint16_t Message::getMidiOn(void) const
{
    return (midiMax);
}

void Message::setParameterNumber(uint16_t newParameterNumber)
{
    parameterNumber = newParameterNumber;
}

uint16_t Message::getParameterNumber(void) const
{
    return (parameterNumber);
}

void Message::setValue(uint16_t newValue)
{
    value = newValue;
}

void Message::setEmptyValue(void)
{
    value = MIDI_VALUE_DO_NOT_SEND;
}

bool Message::hasEmptyValue(void) const
{
    return (value == MIDI_VALUE_DO_NOT_SEND);
}

bool Message::hasValidValue(void) const
{
    return (value != MIDI_VALUE_DO_NOT_SEND);
}

uint16_t Message::getValue(void) const
{
    return (value);
}

void Message::setSignMode(SignMode newSignMode)
{
    signMode = (uint8_t)newSignMode;
}

SignMode Message::getSignMode(void) const
{
    return ((SignMode)signMode);
}

void Message::setOffValue(int16_t newOffValue)
{
    midiMin = newOffValue;
}

int16_t Message::getOffValue(void) const
{
    return (midiMin);
}

void Message::setOnValue(int16_t newOnValue)
{
    midiMax = newOnValue;
}

int16_t Message::getOnValue(void) const
{
    return (midiMax);
}

void Message::setLsbFirst(bool newLsbFirst)
{
    lsbFirst = newLsbFirst;
}

bool Message::getLsbFirst(void) const
{
    return (lsbFirst);
}

void Message::setResetRpn(bool newResetRpn)
{
    resetRpn = newResetRpn;
}

bool Message::getResetRpn(void) const
{
    return (resetRpn);
}

void Message::setBitWidth(uint8_t newBitWidth)
{
    bitWidth = newBitWidth;
}

uint8_t Message::getBitWidth(void) const
{
    return (bitWidth);
}

void Message::setRelative(uint8_t shouldBeRelative)
{
    relative = shouldBeRelative;
}

uint8_t Message::isRelative(void) const
{
    return (relative);
}

void Message::setRelativeMode(RelativeMode newRelativeMode)
{
    relativeMode = (uint8_t)newRelativeMode;
}

RelativeMode Message::getRelativeMode(void) const
{
    return ((RelativeMode)relativeMode);
}

void Message::setAccelerated(uint8_t shouldBeAccelerated)
{
    accelerated = shouldBeAccelerated;
}

uint8_t Message::isAccelerated(void) const
{
    return (accelerated);
}

void Message::setControlValue(ControlValue *newControlValue)
{
    controlValue = newControlValue;
}

ControlValue *Message::getControlValue(void) const
{
    return (controlValue);
}

void Message::print(uint8_t logLevel) const
{
    System::logger.write(
        logLevel, "            --[Message]--------------------------------");
    System::logger.write(logLevel, "            address: 0x%08x", this);
    System::logger.write(logLevel, "            deviceId: %d", getDeviceId());
    System::logger.write(logLevel,
                         "            type: %s (%d)",
                         translateType(getType()),
                         getType());
    System::logger.write(
        logLevel, "            parameterNumber: %d", getParameterNumber());
    System::logger.write(logLevel, "            min: %d", getMidiMin());
    System::logger.write(logLevel, "            max: %d", getMidiMax());
    System::logger.write(logLevel, "            value: %d", getValue());
    System::logger.write(logLevel, "            signMode: %d", getSignMode());
    System::logger.write(logLevel, "            lsbFirst: %d", getLsbFirst());
    System::logger.write(logLevel, "            bitWidth: %d", getBitWidth());
    System::logger.write(logLevel, "            relative: %d", isRelative());
    System::logger.write(
        logLevel, "            relativeMode: %d", getRelativeMode());
    System::logger.write(
        logLevel, "            accelerated: %d", isAccelerated());
}

Message::Type Message::translateType(const char *typeText)
{
    if (typeText) {
        if (strcmp(typeText, "cc7") == 0) {
            return (Message::Type::cc7);
        } else if (strcmp(typeText, "cc14") == 0) {
            return (Message::Type::cc14);
        } else if (strcmp(typeText, "nrpn") == 0) {
            return (Message::Type::nrpn);
        } else if (strcmp(typeText, "rpn") == 0) {
            return (Message::Type::rpn);
        } else if (strcmp(typeText, "note") == 0) {
            return (Message::Type::note);
        } else if (strcmp(typeText, "program") == 0) {
            return (Message::Type::program);
        } else if (strcmp(typeText, "sysex") == 0) {
            return (Message::Type::sysex);
        } else if (strcmp(typeText, "start") == 0) {
            return (Message::Type::start);
        } else if (strcmp(typeText, "stop") == 0) {
            return (Message::Type::stop);
        } else if (strcmp(typeText, "tune") == 0) {
            return (Message::Type::tune);
        } else if (strcmp(typeText, "virtual") == 0) {
            return (Message::Type::virt);
        } else if (strcmp(typeText, "atpoly") == 0) {
            return (Message::Type::atpoly);
        } else if (strcmp(typeText, "atchannel") == 0) {
            return (Message::Type::atchannel);
        } else if (strcmp(typeText, "pitchbend") == 0) {
            return (Message::Type::pitchbend);
        } else if (strcmp(typeText, "spp") == 0) {
            return (Message::Type::spp);
        } else if (strcmp(typeText, "relcc") == 0) {
            return (Message::Type::relcc);
        } else if (strcmp(typeText, "none") == 0) {
            return (Message::Type::none);
        }
    } else {
        // default - for rules that do not have type specified
        return (Message::Type::sysex);
    }
    return (Message::Type::none);
}

const char *Message::translateType(Message::Type messageType)
{
    if (messageType == Message::Type::cc7) {
        return ("cc7");
    } else if (messageType == Message::Type::cc14) {
        return ("cc14");
    } else if (messageType == Message::Type::nrpn) {
        return ("nrpn");
    } else if (messageType == Message::Type::rpn) {
        return ("rpn");
    } else if (messageType == Message::Type::note) {
        return ("note");
    } else if (messageType == Message::Type::program) {
        return ("program");
    } else if (messageType == Message::Type::sysex) {
        return ("sysex");
    } else if (messageType == Message::Type::start) {
        return ("start");
    } else if (messageType == Message::Type::stop) {
        return ("stop");
    } else if (messageType == Message::Type::tune) {
        return ("tune");
    } else if (messageType == Message::Type::virt) {
        return ("virtual");
    } else if (messageType == Message::Type::atpoly) {
        return ("atpoly");
    } else if (messageType == Message::Type::atchannel) {
        return ("atchannel");
    } else if (messageType == Message::Type::pitchbend) {
        return ("pitchbend");
    } else if (messageType == Message::Type::spp) {
        return ("spp");
    } else if (messageType == Message::Type::relcc) {
        return ("relcc");
    } else if (messageType == Message::Type::none) {
        return ("none");
    }

    return ("invalid");
}
