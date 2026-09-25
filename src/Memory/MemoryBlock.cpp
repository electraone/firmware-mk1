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
 * @file MemoryBlock.cpp
 *
 * @brief A representation of a chunk of memory in E1 style memory pool.
 */

#include "MemoryBlock.h"
#include "MemoryPool.h"
#include <cstring>

MemoryBlock::MemoryBlock()
    : memoryPool(nullptr),
      startAddress(0),
      currentAddress(0),
      endAddress(0),
      length(0),
      headerLength(0),
      headerFlushed(false),
      overflowed(false)
{
    /* Cleared because callers ask things of an empty block: whether it is a
     * system exclusive message, what manufacturer it carries. Those questions
     * used to be answered from whatever the memory happened to hold. */
    memset(header, 0, sizeof(header));
}

MemoryBlock::MemoryBlock(MemoryPool *newMemoryPool, uint32_t newAddress)
    : memoryPool(newMemoryPool),
      startAddress(newAddress),
      currentAddress(newAddress),
      endAddress(newAddress),
      length(0),
      headerLength(0),
      headerFlushed(false),
      overflowed(false)
{
    memset(header, 0, sizeof(header));
}

MemoryBlock::~MemoryBlock()
{
}

void MemoryBlock::seek(uint32_t address)
{
    currentAddress = startAddress + address;
}

uint32_t MemoryBlock::tell(void) const
{
    return (currentAddress - startAddress);
}

int MemoryBlock::read()
{
    int byte = -1;

    if (memoryPool == nullptr) {
        return (-1);
    }

    if (currentAddress < endAddress) {
        size_t bytePosition = tell(); //currentAddress - startAddress;

        if (bytePosition < headerLength) {
            byte = header[bytePosition];
        } else {
            byte = memoryPool->readByte(currentAddress);
        }

        currentAddress++;
    } else {
        byte = -1;
    }

    return (byte);
}

size_t MemoryBlock::readBytes(uint8_t *buffer, size_t length)
{
    size_t totalReadBytes = 0;
    size_t readBytes = 0;
    size_t offset = 0;

    while ((readBytes = readBytesInternal(buffer + offset, length - offset))
           != 0) {
        offset += readBytes;
        totalReadBytes += readBytes;
    }
    return (totalReadBytes);
}

size_t MemoryBlock::writeBytes(const uint8_t *buffer, size_t lengthToWrite)
{
    if ((memoryPool == nullptr) || (buffer == nullptr) || overflowed) {
        return (0);
    }

    size_t stored = 0;

    /* The first bytes of a message are kept in RAM as well as in the pool, so
     * that what every reader looks at first - the manufacturer, the command,
     * a device's header - costs no transfer at all. A message that fits here
     * entirely never reaches the pool. */
    if (headerLength < headerMaxSize) {
        size_t toHeader = headerMaxSize - headerLength;

        if (toHeader > lengthToWrite) {
            toHeader = lengthToWrite;
        }

        memcpy(header + headerLength, buffer, toHeader);
        headerLength += toHeader;
        stored = toHeader;

        if (headerLength < headerMaxSize) {
            /* Still room, so the message may yet turn out to fit in the
             * header. Nothing goes to the pool until it is clear it does not. */
            length += stored;
            currentAddress += stored;
            endAddress = startAddress + length;
            return (stored);
        }

        if (headerFlushed == false) {
            headerFlushed = true;

            if (memoryPool->write(header, headerMaxSize) == false) {
                /* The header bytes are readable from RAM either way, so they
                 * count; nothing beyond them can be kept. */
                overflowed = true;
                length += stored;
                currentAddress += stored;
                endAddress = startAddress + length;
                return (stored);
            }
        }
    }

    size_t remaining = lengthToWrite - stored;

    if (remaining > 0) {
        if (memoryPool->write(buffer + stored, remaining)) {
            stored += remaining;
        } else {
            /* Reporting a short write is what keeps a reader from being told
             * about bytes that were never stored, and then being handed
             * whatever the pool held there before. */
            overflowed = true;
        }
    }

    length += stored;
    currentAddress += stored;
    endAddress = startAddress + length;

    return (stored);
}

