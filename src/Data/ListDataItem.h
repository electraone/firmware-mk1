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
 * @file ListDataItem.h
 *
 * @brief A entry of Selection Lists.
 */

#pragma once

#include <cstdint>
#include "Hardware.h"
#include "Bitmap.h"
#include "Base64.h"

class ListDataItem
{
public:
    ListDataItem()
        : value(0),
          address(VramStore::invalidHandle),
          labelIsEmpty(true),
          bitmapIsEmpty(true)
    {
    }

    ListDataItem(int16_t newValue,
                 const char *label,
                 const char *encodedBitmap = nullptr)
        : value(newValue),
          address(VramStore::invalidHandle),
          labelIsEmpty(false),
          bitmapIsEmpty(true)
    {
        Bitmap bitmap;

        if (encodedBitmap) {
            bitmap = saveBitmap(encodedBitmap);
            bitmapIsEmpty = false;
        }

        address = saveRecord(bitmap.getAddress(), label);
    }

    virtual ~ListDataItem() = default;

    int16_t getValue(void) const
    {
        return (value);
    }

    /**
     * @brief The item's label.
     *
     * @attention the returned pointer is a shared buffer that the next call
     * overwrites, which is the same contract this had before the label moved
     * into the display's memory. Copy it if it has to outlive the next call.
     */
    const char *getLabel(void) const
    {
        Record record;

        readRecord(record);
        copyString(buffer, record.label, maxLabelLength);

        return (buffer);
    }

    const Bitmap getBitmap(void) const
    {
        if (bitmapIsEmpty) {
            return (Bitmap());
        }

        Record record;

        readRecord(record);

        return (Bitmap(record.bitmapAddress));
    }

    /**
     * @brief The label and the bitmap, fetched together.
     *
     * Both live in one record in the display's memory, so asking for them
     * separately crosses the bus twice for data that arrived the first time.
     * Callers that need both, which is every painter that can draw either,
     * should use this instead.
     *
     * @attention the returned label is the same shared buffer getLabel()
     * hands out, and the next call to either overwrites it.
     */
    const char *getLabelAndBitmap(Bitmap &bitmap) const
    {
        Record record;

        readRecord(record);
        copyString(buffer, record.label, maxLabelLength);

        bitmap = bitmapIsEmpty ? Bitmap() : Bitmap(record.bitmapAddress);

        return (buffer);
    }

    void paintBitmap(uint16_t x, uint16_t y, uint32_t colour)
    {
        Hardware::memory.bitmapPool.paint(getBitmap(), x, y, colour);
    }

    uint32_t getAddress(void) const
    {
        return (address);
    }

    bool isLabelEmpty(void) const
    {
        return (labelIsEmpty);
    }

    bool isBitmapEmpty(void) const
    {
        return (bitmapIsEmpty);
    }

private:
    /** Longest label kept for an item. Anything longer is truncated when it
     *  is stored, so the reader never has to deal with a partial one. */
    static const size_t maxLabelLength = 39;

    /**
     * @brief One item as it lives in the display's memory.
     *
     * The bitmap handle and the label travel together because they are always
     * wanted together and a read costs far more in fixed overhead than in
     * bytes moved.
     */
    struct Record {
        uint32_t bitmapAddress;
        char label[maxLabelLength + 1];
    };

    /** Parse bitmap data
	 *
	 */
    Bitmap saveBitmap(const char *bitmap)
    {
        uint8_t bytes[255] = {};

        base64_decode((char *)bytes, (char *)bitmap, strlen(bitmap));

        return (Hardware::memory.bitmapPool.saveBitmap(bytes));
    }

    VramStore::Handle saveRecord(uint32_t bitmapAddress, const char *label)
    {
        Record record;

        record.bitmapAddress = bitmapAddress;
        copyString(record.label, label ? label : "", maxLabelLength);

        return (Hardware::memory.stringPool.store(
            &record, sizeof(record.bitmapAddress) + strlen(record.label) + 1));
    }

    void readRecord(Record &record) const
    {
        record.bitmapAddress = 0;
        record.label[0] = '\0';

        size_t read = Hardware::memory.stringPool.load(
            address, &record, sizeof(record));

        /* A record that did not survive, because the store was full when it
         * was written, reads back as nothing rather than as somebody else's
         * label. */
        if (read <= sizeof(record.bitmapAddress)) {
            record.bitmapAddress = 0;
            record.label[0] = '\0';
            return;
        }
        record.label[read - sizeof(record.bitmapAddress) - 1] = '\0';
    }

    int16_t value;
    VramStore::Handle address;

    struct {
        bool labelIsEmpty : 1;
        bool bitmapIsEmpty : 1;
    };

    /** Shared scratch the label is handed out through. See getLabel(). */
    static char buffer[maxLabelLength + 1];
};
