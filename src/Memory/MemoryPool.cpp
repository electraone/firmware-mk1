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
 * @file MemoryPool.cpp
 *
 * @brief A representation of the E1 style memory pool for MemoryBlocks.
 */

#include "MemoryPool.h"

/**
 * @brief Space a new block wants to see ahead of it before it starts.
 *
 * A block is written as it arrives, so its length is not known when it is
 * opened and it cannot be split around the end of the pool. Beginning one
 * only while this much room is left means an ordinary message never reaches
 * the end; one larger than this is truncated, and says so, rather than being
 * written past the end of the page.
 *
 * A quarter of the pool at most, so that the rule still makes sense for a
 * pool smaller than the figure below rather than sending it back to the
 * start on every message.
 */
static size_t reserveFor(size_t capacity)
{
    static const size_t largestOrdinaryMessage = 64 * 1024;

    size_t reserve = capacity / 4;

    return ((reserve > largestOrdinaryMessage) ? largestOrdinaryMessage
                                               : reserve);
}

MemoryPool::MemoryPool(Graphics *newFb, uint32_t address, size_t capacity)
    : fb(newFb),
      baseAddress(address),
      capacity(capacity),
      currentAddress(0),
      currentLength(0),
      opened(false),
      overflowed(0),
      cacheStart(0),
      cacheLength(0)
{
}

MemoryPool::MemoryPool(uint32_t address, size_t capacity)
    : fb(nullptr),
      baseAddress(address),
      capacity(capacity),
      currentAddress(0),
      currentLength(0),
      opened(false),
      overflowed(0),
      cacheStart(0),
      cacheLength(0)
{
}

MemoryPool::~MemoryPool()
{
}

void MemoryPool::assignStorageDriver(Graphics *newFb)
{
    fb = newFb;
}

MemoryBlock MemoryPool::openMemoryBlock(void)
{
    return (MemoryBlock(this, openAddress()));
}

uint32_t MemoryPool::openAddress(void)
{
    opened = true;

    /* Wrapping happens here, where a block begins, rather than after one has
     * already been written. Deciding afterwards meant the block that ran off
     * the end had been written there first, and what follows this pool is the
     * page the labels live in. */
    if ((currentAddress + reserveFor(capacity)) > capacity) {
        currentAddress = 0;
        invalidateCache();
    }

    return (currentAddress);
}

uint32_t MemoryPool::getOverflowCount(void) const
{
    return (overflowed);
}

void MemoryPool::invalidateCache(void)
{
    cacheLength = 0;
}

uint32_t MemoryPool::getBaseAddress(void)
{
    return (baseAddress);
}

bool MemoryPool::write(const uint8_t *buffer, size_t length)
{
    if ((opened == false) || (fb == nullptr)) {
        return (false);
    }

    /* Nothing is ever written past the end of the pool. A message that would
     * need it is cut short here, which loses that message; letting the write
     * through would quietly corrupt whatever the next page holds. */
    if ((currentAddress + length) > capacity) {
        overflowed++;
        return (false);
    }

    fb->writeRamData(baseAddress + currentAddress, buffer, length);

    /* What was just written may be sitting in the read window. */
    if ((cacheLength > 0) && (currentAddress < (cacheStart + cacheLength))
        && ((currentAddress + length) > cacheStart)) {
        invalidateCache();
    }

    currentAddress += length;
    currentLength += length;

    return (true);
}

int MemoryPool::readByte(uint32_t address)
{
    if ((fb == nullptr) || (address >= capacity)) {
        return (-1);
    }

    if ((cacheLength == 0) || (address < cacheStart)
        || (address >= (cacheStart + cacheLength))) {
        size_t toRead = cacheSize;

        if ((address + toRead) > capacity) {
            toRead = capacity - address;
        }

        fb->readRamData(baseAddress + address, cache, toRead);
        cacheStart = address;
        cacheLength = toRead;
    }

    return (cache[address - cacheStart]);
}

uint32_t MemoryPool::saveString(const char *text)
{
    noInterrupts();
    uint32_t address = openAddress();

    write((uint8_t *)text, strlen(text) + 1);
    close();
    interrupts();

    return (address);
}

uint32_t MemoryPool::saveItem(uint32_t bitmap, const char *text)
{
    noInterrupts();
    uint32_t address = openAddress();

    write((uint8_t *)&bitmap, 4);
    write((uint8_t *)text, strlen(text) + 1);
    close();
    interrupts();

    return (address);
}

size_t MemoryPool::getString(uint32_t address, char *buffer, size_t maxLength)
{
    fb->readRamData(baseAddress + address, (uint8_t *)buffer, maxLength);
    buffer[maxLength - 1] = '\0';

    return (strlen(buffer));
}

void MemoryPool::readData(uint32_t address, uint8_t *buffer, size_t maxLength)
{
    fb->readRamData(baseAddress + address, buffer, maxLength);
}

size_t MemoryPool::getItem(uint32_t address,
                           char *buffer,
                           size_t maxLength,
                           uint32_t *bitmap)
{
    uint8_t tmpBuffer[4 + maxLength];

    fb->readRamData(baseAddress + address, (uint8_t *)tmpBuffer, maxLength + 4);
    *bitmap = *((uint32_t *)tmpBuffer);
    copyString(buffer, (const char *)(tmpBuffer + 4), maxLength - 1);

    return (strlen(buffer));
}

void MemoryPool::close(void)
{
    currentLength = 0;
    opened = false;
}
