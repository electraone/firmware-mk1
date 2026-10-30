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
 * @file CustomValueIds.cpp
 *
 * @brief The ids a preset gave the values of its Custom controls.
 */

#include "CustomValueIds.h"
#include "Pool.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace
{
    struct Entry {
        const Control *control;
        const char *ids;
        uint8_t count;
    };

    /* Sorted by the control's address, which does not move: a control lives
     * in a pool until the preset goes. */
    std::vector<Entry> entries;

    Pool<char, 512> arena;

    const Entry *findEntry(const Control *control)
    {
        auto entry = std::lower_bound(
            entries.begin(),
            entries.end(),
            control,
            [](const Entry &e, const Control *c) { return (e.control < c); });

        return (((entry == entries.end()) || (entry->control != control))
                    ? nullptr
                    : &*entry);
    }
} // namespace

void CustomValueIds::set(const Control *control,
                         const char *packed,
                         size_t size,
                         uint8_t count)
{
    char *ids = arena.allocate(size);

    if (!ids) {
        return;
    }

    memcpy(ids, packed, size);

    const Entry entry{ control, ids, count };

    entries.insert(std::lower_bound(entries.begin(),
                                    entries.end(),
                                    entry,
                                    [](const Entry &a, const Entry &b) {
                                        return (a.control < b.control);
                                    }),
                   entry);
}

const char *CustomValueIds::get(const Control *control, uint8_t index)
{
    const Entry *entry = findEntry(control);

    if (!entry || (index >= entry->count)) {
        return (nullptr);
    }

    const char *id = entry->ids;

    for (uint8_t i = 0; i < index; i++) {
        id += strlen(id) + 1;
    }

    return (id);
}

bool CustomValueIds::find(const Control *control,
                          const char *id,
                          uint8_t &index)
{
    const Entry *entry = findEntry(control);

    if (!entry) {
        return (false);
    }

    const char *candidate = entry->ids;

    for (uint8_t i = 0; i < entry->count; i++) {
        if (strcmp(candidate, id) == 0) {
            index = i;
            return (true);
        }
        candidate += strlen(candidate) + 1;
    }

    return (false);
}

void CustomValueIds::clear(void)
{
    entries.clear();
    entries.shrink_to_fit();
    arena.clear();
}
