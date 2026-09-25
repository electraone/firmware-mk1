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

#include "LookupEntry.h"

LookupEntry::LookupEntry()
    : midiValue(MIDI_VALUE_DO_NOT_SEND), dirty(false), callFunction(false)
{
}

bool LookupEntry::setMidiValue(uint16_t newMidiValue)
{
    if (midiValue != newMidiValue) {
        midiValue = newMidiValue;
        dirty = true;
        callFunction = true;
        return (true);
    }
    return (false);
}

void LookupEntry::setRelativeMidiValue(uint16_t newDelta)
{
    midiValue = newDelta;
}

void LookupEntry::applyToMidiValue(uint16_t midiValueFragment)
{
    midiValue |= midiValueFragment;
    dirty = true;
    callFunction = true;
}

void LookupEntry::resetMidiValue(void)
{
    midiValue = MIDI_VALUE_DO_NOT_SEND;
}

uint16_t LookupEntry::getMidiValue(void) const
{
    return (midiValue);
}

bool LookupEntry::hasValidMidiValue(void) const
{
    return (midiValue != MIDI_VALUE_DO_NOT_SEND);
}

bool LookupEntry::addDestination(ControlValue *newDestination)
{
    bool added = false;
    if (std::find(messageDestination.begin(),
                  messageDestination.end(),
                  newDestination)
        == messageDestination.end()) {
        messageDestination.push_back(newDestination);
        added = true;
    }
    return (added);
}

bool LookupEntry::removeDestination(ControlValue *destination)
{
    bool removed = false;
    auto it = std::find(
        messageDestination.begin(), messageDestination.end(), destination);
    if (it != messageDestination.end()) {
        messageDestination.erase(it);
        removed = true;
    }
    return (removed);
}

void LookupEntry::removeAllDestinations(void)
{
    messageDestination.clear();
}

bool LookupEntry::hasDestinations(void) const
{
    return (!messageDestination.empty());
}

std::vector<ControlValue *> &LookupEntry::getDestinations(void)
{
    return (messageDestination);
}

Message &LookupEntry::getMessage(void)
{
    if (messageDestination.empty()) {
        return (emptyMessage);
    }
    return (messageDestination[0]->message);
}

void LookupEntry::markForRepaintWithoutFunction(void)
{
    dirty = true;
    callFunction = false;
}

void LookupEntry::markForFullRepaint(void)
{
    dirty = true;
    callFunction = true;
}

void LookupEntry::markAsProcessed(void)
{
    dirty = false;
    callFunction = false;
}

bool LookupEntry::isDirty(void) const
{
    return (dirty);
}

bool LookupEntry::isForFullRepaint(void) const
{
    return (dirty && callFunction);
}

bool LookupEntry::isForRepaintWithoutFunction(void) const
{
    return (dirty && !callFunction);
}

Message LookupEntry::emptyMessage;