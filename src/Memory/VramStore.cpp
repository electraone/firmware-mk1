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
 * @file VramStore.cpp
 *
 * @brief Keeps arbitrary data in the display controller's memory.
 */

#include "VramStore.h"
#include "VideoRam.h"

#include <string.h>

VramStore::VramStore(uint32_t newBaseAddress, size_t newCapacity)
    : driver(nullptr),
      baseAddress(newBaseAddress),
      capacity(newCapacity),
      used(0),
      rejected(0)
{
}

void VramStore::assignStorageDriver(VideoRam *newDriver)
{
    driver = newDriver;
}

void VramStore::reset(void)
{
    used = 0;
    rejected = 0;
}

size_t VramStore::getCapacity(void) const
{
    return (capacity);
}

size_t VramStore::getUsed(void) const
{
    return (used);
}

size_t VramStore::getRemaining(void) const
{
    return (capacity - used);
}

uint32_t VramStore::getRejectedCount(void) const
{
    return (rejected);
}

VramStore::Handle VramStore::store(const void *data, size_t size)
{
    if ((driver == nullptr) || (size > maxRecordSize)) {
        rejected++;
        return (invalidHandle);
    }

    if ((used + headerSize + size) > capacity) {
        /* Refusing is the lesser evil. Wrapping would hand out a handle that
         * aliases a record something else is still using, and the damage
         * would show up far from here. */
        rejected++;
        return (invalidHandle);
    }

    Handle handle = (Handle)used;
    uint8_t header[headerSize];

    header[0] = (uint8_t)(size & 0xFF);
    header[1] = (uint8_t)((size >> 8) & 0xFF);

    driver->writeRamData(baseAddress + used, header, headerSize);
    used += headerSize;

    if (size > 0) {
        driver->writeRamData(
            baseAddress + used, (const uint8_t *)data, (uint16_t)size);
        used += size;
    }

    return (handle);
}

VramStore::Handle VramStore::storeString(const char *text)
{
    if (text == nullptr) {
        text = "";
    }
    return (store(text, strlen(text) + 1));
}

size_t VramStore::sizeOf(Handle handle) const
{
    if ((driver == nullptr) || (handle == invalidHandle)
        || ((size_t)handle + headerSize > used)) {
        return (0);
    }

    uint8_t header[headerSize];

    driver->readRamData(baseAddress + handle, header, headerSize);

    return ((size_t)header[0] | ((size_t)header[1] << 8));
}

size_t VramStore::load(Handle handle, void *buffer, size_t bufferSize) const
{
    if ((driver == nullptr) || (buffer == nullptr) || (bufferSize == 0)
        || (handle == invalidHandle)
        || ((size_t)handle + headerSize > used)) {
        return (0);
    }

    size_t available = used - (size_t)handle;

    if (bufferSize <= inlineReadLimit) {
        /* Header and payload in one go. A read costs far more in fixed
         * overhead than in bytes moved, so one slightly oversized read beats
         * two exact ones. Whatever follows the record is read and discarded. */
        uint8_t scratch[headerSize + inlineReadLimit];
        size_t toRead = headerSize + bufferSize;

        if (toRead > available) {
            toRead = available;
        }

        driver->readRamData(baseAddress + handle, scratch, (uint16_t)toRead);

        size_t recordSize = (size_t)scratch[0] | ((size_t)scratch[1] << 8);
        size_t copied = recordSize;

        if (copied > bufferSize) {
            copied = bufferSize;
        }
        if (copied > (toRead - headerSize)) {
            copied = toRead - headerSize;
        }

        memcpy(buffer, scratch + headerSize, copied);
        return (copied);
    }

    /* A large record is read straight into the caller's buffer, after its
     * length is fetched. Two round trips, but no scratch of unbounded size. */
    uint8_t header[headerSize];

    driver->readRamData(baseAddress + handle, header, headerSize);

    size_t recordSize = (size_t)header[0] | ((size_t)header[1] << 8);
    size_t copied = recordSize;

    if (copied > bufferSize) {
        copied = bufferSize;
    }
    if (copied > (available - headerSize)) {
        copied = available - headerSize;
    }

    if (copied > 0) {
        driver->readRamData(baseAddress + handle + headerSize,
                            (uint8_t *)buffer,
                            (uint16_t)copied);
    }

    return (copied);
}

const char *
    VramStore::loadString(Handle handle, char *buffer, size_t bufferSize) const
{
    if ((buffer == nullptr) || (bufferSize == 0)) {
        return ("");
    }

    size_t copied = load(handle, buffer, bufferSize);

    /* Always terminated, whatever came back. A record longer than the buffer
     * is truncated rather than allowed to run off the end. */
    if (copied >= bufferSize) {
        copied = bufferSize - 1;
    }
    buffer[copied] = '\0';

    return (buffer);
}
