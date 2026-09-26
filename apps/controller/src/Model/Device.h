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
 * @file Device.h
 *
 * @brief Implements MIDI device within the Preset model.
 */

#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <deque>
#include "Response.h"
#include "Message.h"
#include "MidiOutput.h"
#include "Data.h"

class Preset;

class Device : public MidiOutput
{
public:
    Device();
    Device(uint8_t newId,
           const char *newName,
           uint8_t newPort,
           uint8_t newChannel,
           uint16_t newRate);

    virtual ~Device() = default;

    bool isValid(void) const;
    uint8_t getId(void) const;
    void setName(const char *newName);
    const char *getName(void) const;
    uint8_t getResponseIndex(uint8_t id) const;
    DataBytes *registerData(JsonVariant jData, Preset *preset);

    /** A template the preset gave an id, from the device's patch messages,
     *  so a value's message may name it instead of carrying its own. */
    void setNamedTemplate(uint16_t id, const DataBytes &bytes);
    DataBytes *getNamedTemplate(uint16_t id);
    void print(uint8_t logLevel = LOG_TRACE) const;

private:
    static constexpr uint8_t MaxNameLength = 20;

    struct {
        uint8_t id : 6;
    };
    char name[MaxNameLength + 1];

    /* The ids the preset wrote, and which template each names. A message
     * carrying its own data has no id and needs none - it keeps a pointer
     * straight to its template. */
    std::vector<std::pair<uint16_t, uint16_t>> namedTemplates;

public:
    std::vector<std::vector<uint8_t>> requests;
    std::vector<Response> responses;
    /* Every SysEx template of this device, in the order they were met. A
     * deque, not a vector: messages point at their template, and a deque
     * does not move what it holds when it grows. Not a map: this used to be
     * one, keyed by an eight-bit id that was also handed out by counting,
     * so a preset with more than 255 SysEx controls had its later templates
     * overwrite its first ones, and every node cost 40 bytes besides. */
    std::deque<DataBytes> sysexTemplates;
};

typedef std::map<uint8_t, Device> Devices;
