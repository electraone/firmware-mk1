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
 * @brief How a link message leaves this unit.
 *
 * The unit sits in the primary's USB host socket, so its own USB device port
 * is the cable facing the primary, and the CTRL cable of it is where the
 * primary listens. Nothing to choose: every message goes there.
 *
 * The framing is done here, into a buffer in main memory, and handed to the
 * USB stack in one call. The controller frames its outgoing SysEx through
 * the pool in the display's memory, which is the right place for a preset
 * but two SPI transfers too many for a ping.
 */

#pragma once

#include "src/Link/LinkTransport.h"

class LinkTransport : public Link::Transport
{
public:
    LinkTransport() = default;

    void sendLink(uint8_t opcode,
                  const uint8_t *payload,
                  size_t length) override;

    /** Messages sent since boot, for the status line. */
    uint32_t getMessagesSent(void) const
    {
        return (sent);
    }

private:
    uint32_t sent = 0;
};
