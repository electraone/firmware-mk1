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
 * @file VramStore.h
 *
 * @brief Keeps arbitrary data in the display controller's memory instead of
 *        in the MCU's own RAM.
 *
 * There are sixteen megabytes attached to the display controller and a
 * quarter of a megabyte attached to the processor, so anything that is read
 * rarely, and never from an interrupt, is better kept over there.
 *
 * What this is for: data written once and read occasionally. Labels, names
 * and other text read only while painting are the obvious case. What this is
 * not for: anything on the MIDI path or anything read in a loop. A read costs
 * tens of microseconds and takes the SPI bus, which the display also needs.
 *
 * Records are addressed by an opaque handle and carry their own length, so a
 * reader does not have to know how big something was when it was stored.
 *
 * Allocation is a bump pointer and there is no per-record free. That matches
 * how the data behaves: it is filled in when a preset loads and becomes
 * worthless all at once when the preset is replaced, at which point reset()
 * reclaims the lot. A store that fills up refuses further records and says so
 * rather than wrapping and quietly overwriting live ones.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

class VideoRam;

class VramStore
{
public:
    typedef uint32_t Handle;

    /** Returned when a record could not be stored. Reading it yields
     *  nothing, so a caller that ignores the failure gets empty data rather
     *  than someone else's. */
    static const Handle invalidHandle = 0xFFFFFFFFu;

    /** Largest single record. The length is kept in two bytes. */
    static const size_t maxRecordSize = 0xFFFF;

    VramStore(uint32_t baseAddress, size_t capacity);
    ~VramStore() = default;

    /** The driver is assigned separately because the store is constructed
     *  before the display exists. */
    void assignStorageDriver(VideoRam *newDriver);

    /** Discards every record. Handles issued before this become meaningless,
     *  so call it only when nothing holds one: preset teardown, not mid-use. */
    void reset(void);

    /**
     * @brief Copies data into the store.
     *
     * @return a handle to read it back with, or invalidHandle if it did not
     *         fit or the store has no driver yet.
     */
    Handle store(const void *data, size_t size);

    /** Stores a string, terminator included. A null or empty string is
     *  stored as an empty record rather than being refused. */
    Handle storeString(const char *text);

    /**
     * @brief Copies a record back out.
     *
     * Reads at most what the record holds and at most what the buffer can
     * take, so an oversized buffer costs nothing.
     *
     * @return the number of bytes written into the buffer, zero if the handle
     *         is not valid.
     */
    size_t load(Handle handle, void *buffer, size_t bufferSize) const;

    /**
     * @brief Copies a stored string into the buffer.
     *
     * The result is always terminated, even when the record is longer than
     * the buffer or the handle is invalid.
     *
     * @return the buffer, so it can be used directly in a call.
     */
    const char *loadString(Handle handle, char *buffer, size_t bufferSize) const;

    /** Length of a stored record, or zero if the handle is not valid. */
    size_t sizeOf(Handle handle) const;

    size_t getCapacity(void) const;
    size_t getUsed(void) const;
    size_t getRemaining(void) const;

    /** Records refused because the store was full. Non-zero means data was
     *  lost and whatever was meant to show it will be blank. */
    uint32_t getRejectedCount(void) const;

private:
    /** Every record starts with its length, so a reader needs no agreement
     *  with the writer about sizes. */
    static const size_t headerSize = 2;

    /** Records up to this size are fetched in a single transfer through a
     *  stack buffer. Anything larger is read straight into the caller's
     *  buffer after its length, which costs a second transfer but bounds the
     *  stack. Every current user is far below this. */
    static const size_t inlineReadLimit = 128;

    VideoRam *driver;
    uint32_t baseAddress;
    size_t capacity;
    size_t used;
    uint32_t rejected;
};
