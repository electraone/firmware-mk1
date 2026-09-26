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

#include "TaskSendMidi.h"
#include "MidiOutput.h"

/*
 * Send queued messages
 */
void sendMidi(void)
{
        MidiMessageTransport message;
    uint8_t i = 0;

    /* Up to ten a tick, which is five thousand a second. The loop used to
     * also stop at a message carrying the same timestamp as the first one
     * sent this tick, a guard for a requeue that no longer exists - and two
     * messages queued in the same microsecond, the two halves of a 14-bit
     * CC, had the second one wait for the next tick. */
    while (!MidiOutput::outgoingQueue.isEmpty() && i < 10) {
        message = MidiOutput::outgoingQueue.shift();

        if (message.invalid) {
            continue;
        }

        if (message.getType() == MidiMessage::Type::SystemExclusive) {
            MidiOutput::send(
                message.getInterfaceType(), message.getPort(), message);
        } else {
            MidiOutput::send(message.getInterfaceType(),
                             message.getPort(),
                             message.getType(),
                             message.getChannel(),
                             message.getData1(),
                             message.getData2());
        }

#ifdef DEBUG
        System::logger.write(
            ERROR,
            "sendMidi: queued message (%d): interface:%d, port:%d, channel: %d,"
            " parameterNumber: %d, value=%d, type=%s",
            message.id,
            message.getInterfaceType(),
            message.getPort(),
            message.getChannel(),
            message.getData1(),
            message.getData2(),
            message.getDescription());
#endif
        i++;
    }
}
