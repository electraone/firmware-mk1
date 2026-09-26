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
 * @file Md5.h
 *
 * @brief RFC 1321, for checking that a transferred file arrived whole.
 *
 * Here because the file transfer API asks for one: a host commits a staged
 * upload with a digest of what it sent, and the controller has to be able to
 * say whether what it has matches. It is a checksum in this firmware and
 * nothing else - MD5 has been broken for signatures since 2004 and must not
 * be used for one.
 *
 * Written for a device with a quarter of a megabyte and no hash unit: the
 * state is a hundred bytes, taken a block at a time so a file of any size
 * costs the same, and the rounds are a loop over a table rather than sixty-four
 * lines of macro - which is about a kilobyte of flash rather than four, and
 * still faster by a wide margin than reading the same bytes off the card.
 */

#pragma once

#include <cstddef>
#include <cstdint>

class Md5
{
public:
    /** Bytes of digest, and characters of the hexadecimal form. */
    static constexpr size_t digestLength = 16;
    static constexpr size_t hexLength = 32;

    Md5();

    /** Adds bytes. Any number of calls, of any length, are the same as one. */
    void update(const uint8_t *data, size_t length);

    /**
     * @brief Finishes the digest.
     *
     * The object is not usable afterwards: make another one for another file.
     */
    void finish(uint8_t digest[digestLength]);

    /**
     * @brief The digest as lower case hexadecimal, terminated.
     *
     * @param size the buffer, which needs hexLength + 1
     */
    static void
        toHex(const uint8_t digest[digestLength], char *hex, size_t size);

    /**
     * @brief The digest of a file on the card.
     *
     * Reads the file rather than remembering what was written to it, which is
     * the point: what a commit has to check is the bytes that are actually on
     * the card and about to be moved into place, not the ones that went past
     * on their way there.
     *
     * @param hex where the hexadecimal digest is left; needs hexLength + 1
     * @return false when the file cannot be read
     */
    static bool ofFile(const char *path, char *hex, size_t size);

    /** Whether two hexadecimal digests match, ignoring case. */
    static bool hexEquals(const char *a, const char *b);

private:
    void transform(const uint8_t block[64]);

    uint32_t state[4];
    uint64_t bytes;
    uint8_t buffer[64];
    size_t buffered;
};
