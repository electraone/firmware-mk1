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
 * @file Device.cpp
 *
 * @brief Implements MIDI device within the Preset model.
 */

#include "Device.h"
#include "Data.h"

Device::Device()
    : MidiOutput(MidiInterface::Type::MidiAll, 0, 0, 0),
      id(0),
      lastMessageId(100)
{
    *name = '\0';
}

Device::Device(uint8_t newId,
               const char *newName,
               uint8_t newPort,
               uint8_t newChannel,
               uint16_t newRate)
    : MidiOutput(MidiInterface::Type::MidiAll, newPort, newChannel, newRate),
      id(newId),
      lastMessageId(1)
{
    setName(newName);
}

bool Device::isValid(void) const
{
    return (id != 0);
}

uint8_t Device::getId(void) const
{
    return (id);
}

void Device::setName(const char *newName)
{
    if (newName) {
        copyString(name, newName, MaxNameLength);
    } else {
        *name = '\0';
    }
}

const char *Device::getName(void) const
{
    return (name);
}

uint8_t Device::getResponseIndex(uint8_t id) const
{
    uint8_t index = 0;

    for (auto &response : responses) {
        if (response.getId() == id) {
            return (index);
        }
        index++;
    }
    return (0);
}

/** Create new data or assign existing
 *
 */
std::vector<uint8_t> *Device::registerData(JsonVariant jData, Preset *preset)
{
    std::vector<uint8_t> *registeredData;

    if (jData.is<JsonArray>()) {
        Data data(jData.as<JsonArray>(), preset);
        data.makeSysexData();
        sysexMessages[lastMessageId] = data.get();
        registeredData = &sysexMessages[lastMessageId];
        lastMessageId++;
    } else {
        uint16_t sysexDataId = jData.as<uint16_t>();
        registeredData = &(sysexMessages[sysexDataId]);
    }
    return (registeredData);
}

void Device::print(uint8_t logLevel) const
{
    System::logger.write(logLevel, "id: %d", getId());
    System::logger.write(logLevel, "name: %s", getName());
    System::logger.write(logLevel, "port: %d", getPort());
    System::logger.write(logLevel, "channel: %d", getChannel());
    System::logger.write(logLevel, "rate: %d", getRate());
    System::logger.write(logLevel, "requests: %d", requests.size());
    System::logger.write(logLevel, "responses: %d", responses.size());
    System::logger.write(logLevel, "sysex messages: %d", sysexMessages.size());
}
