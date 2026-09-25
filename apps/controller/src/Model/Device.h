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
    void print(uint8_t logLevel = LOG_TRACE) const;

private:
    static constexpr uint8_t MaxNameLength = 20;

    struct {
        uint8_t id : 6;
    };
    char name[MaxNameLength + 1];
    uint16_t lastMessageId;

public:
    std::vector<std::vector<uint8_t>> requests;
    std::vector<Response> responses;
    SysexMessages sysexMessages;
};

typedef std::map<uint8_t, Device> Devices;
