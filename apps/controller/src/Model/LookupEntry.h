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

/**
 * @file LookupEntry.h
 *
 * @brief Implements a storage for MIDI parameter value
 *       within the the ParamemeterMap
 */

#pragma once

#include "ControlValue.h"
#include <cstdint>
#include <vector>

class LookupEntry
{
public:
    LookupEntry();
    ~LookupEntry() = default;

    /**
     * @brief Sets the MIDI value for the entry
     * 
     * @param newMidiValue a MIDI value to be set
     * @return true when the stored MIDI value has changed
     */
    bool setMidiValue(uint16_t newMidiValue);

    /**
     * @brief Sets the relative MIDI value for the entry
     * 
     * @param newDelta a MIDI value to be set
     */
    void setRelativeMidiValue(uint16_t newDelta);

    /**
     * @brief Applies a MIDI value fragment to the stored MIDI value
     *  The fragment is ORed with the stored MIDI value
     * 
     * @param midiValueFragment a MIDI value fragment to be applied
     */
    void applyToMidiValue(uint16_t midiValueFragment);

    /**
     * @brief Resets the stored MIDI value
     */
    void resetMidiValue(void);

    /**
     * @brief Returns the stored MIDI value
     * 
     * @return uint16_t the stored MIDI value
     */
    uint16_t getMidiValue(void) const;

    /**
     * @brief Returns true when the stored MIDI value is valid
     * 
     * @return true when the stored MIDI value is valid
     */
    bool hasValidMidiValue(void) const;

    /**
     * @brief Adds a destination (ControlValue) to the entry
     * 
     * @param newDestination a pointer to ControlValue to be added
     * @return true when the destination has been added
     */
    bool addDestination(ControlValue *newDestination);

    /**
     * @brief Removes a destination (ControlValue) from the entry
     * 
     * @param destination a ControlValue to be removed
     * @return true when the destination has been removed
     */
    bool removeDestination(ControlValue *destination);

    /**
     * @brief Removes all destinations from the entry
     */
    void removeAllDestinations(void);

    /**
     * @brief Returns true when the entry has destinations
     * 
     * @return true when the entry at least one destination
     */
    bool hasDestinations(void) const;

    /**
     * @brief Returns the list of destinations
     * 
     * @return std::vector<ControlValue*>& the list of destinations
     */
    std::vector<ControlValue *> &getDestinations(void);

    /**
     * @brief Returns the first MIDI message associated with the entry
     * 
     * @return MidiMessage& the MIDI message
     */
    Message &getMessage(void);

    /**
     * @brief Marks the entry for processing
     *  It marks the entry as dirty without
     *  calling the Lua Function. The entry will be
     *  picked up for processing.
     */
    void markForRepaintWithoutFunction(void);

    /**
     * @brief Marks the entry for processing
     *  It marks the entry as dirty and schedules
     *  the Lua Function call. The entry will be
     *  picked up for processing.
     */
    void markForFullRepaint(void);

    /**
     * @brief Marks the entry as processed
     *  The is not considered to be dirty any more. Meaning
     *  it was processed.
     */
    void markAsProcessed(void);

    /**
     * @brief Returns true when the entry is dirty
     *  A dirty entry should be processed
     * 
     * @return true when the entry is dirty
     */
    bool isDirty(void) const;

    /**
     * @brief Returns true when the entry is dirty
     *  and full repaint including Lua FUnction call is required
     * 
     * @return true when the entry is dirty
     */
    bool isForFullRepaint(void) const;

    /**
     * @brief Returns true when the entry is dirty
     *  and Lua Function call is not to be called.
     *  This is used to not to trigger Lua function
     *  for Message::Type:none
     * 
     * @return true when the entry is dirty
     */
    bool isForRepaintWithoutFunction(void) const;

private:
    uint16_t midiValue;
    struct {
        bool dirty : 1;
        bool callFunction : 1;
    };
    std::vector<ControlValue *> messageDestination;

    static Message emptyMessage;
};