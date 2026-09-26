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

#include "LinkInbox.h"
#include <cstring>
#include <new>

LinkInbox::Message *LinkInbox::slots = nullptr;
uint8_t LinkInbox::depth = 0;
volatile uint8_t LinkInbox::head = 0;
volatile uint8_t LinkInbox::tail = 0;
bool LinkInbox::receiving = false;
bool LinkInbox::overflowed = false;
uint32_t LinkInbox::dropped = 0;
uint32_t LinkInbox::refused = 0;
uint8_t LinkInbox::highWater = 0;

bool LinkInbox::enable(uint8_t newDepth)
{
    if (slots != nullptr) {
        return (true);
    }

    if (newDepth < 2) {
        newDepth = 2;
    } else if (newDepth > 64) {
        newDepth = 64;
    }

    slots = new (std::nothrow) Message[newDepth];

    if (slots == nullptr) {
        return (false);
    }

    depth = newDepth;
    head = 0;
    tail = 0;

    return (true);
}

bool LinkInbox::isEnabled(void)
{
    return (slots != nullptr);
}

void LinkInbox::begin(uint8_t port)
{
    if (slots == nullptr) {
        return;
    }

    const uint8_t next = (uint8_t)((head + 1) % depth);

    /* One slot is always left empty, so that head == tail means empty and
     * never full. */
    overflowed = (next == tail);
    receiving = true;

    if (overflowed) {
        dropped++;
        return;
    }

    slots[head].port = port;
    slots[head].length = 0;
}

void LinkInbox::append(const uint8_t *data, uint16_t length)
{
    if ((slots == nullptr) || !receiving || overflowed) {
        return;
    }

    Message &message = slots[head];

    if ((message.length + length) > MaxMessageLength) {
        overflowed = true;
        dropped++;
        return;
    }

    memcpy(message.bytes + message.length, data, length);
    message.length = (uint16_t)(message.length + length);
}

void LinkInbox::commit(void)
{
    if ((slots == nullptr) || !receiving) {
        return;
    }

    receiving = false;

    if (overflowed) {
        overflowed = false;
        return;
    }

    /* The bytes are all in place before the index moves: the store below is
     * what makes the message visible to the main loop. */
    const uint8_t next = (uint8_t)((head + 1) % depth);

    head = next;

    const uint8_t waiting = (uint8_t)((depth + head - tail) % depth);

    if (waiting > highWater) {
        highWater = waiting;
    }
}

void LinkInbox::abandon(void)
{
    receiving = false;
    overflowed = false;
}

const LinkInbox::Message *LinkInbox::peek(void)
{
    if ((slots == nullptr) || (tail == head)) {
        return (nullptr);
    }

    return (&slots[tail]);
}

void LinkInbox::release(void)
{
    if ((slots == nullptr) || (tail == head)) {
        return;
    }

    tail = (uint8_t)((tail + 1) % depth);
}

uint32_t LinkInbox::getDropped(void)
{
    return (dropped);
}

void LinkInbox::countRefused(void)
{
    refused++;
}

uint32_t LinkInbox::getRefused(void)
{
    return (refused);
}

uint8_t LinkInbox::getHighWater(void)
{
    return (highWater);
}
