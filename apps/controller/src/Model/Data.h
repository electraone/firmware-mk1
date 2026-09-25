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
 * @file Data.h
 *
 * @brief Implements a container for SysEx data.
 */

#pragma once

#include <cstdint>
#include <vector>
#include "ArduinoJson.h"

class Preset;

typedef std::vector<uint8_t> DataBytes;

class Data
{
public:
    Data(JsonArray jData, Preset *preset);
    virtual ~Data() = default;

    DataBytes get(void) const;
    DataBytes *getPointer(void);
    void makeSysexData(void);
    void addLeadingSysexByte(void);

private:
    void transformParameter(JsonArray jRules);
    void transformValue(JsonArray jRules);
    void transformChecksum(JsonVariant jByte);
    void transformFunction(JsonVariant jByte, Preset *preset);

    DataBytes dataBytes;

    static DataBytes emptyData;
};
