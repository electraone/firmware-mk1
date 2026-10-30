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
 * @file ParameterMap.h
 *
 * @brief Implements a responsive parameter value storage
 */

#pragma once

#include <ArduinoJson.h>
#include "LookupEntry.h"
#include "Control.h"
#include "Origin.h"
#include "Event.h"
#include "System.h"
#include <functional>

class ParameterMapWindow;

class ParameterMap
{
public:
    enum RepaintAction { RepaintLookupEntry, RepaintParameterMap };

    /**
     * @brief Construct a new Parameter Map object
     *
     */
    ParameterMap();

    /**
     * @brief Destroy the Parameter Map object
     *
     */
    ~ParameterMap() = default;

    /**
     * @brief Set the project Id
     * 
     * The project Id is use for manipulating saved parameter maps.
     * 
     * @param newProjectId
     */
    void setProjectId(const char *newProjectId);

    /**
     * @brief Set the App Sandbox name
     * 
     * The app sandbox name tells where to store the parameter maps.
     * 
     * @param newAppSandbox 
     */
    void setAppSandbox(const char *newAppSandbox);

    /**
     * @brief Get or create the LookupEntry
     * 
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * @param controlValue pointer to the ControlValue to be added as a destination
     * 
     * @return LookupEntry* pointer to found of newly created LookupEntry
     */
    LookupEntry *getOrCreate(uint8_t deviceId,
                             Message::Type type,
                             uint16_t parameterNumber,
                             ControlValue *controlValue = nullptr);

    /**
     * @brief Get the LookupEntry
     * 
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * 
     * @return LookupEntry* pointer to found LookupEntry or nullptr
     */
    LookupEntry *
        get(uint8_t deviceId, Message::Type type, uint16_t parameterNumber);

    /**
     * @brief Get MIDI value stored in the LookupEntry
     * 
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * 
     * @return uint16_t 
     */
    uint16_t getValue(uint8_t deviceId,
                      Message::Type type,
                      uint16_t parameterNumber);

    /**
     * @brief  Sets MIDI value stored in the LookupEntry
     * 
     * @param entry a pointer to a LookupEntry
     * @param midiValue MIDI value to be stored
     * @param origin origin of the change
     * 
     * @return LookupEntry* pointer to the modified LookupEntry provided
     */
    LookupEntry *
        setValue(LookupEntry *entry, uint16_t midiValue, Origin origin);

    /**
     * @brief Set MIDI value stored in the LookupEntry. All linked
     *  follow-up actions will be triggered.
     * 
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * @param midiValue MIDI value to be stored
     * @param origin origin of the change
     * 
     * @return LookupEntry* pointer to the modified LookupEntry if any
     */
    LookupEntry *setValue(uint8_t deviceId,
                          Message::Type type,
                          uint16_t parameterNumber,
                          uint16_t midiValue,
                          Origin origin);

    /**
     * @brief Set MIDI value stored in the LookupEntry, without triggering
     *  any follow-up actions.
     * 
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * @param midiValue MIDI value to be stored
     * 
     * @return LookupEntry* pointer to the modified LookupEntry if any
     */
    LookupEntry *setValueSimple(uint8_t deviceId,
                                Message::Type type,
                                uint16_t parameterNumber,
                                uint16_t midiValue);

    /**
     * @brief Modulates MIDI value stored in the LookupEntry. The stored
     *  MIDI value will not be changed, calculated modulated MIDI value
     *  will be sent out.
     * 
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * @param modilationValue MIDI value to be stored
     * @param depth depth of the modulation
     * 
     * @return LookupEntry* pointer to the modified LookupEntry if any
     */
    LookupEntry *modulateValue(uint8_t deviceId,
                               Message::Type type,
                               uint16_t parameterNumber,
                               float modulationValue,
                               int8_t depth);

    /**
     * @brief Process relative MIDI value stored in the LookupEntry
     * 
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * @param midiValue MIDI value to be stored
     * @param origin origin of the change
     * 
     * @return LookupEntry* pointer to the modified LookupEntry if any
     */
    LookupEntry *setRelative(uint8_t deviceId,
                             Message::Type type,
                             uint16_t parameterNumber,
                             uint16_t midiValue,
                             Origin origin);

