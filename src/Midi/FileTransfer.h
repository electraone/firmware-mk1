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
 * @file FileTransfer.h
 *
 * @brief Naming, listing and fetching files over SysEx.
 *
 * The reading half of the File Transfer API the host tools already speak:
 * https://docs.electra.one/developers/filetransfer.html
 *
 * A file is named by a descriptor rather than by a path - `{"location":
 * "tmp", "type": "screenshot", "name": "scrn000"}` - so that a host never
 * spells out where the firmware keeps anything, and the firmware can move it.
 * Only the locations and types this actually serves are accepted; a
 * descriptor naming anything else is refused rather than guessed at, because
 * a guess here is a path.
 *
 * Enough of the API to get a screenshot off the unit, and no more: list a
 * location, fetch a file, remove a file. Writing is the other half and the
 * preset transfer already has its own.
 */

#pragma once

#include <cstddef>
#include <cstdint>

class MemoryBlock;

namespace FileTransfer
{
    /** The longest path this can name. */
    constexpr size_t maxPathLength = 64;

    /**
 * @brief Turns a descriptor into a path on the card.
 *
 * @param path where the path is left
 * @param size the size of that buffer
 * @param descriptor the JSON object the host sent
 * @return false when the descriptor names a location or a type that is not
 *         served, or leaves out a name where one is needed
 */
    bool resolve(char *path, size_t size, MemoryBlock &descriptor);

    /**
 * @brief The directory a descriptor's location names, with no file in it.
 *
 * A listing wants only the location, so this takes the descriptor the
 * `Location` object comes with rather than the `File` one.
 */
    bool resolveLocation(char *path, size_t size, MemoryBlock &descriptor);

    /** Answers with the files in the directory, as the API spells a listing. */
    void sendListing(uint8_t port, const char *directory);

    /**
 * @brief Sends the file, base64 encoded, then leaves the caller to ACK.
 *
 * Encoded because SysEx is seven bit and the first byte above 0x7F in a
 * picture would read as a status byte and end the message early.
 */
    bool sendFile(uint8_t port, const char *path);

    /**
 * @brief Empties a directory, leaving the directory itself.
 *
 * What the API calls removing the files of a location. Only the files: a
 * location this serves has nothing below it, and a recursive delete driven
 * from the wire is not something to have unless something needs it.
 *
 * @return false when the directory is not there at all, or a file in it could
 *         not be removed.
 */
    bool removeLocation(const char *directory);

    /** Makes the directory. Answers true when it is already there. */
    bool createDirectory(const char *directory);

    /** Removes the directory. Refuses a directory with anything in it. */
    bool removeDirectory(const char *directory);

} // namespace FileTransfer
