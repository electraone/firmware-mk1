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
 * @file Md5.cpp
 *
 * @brief RFC 1321.
 */

#include "Md5.h"
#include "Hardware.h"
#include "System.h"

#include <cstring>

namespace
{
    /** The integer parts of 4294967296 times abs(sin(i)), as the RFC has it. */
    const uint32_t K[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
        0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
        0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
        0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
        0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
        0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
    };

    /** The rotation of each step, four per round. */
    const uint8_t S[16] = { 7, 12, 17, 22, 5, 9,  14, 20,
                            4, 11, 16, 23, 6, 10, 15, 21 };

    inline uint32_t rotateLeft(uint32_t value, uint8_t by)
    {
        return ((value << by) | (value >> (32 - by)));
    }

    /** How much of a file is read at a time: one card block. */
    constexpr size_t readChunk = 512;
} // namespace

Md5::Md5() : bytes(0), buffered(0)
{
    state[0] = 0x67452301;
    state[1] = 0xefcdab89;
    state[2] = 0x98badcfe;
    state[3] = 0x10325476;
}

void Md5::transform(const uint8_t block[64])
{
    /* Copied rather than cast: the block can come straight out of a caller's
     * buffer at any alignment, and a word load off an odd address is a fault
     * on this core. The compiler turns this into word moves when it can. */
    uint32_t m[16];
    memcpy(m, block, sizeof(m));

    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];

    for (uint8_t i = 0; i < 64; i++) {
        uint32_t f;
        uint8_t g;

        /* One loop rather than sixty-four unrolled steps. The four rounds
         * differ only in the mixing function and in which word of the block
         * they take, and a Cortex-M4 spends far longer waiting for the card
         * that the bytes came off than it saves by unrolling this. */
        if (i < 16) {
            f = (b & c) | (~b & d);
            g = i;
        } else if (i < 32) {
            f = (d & b) | (~d & c);
            g = ((5 * i) + 1) & 0x0f;
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = ((3 * i) + 5) & 0x0f;
        } else {
            f = c ^ (b | ~d);
            g = (7 * i) & 0x0f;
        }

        const uint32_t temp = d;

        d = c;
        c = b;
        b = b
            + rotateLeft(a + f + K[i] + m[g], S[((i >> 4) << 2) | (i & 0x03)]);
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

void Md5::update(const uint8_t *data, size_t length)
{
    if ((data == nullptr) || (length == 0)) {
        return;
    }

    bytes += length;

    /* Fill what is already held first, so that a caller handing over the file
     * in pieces of any size gets the same answer as one handing it over whole. */
    if (buffered > 0) {
        size_t wanted = sizeof(buffer) - buffered;

        if (wanted > length) {
            wanted = length;
        }

        memcpy(buffer + buffered, data, wanted);
        buffered += wanted;
        data += wanted;
        length -= wanted;

        if (buffered < sizeof(buffer)) {
            return;
        }

        transform(buffer);
        buffered = 0;
    }

    while (length >= sizeof(buffer)) {
        transform(data);
        data += sizeof(buffer);
        length -= sizeof(buffer);
    }

    if (length > 0) {
        memcpy(buffer, data, length);
        buffered = length;
    }
}

void Md5::finish(uint8_t digest[digestLength])
{
    const uint64_t bits = bytes * 8;

    /* A one bit, then zeros to fifty-six of the block, then the length. */
    static const uint8_t padding[64] = { 0x80 };

    size_t padLength = (buffered < 56) ? (56 - buffered) : (120 - buffered);

    /* update() counts what it is given, and the padding is not part of the
     * message, so the count is put back afterwards. */
    update(padding, padLength);
    bytes -= padLength;

    uint8_t tail[8];

    for (uint8_t i = 0; i < 8; i++) {
        tail[i] = (uint8_t)(bits >> (8 * i));
    }

    update(tail, sizeof(tail));

    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 4; j++) {
            digest[(i * 4) + j] = (uint8_t)(state[i] >> (8 * j));
        }
    }
}

void Md5::toHex(const uint8_t digest[digestLength], char *hex, size_t size)
{
    static const char digits[] = "0123456789abcdef";

    if ((hex == nullptr) || (size < (hexLength + 1))) {
        if ((hex != nullptr) && (size > 0)) {
            hex[0] = '\0';
        }
        return;
    }

    for (size_t i = 0; i < digestLength; i++) {
        hex[i * 2] = digits[digest[i] >> 4];
        hex[(i * 2) + 1] = digits[digest[i] & 0x0f];
    }

    hex[hexLength] = '\0';
}

bool Md5::ofFile(const char *path, char *hex, size_t size)
{
    if ((path == nullptr) || (hex == nullptr) || (size < (hexLength + 1))) {
        return (false);
    }

    FileIoStream file = Hardware::sdcard.createInputStream(path);

    if (!file) {
        System::logger.write(
            LOG_ERROR, "Md5::ofFile: cannot read the file: %s", path);
        return (false);
    }

    Md5 md5;
    uint8_t chunk[readChunk];

    for (;;) {
        const int read = file.read(chunk, sizeof(chunk));

        if (read < 0) {
            System::logger.write(
                LOG_ERROR, "Md5::ofFile: read failed: %s", path);
            file.close();
            return (false);
        }

        if (read == 0) {
            break;
        }

        md5.update(chunk, (size_t)read);
    }

    file.close();

    uint8_t digest[digestLength];

    md5.finish(digest);
    toHex(digest, hex, size);

    return (true);
}

bool Md5::hexEquals(const char *a, const char *b)
{
    if ((a == nullptr) || (b == nullptr)) {
        return (false);
    }

    /* Case insensitive on purpose: the digest is text on the wire, and a host
     * is free to send it in either case. */
    for (size_t i = 0; i < hexLength; i++) {
        char left = a[i];
        char right = b[i];

        if ((left >= 'A') && (left <= 'F')) {
            left += 'a' - 'A';
        }
        if ((right >= 'A') && (right <= 'F')) {
            right += 'a' - 'A';
        }

        if (left != right) {
            return (false);
        }
    }

    return ((a[hexLength] == '\0') && (b[hexLength] == '\0'));
}