    /**
     * @brief Apply logical OR to MIDI value stored in the LookupEntry
     * 
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * @param midiValue MIDI value to be stored
     * @param origin origin of the change
     * 
     * @return LookupEntry* pointer to the modified LookupEntry if any
     */
    LookupEntry *applyToValue(uint8_t deviceId,
                              Message::Type type,
                              uint16_t parameterNumber,
                              uint16_t midiValueFragment,
                              Origin origin);

    /**
     * @brief Add ControlValue destination to the LookupEntry.
     * 
     * @param message message used to find or create the LookupEntry
     * 
     * @return true when the destination was added
     */
    bool addDestination(Message *message);

    /**
     * @brief  Remove ControlValue destination from the LookupEntry.
     * 
     * @param message message used to find the LookupEntry
     * 
     * @return true when the destination was removed
     */
    bool removeDestination(Message *message);

    /**
      * @brief  Remove ControlValue destination from the LookupEntry.
      * 
      * @param value ControlValue used to find the LookupEntry
      * 
      * @return true when the destination was removed
      */
    bool removeDestination(ControlValue *value);

    /**
     * @brief Reset all LookupEntry values for a given device
     * 
     * Resetting means setting the MIDI value to 0.
     * 
     * @param deviceId an Id of the Device
     */
    void resetDeviceValues(uint8_t deviceId);

    /**
     * @brief Clear whole ParameterMap
     * 
     * Clear all contents and reset the project Id associated with the map.
     */
    void clear(void);

    /**
     * @brief Print the contents of the ParameterMap
     * 
     * @param logLevel log level to be used for printing
     */
    void print(uint8_t logLevel = LOG_TRACE);

    /**
     * @brief Keep the current state of the ParameterMap
     * 
     * The current state is stored on the persistent storage
     * using the default .map file name.
     */
    void keep(void);

    /**
     * @brief Save the current state of the ParameterMap
     * 
     * The current state is stored on the persistent storage
     * using the provided filename.
     * 
     * @param filename name of the file to be used for storing the state
     */
    void save(const char *filename);

    /**
     * @brief Load the state of the ParameterMap from the persistent storage
     * 
     * @param filename name of the file to be used for loading the state
     * 
     * @return true if the state was loaded successfully
     */
    bool load(const char *filename);

    /**
     * @brief Recall the last saved state of the ParameterMap
     * 
     * The last saved state is loaded from the persistent storage
     * using the default .map file name.
     * 
     * @return true if the state was loaded successfully
     */
    bool recall(void);

    /**
     * @brief Forget (remove) the last saved state of the ParameterMap
     * 
     */
    void forget(void);

    /**
     * @brief Add a ParameterMapWindow to the list of windows to be notified
     * 
     * @param windowToAdd 
     */
    void addWindow(ParameterMapWindow *windowToAdd);

    /**
     * @brief Remove a ParameterMapWindow from the list of windows to be notified
     * 
     * @param windowToRemove 
     */
    void removeWindow(ParameterMapWindow *windowToRemove);

    /**
     * @brief Print list of registered ParameterMapWindows
     * 
     */
    void listWindows(void);

    /**
     * @brief Repaint all registered ParameterMapWindows
     * 
     */
    void repaintParameterMap(void);

    /**
     * @brief Schedule repaint of all entries with Lua
     * functons and formatters
     * 
     */
    void scheduleLuaProcessing(void);

    /**
     * @brief Repaint a single LookupEntry
     * 
     * @param mapEntry 
     */
    void repaintLookupEntry(LookupEntry *lookupEntry);

    /**
     * @brief  Enable parameter map repainting
     * 
     */
    void enable(bool newPresetLoaded = false);

    /**
     * @brief Disable parameter map repainting
     * 
     */
    void disable(void);

