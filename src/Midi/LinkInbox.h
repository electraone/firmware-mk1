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
 * @file LinkInbox.h
 *
 * @brief Where incoming Electra Link messages wait for the application.
 *
 * Every other Electra SysEx message arriving on the CTRL cable is assembled
 * into the SysEx pool, which lives in the display's SDRAM on the far side of
 * the SPI bus: written from the MIDI interrupt as the packets land, read back
 * on the main loop when the message is handled. That is the right home for a
 * preset upload, which is far too big for main memory. A link message is at
 * most 433 bytes, arrives up to a hundred times a second while an Electra Link is
 * following a knob, and would otherwise cost two SPI transfers each - the
 * first of them inside an interrupt, competing with whatever the main loop is
 * drawing.
 *
 * So the link has a ring of its own in main memory. The interrupt writes each
 * message into the slot at the head as its packets arrive, and commits it with
 * one store; the application takes slots from the tail. One producer, one
 * consumer, and each index written by one side only, so no masking is needed.
 *
 * Nothing is allocated until an application asks for the ring: the controller
 * never does, and pays nothing for it.
 */

#pragma once

#include <cstddef>
#include <cstdint>

class LinkInbox
{
public:
    /** The most bytes one message holds after the SysEx header: the opcode
     *  and the base64-encoded payload. Link::maxEncodedLength plus one, kept
     *  as a number here so the interrupt side needs no protocol headers. */
    static constexpr uint16_t MaxMessageLength = 436;

    struct Message {
        uint8_t port;
        uint16_t length;
        uint8_t bytes[MaxMessageLength];
    };

    /**
     * @brief Allocate the ring and start taking link messages.
     *
     * @param depth how many messages may wait, at most 64
     * @return false when the memory could not be had
     */
    static bool enable(uint8_t depth);

    static bool isEnabled(void);

    /*
     * The interrupt side. begin() claims the head slot for the message being
     * received, append() adds a packet's bytes, commit() publishes it and
     * abandon() gives it up. A message longer than a slot, or arriving when
     * the ring is full, is dropped and counted.
     */
    static void begin(uint8_t port);
    static void append(const uint8_t *data, uint16_t length);
    static void commit(void);
    static void abandon(void);

    /*
     * The application side. peek() answers the oldest message, which stays
     * valid until release() is called for it.
     */
    static const Message *peek(void);
    static void release(void);

    /** Messages dropped for want of room, since boot. */
    static uint32_t getDropped(void);

    /**
     * @brief Note a SysEx that was not a link message and was refused.
     *
     * The interrupt reads the USB port through a repaint only while the
     * inbox is up, and a message that would have gone into the pool across
     * the display's bus at that moment is thrown away instead. Counted here
     * because this is the one place both handlers already reach.
     */
    static void countRefused(void);

    /** Such messages refused, since boot. */
    static uint32_t getRefused(void);

    /** The most messages that have waited at once, since boot. */
    static uint8_t getHighWater(void);

private:
    static Message *slots;
    static uint8_t depth;
    static volatile uint8_t head;
    static volatile uint8_t tail;
    static bool receiving;
    static bool overflowed;
    static uint32_t dropped;
    static uint32_t refused;
    static uint8_t highWater;
};