void MemoryBlock::close(void)
{
    seek(0);

    if (this->memoryPool) {
        this->memoryPool->close();
    }
}

size_t MemoryBlock::getLength(void) const
{
    return (length);
}

MemoryBlock MemoryBlock::getRange(uint32_t rangeStart,
                                  uint32_t rangeLength) const
{
    MemoryBlock newBlock;

    if (memoryPool == nullptr) {
        return (newBlock);
    }

    /* Clamped to what this block actually holds. A caller working out a
     * payload length by subtraction can arrive at a nonsense range from a
     * malformed message, and a range reaching past the end would hand back
     * whatever the pool held there. */
    if (rangeStart > length) {
        rangeStart = length;
    }
    if (rangeLength > (length - rangeStart)) {
        rangeLength = length - rangeStart;
    }

    newBlock.memoryPool = memoryPool;
    newBlock.startAddress = startAddress + rangeStart;
    newBlock.currentAddress = newBlock.startAddress;
    newBlock.length = rangeLength;
    newBlock.endAddress = newBlock.startAddress + newBlock.length;
    newBlock.headerLength =
        (rangeLength > headerMaxSize) ? headerMaxSize : rangeLength;

    /* A range is read, never written, so its header is already as flushed as
     * it will ever be. */
    newBlock.headerFlushed = true;

    /* Taken a byte at a time, which costs nothing: it is at most 56 bytes,
     * they come either from this block's own header in RAM or from a single
     * cached window of the pool. The old code always transferred, even when
     * the whole message was sitting in the header. */
    for (size_t i = 0; i < newBlock.headerLength; i++) {
        size_t sourcePosition = rangeStart + i;

        if (sourcePosition < headerLength) {
            newBlock.header[i] = header[sourcePosition];
        } else {
            newBlock.header[i] =
                (uint8_t)memoryPool->readByte(startAddress + sourcePosition);
        }
    }

    return (newBlock);
}

bool MemoryBlock::isEmpty(void) const
{
    return (length == 0 ? true : false);
}

bool MemoryBlock::isAllocatedInPool(void) const
{
    return (memoryPool != nullptr ? true : false);
}

bool MemoryBlock::isOverflowed(void) const
{
    return (overflowed);
}

int MemoryBlock::peek(size_t position) const
{
    if (memoryPool == nullptr) {
        return (-1);
    }

    /* Past the end is -1, which is what every caller already tests for and
     * what none of them used to get: a byte read from beyond the message was
     * handed back as though the message contained it. */
    if (position >= length) {
        return (-1);
    }

    if (position < headerLength) {
        return (header[position]);
    }

    return (memoryPool->readByte(startAddress + position));
}

size_t MemoryBlock::readBytesInternal(uint8_t *buffer, size_t length)
{
    if (memoryPool == nullptr) {
        return (0);
    }

    uint16_t bytesToRead = 0;

    size_t currentPosition = tell();

    if (currentPosition < headerLength) {
        bytesToRead = (headerLength - currentPosition) > length
                          ? length
                          : headerLength - currentPosition;

        if (bytesToRead) {
            memcpy((void *)buffer, header + currentPosition, bytesToRead);
            currentAddress += bytesToRead;
        }
    } else {
        bytesToRead = (endAddress - currentAddress) >= length
                          ? length
                          : (endAddress - currentAddress);

        if (bytesToRead) {
            memoryPool->fb->readRamData(memoryPool->getBaseAddress()
                                            + currentAddress,
                                        buffer,
                                        bytesToRead);
            currentAddress += bytesToRead;
        }
    }

    return (bytesToRead);
}
