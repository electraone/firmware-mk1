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

#include "LinkTransport.h"
#include "src/Link/LinkRecords.h"
#include "Base64.h"
#include "MidiOutput.h"
#include "System.h"

void LinkTransport::sendLink(uint8_t opcode,
                             const uint8_t *payload,
                             size_t length)
{
    /*
     * The manufacturer id, the command, the opcode and the encoded payload.
     * The USB layer puts the start and end bytes round it. base64_encode()
     * writes a terminator after the text, so the buffer keeps a byte for it.
     */
    static uint8_t message[4 + Link::maxEncodedLength + 1];

    message[0] = 0x00;
    message[1] = 0x21;
    message[2] = 0x45;
    message[3] = Link::commandByte;
    message[4] = opcode;

    size_t total = 5;

    if ((payload != nullptr) && (length > 0)) {
        if (length > Link::maxPayloadLength) {
            System::logger.write(LOG_ERROR,
                                 "LinkTransport: payload of %d bytes is too "
                                 "long to send",
                                 (int)length);
            return;
        }

        const int encoded = base64_encode(
            (char *)(message + 5), (char *)(uintptr_t)payload, (int)length);

        if ((encoded <= 0) || ((size_t)encoded > Link::maxEncodedLength)) {
            System::logger.write(
                LOG_ERROR, "LinkTransport: payload does not encode");
            return;
        }

        total += (size_t)encoded;
    }

    MidiOutput::sendSysEx(MidiInterface::Type::MidiUsbDev,
                          USB_MIDI_PORT_CTRL,
                          message,
                          (uint16_t)total);
    sent++;
}
