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
 * @file LinkTransport.h
 *
 * @brief How a satellite's messages leave it.
 *
 * A link client composes records and knows nothing about how they travel. On
 * a computer that is RtMidi and a SysEx message assembled by hand; on an
 * Electra One it is the firmware's own MIDI output, which already frames and
 * encodes an outgoing link message. Neither is the client's business, and the
 * two would otherwise be two copies of the same framing.
 *
 * The interface is at the level of a *message*, not of bytes: an opcode and a
 * payload. Everything below that - the manufacturer header, the base64, the
 * cable it goes out on - belongs to whichever program is doing the sending,
 * because each already has code for it.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace Link
{
    class Transport
    {
    public:
        virtual ~Transport() = default;

        /**
         * @brief Send one link message.
         *
         * @param opcode what the message is
         * @param payload the composed record, unencoded, or nullptr
         * @param length its length in bytes
         */
        virtual void
            sendLink(uint8_t opcode, const uint8_t *payload, size_t length) = 0;
    };
} // namespace Link