    /**
     * @brief Repaint the ParameterMap
     * 
     * Forces repainting of all ParameterMap dirty entries.
     * 
     */
    void repaint(void)
    {
        postRepaint();
    }

    /**
     * @brief Callback function to be called when a LookupEntry changes
     * 
     * @param entry pointer to the LookupEntry that has changed
     * @param origin origin of the change
     * 
     */
    std::function<void(LookupEntry *entry, Origin origin)> onChange;

    /**
     * @brief Process contents of the ParameterMap queue
     * 
     */
    void processQueue(void);

private:
    /**
     * @brief Find a LookupEntry by hash
     * 
     * The entry is cached for faster access.
     * 
     * @param hash value to identify the entry
     * 
     * @return pointer to the found LookupEntry or nullptr
     */
    LookupEntry *getAndCache(uint32_t hash);

    /**
     * @brief Serialize the ParameterMap entries.
     * 
     * The JSON is written directly to the provided file.
     * 
     * @param file file to write to
     */
    void serializeMap(File &file);

    /**
     * @brief Serialize the common information about the ParameterMap.
     * 
     * version and projectId are included. The JSON is written directly
     * to the provided file.
     * 
     * @param file file to write to
     */
    void serializeRoot(File &file);

    /**
     * @brief Deserialize the ParameterMap entries.
     * 
     * @param file file to read from
     * 
     * @return true if the JSON was parsed successfully
     */
    bool parse(File &file);

    /**
     * @brief Deserialize the parameters stored the ParameterMap file.
     * 
     * @param file file to read from
     * 
     * @return true if the JSON was parsed successfully
     */
    bool parseParameters(File &file);

    /**
     * @brief Compose a name for keeping the ParameterMap state.
     * 
     * @param buffer buffer to store the name
     * @param maxLength maximum length of the buffer
     */
    void prepareMapStateFilename(char *buffer, size_t maxLength);

    /**
     * @brief Post a LookupEntry for repainting.
     * 
     * @param entry LookupEntry to be repainted
     */
    void postEntry(LookupEntry *entry);

    /**
     * @brief Post whole ParameterMap for repainting.
     * 
     */
    void postRepaint(void);

    /**
     * @brief Post a LookupEntry or the whole ParameterMap for repainting.
     * 
     * @param entry LookupEntry to be repainted
     * @param repaintAction action to be taken
     */
    void postMessage(LookupEntry *entry, RepaintAction repaintAction);

    /**
     * @brief Create the maps directory if it does not exist.
     * 
     * @return true if the directory was created successfully
     */
    bool createMapsDir(void);

    /**
     * @brief Calculate a hash for storing a LookupEntry
     *
     * @param deviceId an Id of the Device
     * @param type type of the Message (Paramater type)
     * @param parameterNumber an identifier of the parameter
     * 
     * @return 32bit hash value identifying provided data.
     */
    inline static uint32_t calculateHash(uint8_t deviceId,
                                         Message::Type type,
                                         uint16_t parameterNumber);

    /**
     * @brief Extract the Device Id from the hash
     * 
     * @param hash an identifier of the LookupEntry
     * @return uint8_t an Id of the Device
     */
    inline static uint8_t getDeviceId(uint32_t hash);

    /**
     * @brief Extract the Message type from the hash
     * 
     * @param hash an identifier of the LookupEntry
     * @return uint8_t type of the Message (Paramater type)
     */
    inline static uint8_t getType(uint32_t hash);

    /**
     * @brief Extract the parameter number from the hash
     * 
     * @param hash an identifier of the LookupEntry
     * @return uint16_t an identifier of the parameter
     */
    inline static uint16_t getParameterNumber(uint32_t hash);

    std::map<uint32_t, LookupEntry> entries;
    LookupEntry *lastRead;
    uint32_t lastReadHash;
    bool enabled;
    bool onReadyPending;
    char projectId[20 + 1];
    char appSandbox[20 + 1];

    std::vector<ParameterMapWindow *> windows;

    Task repaintParameterMapTask;
};

extern ParameterMap parameterMap;
