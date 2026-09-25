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

#pragma once

#include "helpers.h"
#include "MemoryBlock.h"
#include "ElectraCommand.h"

class SysexBlock : public MemoryBlock
{
public:
    SysexBlock();
    SysexBlock(MemoryPool *memoryPool, uint32_t address);
    SysexBlock(const MemoryBlock &memoryBlock);
    ~SysexBlock();

    bool isSysex(void) const;
    bool isEmpty(void) const;
    uint32_t getManufacturerSysexId(void) const;
    bool isElectraSysex(void) const;
    ElectraCommand getElectraSysexCommand(void) const;
    MemoryBlock getElectraSysexPayload(void) const;

private:
    // \todo this needs to be deduplicated
    static const uint32_t nonProfitManufacturer = 0x7d0000;
    static const uint32_t electraSroManufacturer = 0x002145;

    /** Where the command byte sits: past 0xF0 and the three byte
     *  manufacturer identifier. */
    static const size_t commandOffset = 4;
};
