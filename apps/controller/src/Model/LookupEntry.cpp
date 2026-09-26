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
    : midiValue(MIDI_VALUE_DO_NOT_SEND),
      dirty(false),
      callFunction(false),
      firstDestination(nullptr)
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
    /* Appended at the tail so the first value registered stays first, which
     * is the one getMessage() answers with. The chains are one or two values
     * long, so the walk costs nothing worth a tail pointer. */
    ControlValue **link = &firstDestination;

    while (*link) {
        if (*link == newDestination) {
            return (false);
        }
        link = &(*link)->nextDestinationRef();
    }
    newDestination->setNextDestination(nullptr);
    *link = newDestination;
    return (true);
}

bool LookupEntry::removeDestination(ControlValue *destination)
{
    ControlValue **link = &firstDestination;

    while (*link) {
        if (*link == destination) {
            *link = destination->getNextDestination();
            destination->setNextDestination(nullptr);
            return (true);
        }
        link = &(*link)->nextDestinationRef();
    }
    return (false);
}

void LookupEntry::removeAllDestinations(void)
{
    ControlValue *value = firstDestination;

    while (value) {
        ControlValue *next = value->getNextDestination();
        value->setNextDestination(nullptr);
        value = next;
    }
    firstDestination = nullptr;
}

bool LookupEntry::hasDestinations(void) const
{
    return (firstDestination != nullptr);
}

DestinationRange LookupEntry::getDestinations(void) const
{
    return (DestinationRange(firstDestination));
}

Message &LookupEntry::getMessage(void)
{
    if (!firstDestination) {
        return (emptyMessage);
    }
    return (firstDestination->message);
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