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

#include "ComponentTable.h"
#include <algorithm>
#include <utility>
#include <vector>

namespace
{
    using Entry = std::pair<const void *, Component *>;

    /* Sorted by owner, so a lookup is a binary search over the few dozen
     * components on the screen. */
    std::vector<Entry> entries;

    std::vector<Entry>::iterator lowerBound(const void *owner)
    {
        return (std::lower_bound(entries.begin(),
                                 entries.end(),
                                 owner,
                                 [](const Entry &entry, const void *key) {
                                     return (entry.first < key);
                                 }));
    }
} // namespace

void ComponentTable::set(const void *owner, Component *component)
{
    auto it = lowerBound(owner);
    const bool present = (it != entries.end()) && (it->first == owner);

    if (component == nullptr) {
        if (present) {
            entries.erase(it);
        }
    } else if (present) {
        it->second = component;
    } else {
        entries.insert(it, Entry(owner, component));
    }
}

Component *ComponentTable::get(const void *owner)
{
    auto it = lowerBound(owner);

    if ((it != entries.end()) && (it->first == owner)) {
        return (it->second);
    }
    return (nullptr);
}

void ComponentTable::clear(void)
{
    entries.clear();
}
