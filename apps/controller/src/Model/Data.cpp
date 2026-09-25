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
 * @file Data.cpp
 *
 * @brief Implements a container for SysEx data.
 */

#include "Data.h"
#include "Message.h"
#include "Checksum.h"
#include <cstring>
#include "Preset.h"

/** Parse JSON sysex data object
 *
 */
Data::Data(JsonArray jData, Preset *preset)
{
    for (JsonVariant jByte : jData) {
        if (jByte.is<JsonObject>()) {
            const char *type = jByte["type"];
            if (type) {
                if (strcmp(type, "value") == 0) {
                    transformValue(jByte["rules"]);
                } else if (strcmp(type, "parameter") == 0) {
                    transformParameter(jByte["rules"]);
                } else if (strcmp(type, "checksum") == 0) {
                    transformChecksum(jByte);
                } else if (strcmp(type, "function") == 0) {
                    transformFunction(jByte, preset);
                }
            }
        } else {
            uint8_t byte = 0;

            // Support both Hex and Dec notation
            if (jByte.is<char *>()) {
                byte = strtol(jByte, 0, 16);
            } else {
                byte = jByte.as<uint8_t>();
            }

            // Add byte, unless it is a SysEx start or stop byte
            if ((byte != 0xF0) && (byte != 0xF7)) {
                dataBytes.push_back(byte);
            }
        }
    }
}

/** return reference JSON sysex data object
 *
 */
DataBytes Data::get(void) const
{
    return (dataBytes);
}

/** return pointer JSON sysex data object
 *
 */
DataBytes *Data::getPointer(void)
{
    return (&dataBytes);
}

/** Transforms parameter variable to the SysEx template bytes
 *
 */
void Data::transformParameter(JsonArray jRules)
{
    dataBytes.push_back(VARIABLE_PARAMETER);

    if (!jRules || (jRules.size() == 0)) {
        dataBytes.push_back(0);
        dataBytes.push_back(0);
        dataBytes.push_back(7);
    } else {
        for (JsonObject jRule : jRules) {
            uint8_t pPos = 0;
            if (!jRule["parameterBitPosition"].isNull()) {
                pPos = jRule["parameterBitPosition"].as<uint8_t>();
            } else {
                pPos = jRule["pPos"].as<uint8_t>();
            }

            uint8_t bPos = 0;
            if (!jRule["byteBitPosition"].isNull()) {
                bPos = jRule["byteBitPosition"].as<uint8_t>();
            } else {
                bPos = jRule["bPos"].as<uint8_t>();
            }

            uint8_t size = 0;
            if (!jRule["bitWidth"].isNull()) {
                size = jRule["bitWidth"].as<uint8_t>();
            } else if (!jRule["size"].isNull()) {
                size = jRule["size"].as<uint8_t>();
            } else {
                size = 7;
            }

            dataBytes.push_back(pPos);
            dataBytes.push_back(bPos);
            dataBytes.push_back(size);
        }
    }
    dataBytes.push_back(VARIABLE_END);
}

/** Transforms value variable to the SysEx template bytes
 *
 */
void Data::transformValue(JsonArray jRules)
{
    dataBytes.push_back(VARIABLE_DATA);

    if (!jRules || (jRules.size() == 0)) {
        dataBytes.push_back((uint8_t)Message::Type::sysex);
        dataBytes.push_back(0);
        dataBytes.push_back(0);
        dataBytes.push_back(0);
        dataBytes.push_back(0);
        dataBytes.push_back(7);
    } else {
        for (JsonObject jRule : jRules) {
            const char *type = jRule["type"].as<char *>();

            uint16_t parameterNumber = 0;
            if (!jRule["parameterNumber"].isNull()) {
                parameterNumber = jRule["parameterNumber"].as<uint16_t>();
            } else {
                parameterNumber = jRule["id"].as<uint16_t>();
            }

            uint8_t pPos = 0;
            if (!jRule["parameterBitPosition"].isNull()) {
                pPos = jRule["parameterBitPosition"].as<uint8_t>();
            } else {
                pPos = jRule["pPos"].as<uint8_t>();
            }

            uint8_t bPos = 0;
            if (!jRule["byteBitPosition"].isNull()) {
                bPos = jRule["byteBitPosition"].as<uint8_t>();
            } else {
                bPos = jRule["bPos"].as<uint8_t>();
            }

            uint8_t size = 0;
            if (!jRule["bitWidth"].isNull()) {
                size = jRule["bitWidth"].as<uint8_t>();
            } else if (!jRule["size"].isNull()) {
                size = jRule["size"].as<uint8_t>();
            } else {
                size = 7;
            }

            Message::Type messageType = Message::translateType(type);

            dataBytes.push_back((uint8_t)messageType);
            dataBytes.push_back(parameterNumber & 0x7F);
            dataBytes.push_back(parameterNumber >> 7);
            dataBytes.push_back(pPos);
            dataBytes.push_back(bPos);
            dataBytes.push_back(size);
        }
    }

    dataBytes.push_back(VARIABLE_END);
}

/** Transforms checksum variable to the SysEx template bytes
 *
 */
void Data::transformChecksum(JsonVariant jByte)
{
    dataBytes.push_back(CHECKSUM);
    uint8_t start = jByte["start"].as<uint8_t>();
    uint8_t length = jByte["length"].as<uint8_t>();
    const char *algorithm = jByte["algorithm"].as<char *>();
    ChecksumAlgorithm checksumAlgorithm = translateAlgorithm(algorithm);

    dataBytes.push_back((uint8_t)checksumAlgorithm);
    dataBytes.push_back(start);
    dataBytes.push_back(length);
}

/** Transforms checksum variable to the SysEx template bytes
 *
 */
void Data::transformFunction(JsonVariant jByte, Preset *preset)
{
    const char *functionName = jByte["name"].as<char *>();

    dataBytes.push_back(LUAFUNCTION);
    dataBytes.push_back(preset->registerFunction(functionName));
}

void Data::makeSysexData(void)
{
    if (dataBytes.size() > 0) {
        if (dataBytes[0] != 0xF0) {
            dataBytes.insert(dataBytes.begin(), 0xF0);
            dataBytes.push_back(0xF7);
        }
    }
}

void Data::addLeadingSysexByte(void)
{
    if (dataBytes.size() > 0) {
        if (dataBytes[0] != 0xF0) {
            dataBytes.insert(dataBytes.begin(), 0xF0);
        }
    }
}
