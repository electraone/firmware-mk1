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
#include "Pool.h"

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

    /* No virtual destructor, and none needed: a Device is only ever held
     * by value in the preset's map. Declaring one would also have cost the
     * implicit move operations that put a Device into that map without
     * copying its template store. */

    /**
     * @brief Where a SysEx template is in this device's template store.
     *
     * Three bytes, which is what a Message keeps in place of a pointer to a
     * vector. A length of zero is no template at all.
     */
    struct Template {
        uint16_t offset;
        uint8_t length;
    };

    static constexpr Template noTemplate = { 0, 0 };

    bool isValid(void) const;
    uint8_t getId(void) const;
    void setName(const char *newName);
    const char *getName(void) const;
    uint8_t getResponseIndex(uint8_t id) const;
    Template registerData(JsonVariant jData, Preset *preset);

    /** A template the preset gave an id, from the device's patch messages,
     *  so a value's message may name it instead of carrying its own. */
    void setNamedTemplate(uint16_t id, const DataBytes &bytes);
    Template getNamedTemplate(uint16_t id) const;

    /** Puts a template into the store and says where it landed. */
    Template storeTemplate(const DataBytes &bytes);

    /** The bytes of a template, by the offset a Template carries. */
    const uint8_t *templateBytes(uint16_t offset) const;

    /**
     * @brief Every template in the store, in the order they were stored.
     *
     * Each template sits behind a byte giving its length, which is what
     * makes a walk through the store possible; a run never crosses a page,
     * so the walk skips the unused tail of each page.
     *
     * @param f called as f(const uint8_t *bytes, uint8_t length) for each
     */
    template <typename F>
    void forEachTemplate(F f) const
    {
        constexpr size_t perPage = decltype(templates)::perPage;
        size_t index = 0;

        while (true) {
            const uint8_t *prefix = templates.at(index);

            if (prefix == nullptr) {
                const size_t nextPage = (index / perPage) + 1;

                if (nextPage >= templates.pageCount()) {
                    break;
                }
                index = nextPage * perPage;
                continue;
            }
            f(prefix + 1, *prefix);
            index += 1 + *prefix;
        }
    }

    /** How many templates the store holds, for the record. */
    size_t getNumTemplates(void) const;
    void print(uint8_t logLevel = LOG_TRACE) const;

private:
    static constexpr uint8_t MaxNameLength = 20;

    struct {
        uint8_t id : 6;
    };
    char name[MaxNameLength + 1];

    /* The ids the preset wrote, and the template each names. A message
     * carrying its own data has no id and needs none. */
    struct NamedTemplate {
        uint16_t id;
        Template bytes;
    };
    std::vector<NamedTemplate> namedTemplates;

    /* Every SysEx template of this device, back to back in pages of two
     * kilobytes, each behind a byte of length. A template is a dozen bytes, and keeping each in a vector
     * of its own cost twelve bytes of vector and a sixteen-byte heap block
     * for every one - more than twice the bytes themselves, three hundred
     * times over on a synth editor. The store never moves a byte once it is
     * in, and a Template names a run by number, so a Message needs three
     * bytes to find its template rather than a four-byte pointer. */
    Pool<uint8_t, 2048> templates;
    size_t numTemplates;

public:
    std::vector<std::vector<uint8_t>> requests;
    std::vector<Response> responses;
};

typedef std::map<uint8_t, Device> Devices;
