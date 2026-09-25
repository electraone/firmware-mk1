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
 * @file SysexBlock.cpp
 *
 * @brief A representation of a Sysex message of memory in E1 style
 *  memory pool.
 */

#include "SysexBlock.h"
#include "MemoryPool.h"

SysexBlock::SysexBlock()
{
}

SysexBlock::SysexBlock(MemoryPool *memoryPool, uint32_t address)
    : MemoryBlock(memoryPool, address)
{
}

SysexBlock::SysexBlock(const MemoryBlock &memoryBlock)
    : MemoryBlock(memoryBlock)
{
}

SysexBlock::~SysexBlock()
{
}

bool SysexBlock::isSysex(void) const
{
    /* The length is checked first. A block that holds nothing has nothing at
     * header[0] to ask about. */
    return ((length > 0) && (header[0] == 0xF0));
}

bool SysexBlock::isEmpty(void) const
{
    return (length < 3);
}

uint32_t SysexBlock::getManufacturerSysexId(void) const
{
    if (length < 2) {
        return (0);
    }

    /* A leading zero means a three byte identifier follows, so there have to
     * be three bytes there. The old code settled for two and read the third
     * from beyond the message. */
    if (header[1] == 0) {
        if (length < 4) {
            return (0);
        }
        return (((uint32_t)header[2] << 8) | header[3]);
    }

    return (header[1]);
}

bool SysexBlock::isElectraSysex(void) const
{
    uint32_t manufacturerSysexId = getManufacturerSysexId();

    return (manufacturerSysexId == electraSroManufacturer);
}

ElectraCommand SysexBlock::getElectraSysexCommand(void) const
{
    ElectraCommand electraCommand;

    if (getManufacturerSysexId() != electraSroManufacturer) {
        return (electraCommand);
    }

    /* Six bytes are read, so six bytes have to be there. A message that
     * stops early gives a command built from zeros rather than from whatever
     * followed it in memory. */
    uint8_t bytes[6] = {};

    for (size_t i = 0; (i < 6) && ((commandOffset + i) < length); i++) {
        bytes[i] = header[commandOffset + i];
    }

    electraCommand.set(
        bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);

    return (electraCommand);
}

MemoryBlock SysexBlock::getElectraSysexPayload(void) const
{
    /* Past the manufacturer identifier, then past the command and the object
     * byte. A non Electra message is treated as having a single byte
     * identifier, which is what this did before. */
    uint32_t offset =
        (getManufacturerSysexId() == electraSroManufacturer)
            ? (commandOffset + 2)
            : 2;

    /* The trailing 0xF7 is not payload. Working the length out by subtraction
     * wrapped round on a message too short to have a payload at all, and the
     * range that came back ran to four gigabytes of video memory. */
    if (length < (offset + 1)) {
        return (MemoryBlock());
    }

    return (getRange(offset, length - offset - 1));
}
