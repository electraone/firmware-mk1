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
#include <algorithm>

Device::Device()
    : MidiOutput(MidiInterface::Type::MidiAll, 0, 0, 0),
      id(0),
      numTemplates(0)
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
      numTemplates(0)
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
Device::Template Device::registerData(JsonVariant jData, Preset *preset)
{
    if (jData.is<JsonArray>()) {
        Data data(jData.as<JsonArray>(), preset);
        data.makeSysexData();
        return (storeTemplate(data.get()));
    }
    return (getNamedTemplate(jData.as<uint16_t>()));
}

Device::Template Device::storeTemplate(const DataBytes &bytes)
{
    /* A Template counts its length in a byte, and a run has to fit a page
     * of the store. Anything longer could not be sent either: the sender
     * stops at 512. */
    if (bytes.empty()) {
        return (noTemplate);
    }
    if ((bytes.size() > 255)
        || (1 + bytes.size() > decltype(templates)::perPage)) {
        System::logger.write(
            LOG_ERROR,
            "Device::storeTemplate: template of %d bytes is too long, the "
            "limit is 255: device=%d",
            bytes.size(),
            getId());
        return (noTemplate);
    }

    /* The length goes in front of the bytes, so that the store can be
     * walked - the incoming SysEx matcher tries every template. */
    uint8_t *run = templates.allocate(1 + bytes.size());

    if (run == nullptr) {
        return (noTemplate);
    }

    run[0] = (uint8_t)bytes.size();
    std::copy(bytes.begin(), bytes.end(), run + 1);

    const size_t index = templates.indexOf(run + 1);

    if (index > 0xFFFF) {
        /* Sixty-four kilobytes of templates on one device. The bytes stay
         * in the store, where the incoming matcher will still see them. */
        System::logger.write(
            LOG_ERROR,
            "Device::storeTemplate: template store is full: device=%d",
            getId());
        return (noTemplate);
    }
    numTemplates++;
    return (Template{ (uint16_t)index, (uint8_t)bytes.size() });
}

const uint8_t *Device::templateBytes(uint16_t offset) const
{
    return (templates.at(offset));
}

size_t Device::getNumTemplates(void) const
{
    return (numTemplates);
}

void Device::setNamedTemplate(uint16_t id, const DataBytes &bytes)
{
    const Template stored = storeTemplate(bytes);

    for (auto &named : namedTemplates) {
        if (named.id == id) {
            named.bytes = stored;
            return;
        }
    }
    namedTemplates.push_back(NamedTemplate{ id, stored });
}

Device::Template Device::getNamedTemplate(uint16_t id) const
{
    for (const auto &named : namedTemplates) {
        if (named.id == id) {
            return (named.bytes);
        }
    }

    /* Never defined - the device's patch messages, where ids are given,
     * are read before any control names one - so the message sends
     * nothing rather than something else's bytes. */
    return (noTemplate);
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
    System::logger.write(logLevel, "sysex templates: %d", getNumTemplates());
}
