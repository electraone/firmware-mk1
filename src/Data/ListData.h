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
 * @file ListData.h
 *
 * @brief A container to hold Selection Lists.
 */

#pragma once

#include "ListDataItem.h"
#include "System.h"
#include <cstdint>
#include <string>
#include <vector>

class ListData
{
public:
    ListData() : id(0)
    {
    }

    explicit ListData(uint8_t newId) : id(newId)
    {
    }

    virtual ~ListData() = default;

    uint16_t getId(void) const
    {
        return (id);
    }

    void addItem(int16_t value,
                 const char *label,
                 const char *bitmap = nullptr,
                 uint16_t colour = 0,
                 bool colourIsSet = false)
    {
        items.push_back(
            ListDataItem(value, label, bitmap, colour, colourIsSet));
    }

    ListDataItem getByIndex(uint16_t index) const
    {
        return (items[index]);
    }

    ListDataItem getByValue(int16_t value) const
    {
        for (const auto &item : items) {
            if (item.getValue() == value) {
                return (item);
            }
        }
        return (ListDataItem());
    }

    int16_t getIndexByValue(int16_t value) const
    {
        for (uint16_t i = 0; i < items.size(); i++) {
            if (items[i].getValue() == value) {
                return (i);
            }
        }
        return (-1);
    }

    int16_t getValueByIndex(uint16_t index) const
    {
        return (items[index].getValue());
    }

        uint16_t getNumItems(void) const
    {
        return (items.size());
    }

    /** Give back the room push_back() grew into and did not use. Called
     *  once the items have all been added. */
    void compact(void)
    {
        items.shrink_to_fit();
    }

    uint16_t getMaxIndex(void) const
    {
        return (getNumItems() - 1);
    }

    bool hasItems(void) const
    {
        return (!items.empty());
    }

    bool isEmpty(void) const
    {
        return (items.empty());
    }

    void print(uint8_t logLevel = LOG_TRACE) const
    {
        System::logger.write(logLevel, "id: %d", getId());
        for (const auto &item : items) {
            System::logger.write(
                LOG_ERROR,
                "    midiValue: %d, label: %s, bitmapEmpty: %d, address=%d",
                item.getValue(),
                item.getLabel(),
                item.isBitmapEmpty(),
                item.getAddress());
        }
    }

private:
    uint16_t id;
    std::vector<ListDataItem> items;
};
