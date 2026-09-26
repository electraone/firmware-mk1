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
 * @file StagedUpload.h
 *
 * @brief The writing half of the File Transfer API: the cache transaction.
 *
 * https://docs.electra.one/developers/filetransfer.html
 *
 * A host opens a transaction, says how big each file it is about to send is,
 * sends each one in chunks, and then commits with a digest of each. Nothing
 * reaches its destination until the commit, and the commit applies either all
 * of the files or none of them.
 *
 * That is what it is for. A preset and the script that goes with it are one
 * thing, and writing them one message at a time leaves a window in which the
 * slot holds a new preset and the old script. The preset upload message on its
 * own cannot close that window - it deletes the slot's script, so the script
 * has to follow it - and it can only ever write the slot that is selected. A
 * transaction writes any slot, both files, at once, and leaves the slot
 * untouched if either of them did not arrive whole.
 *
 * The staging area is a directory on the card rather than memory: a preset is
 * tens of kilobytes and this device has a quarter of a megabyte.
 */

#pragma once

#include "ElectraCommand.h"

#include <cstddef>
#include <cstdint>

class MemoryBlock;
class SysexBlock;

namespace StagedUpload
{
    /**
     * @brief How many files one transaction may carry.
     *
     * A preset, its script, and room to spare. Each one costs a dozen bytes
     * of the table below for as long as the transaction is open.
     */
    constexpr uint8_t maxFiles = 8;

    /** Starts a transaction, throwing away anything a previous one left. */
    void open(uint8_t port);

    /** Announces one file's id and the number of bytes it will carry. */
    void registerFile(uint8_t port, ElectraCommand &cmd);

    /** Takes one chunk of one file, and reports the total received so far. */
    void receiveChunk(uint8_t port, const SysexBlock &sysexBlock);

    /**
     * @brief Checks every file and then moves them all into place.
     *
     * Nothing is applied unless every file in the commit is complete and
     * matches the digest the host sent for it.
     */
    void commit(uint8_t port, MemoryBlock &sysexPayload);

    /** Forgets an open transaction and empties the staging area. */
    void discard(void);

} // namespace StagedUpload
