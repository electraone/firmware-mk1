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
 * @file MemoryPool.h
 *
 * @brief A representation of the E1 style memory pool for MemoryBlocks.
 *
 * This holds system exclusive messages, and nothing else. It used to carry a
 * second interface for keeping strings and list items here as well; VramStore
 * does that now, and does it with records that know their own length, so
 * those went with it.
 */

#pragma once

#include "MemoryBlock.h"
#include "helpers.h"

class MemoryPool
{
public:
    MemoryPool(Graphics *newFb, uint32_t address, size_t capacity);
    MemoryPool(uint32_t address, size_t capacity);
    ~MemoryPool();
    void assignStorageDriver(Graphics *newFb);
    MemoryBlock openMemoryBlock(void);
    uint32_t openAddress(void);

    /** Blocks refused because the pool had no room left for them. Non-zero
     *  means a message was truncated, which is a great deal better than the
     *  overrun into the next page it used to be. */
    uint32_t getOverflowCount(void) const;

private:
    Graphics *fb;
    uint32_t getBaseAddress(void);
    bool write(const uint8_t *buffer, size_t length);
    void close(void);

    /**
     * @brief One byte of the pool, served from a cached window where it can be.
     *
     * Everything that examines a received message walks it a byte at a time:
     * matching a device's header, extracting parameters, printing it to the
     * logger. Each of those bytes used to be its own transfer over the SPI
     * bus, which costs far more in fixed overhead than in the byte it
     * carries. Reading a window around the byte instead turns a scan of a
     * kilobyte from a thousand transfers into four.
     *
     * The window is invalidated whenever anything is written to the pool, so
     * it can never hand back a byte that has since been replaced.
     */
    int readByte(uint32_t address);

    /**
     * @brief Replaces one byte of the pool, in place.
     *
     * write() only ever appends, at the pool's own cursor, which is what a
     * message being received needs. Patching a byte of a message already
     * stored - the one thing sysexBlock:poke() does - needs an address
     * instead.
     *
     * @return false when the address lies outside the pool.
     */
    bool writeByte(uint32_t address, uint8_t byte);

    void invalidateCache(void);

    /** Enough to cover a typical device response in one go, small enough
     *  that a miss costs little. One of these for the whole pool, not one
     *  per block: blocks are copied about freely and must stay cheap. */
    static const size_t cacheSize = 256;

    uint32_t baseAddress;
    size_t capacity;

    uint32_t currentAddress;
    uint32_t currentLength;
    bool opened;
    uint32_t overflowed;

    uint8_t cache[cacheSize];
    uint32_t cacheStart;
    size_t cacheLength;

    friend class MemoryBlock;
};
