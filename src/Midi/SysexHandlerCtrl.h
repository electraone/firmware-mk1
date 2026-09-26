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
 * @file SysexHandler.h
 *
 * @brief A callback to process incoming Sysex data on CTRL port.
 */

#pragma once

#include "SysexHandler.h"
#include "LocalFile.h"
#include "System.h"
#include "Base64.h"
#include "MidiOutput.h"
#include "LinkInbox.h"

enum class FileType { preset, config, luaScript, uiToolkit, unknown };

class SysexHandlerCtrl : private SysexHandler
{
public:
    SysexHandlerCtrl(MidiInterface::Type newMidiInterface, uint8_t newPort)
        : SysexHandler(newMidiInterface, newPort),
          fileType(ElectraCommand::Object::Unknown),
          writeToFile(false),
          filename(nullptr),
          hasTransactionId(false),
          transactionId(0),
          linkMessage(false),
          refusing(false),
          deferring(false),
          deferredReady(false),
          deferredFromMainLoop(false),
          deferredLength(0),
          deferredPort(-1)
    {
    }

    /**
     * @brief Handle a message kept back during a repaint.
     *
     * Called from the main loop, where the pool may be written: the message
     * goes through process() as one complete packet, so everything the
     * interrupt would have done with it is done now instead.
     */
    void flushDeferred(void)
    {
        if (!deferredReady) {
            return;
        }

        /* Not while the interrupt is part way through another message on
         * this handler: the packet counter and the block are one set of
         * state, and this would be a second message interleaved with the
         * first. The next pass will find the counter back at zero. */
        if (packetNr != 0) {
            return;
        }

        /* Masked for the hand-over: the interrupt must not start a message
         * of its own between the check above and the end of this one. The
         * pool write inside takes a fraction of a millisecond. */
        __disable_irq();
        deferredReady = false;
        deferredFromMainLoop = true;
        process(deferred, deferredLength, true, deferredPort);
        deferredFromMainLoop = false;
        deferredLength = 0;
        __enable_irq();
    }

    virtual ~SysexHandlerCtrl() = default;

    void process(const uint8_t *sysExData,
                 uint16_t sysExSize,
                 bool complete,
                 int8_t portOverride = -1) override
    {
        uint16_t decodedDataSize = 0;

        /* An Electra Link message, for an application that takes them. It
         * goes into the link's own ring in main memory and never near the
         * pool: see LinkInbox.h. Decided on the first packet, which always
         * holds the whole header, and remembered for the rest. Nothing else
         * in here is touched for one - not the MIDI gate, not the pool, not
         * the file machinery. */
        if (packetNr == 0) {
            linkMessage = LinkInbox::isEnabled() && (sysExSize >= 6)
                          && (sysExData[1] == 0x00) && (sysExData[2] == 0x21)
                          && (sysExData[3] == 0x45)
                          && (sysExData[4] == LinkCommandByte);
        }

        if (linkMessage) {
            processLinkPacket(
                sysExData, sysExSize, complete, portOverride);
            return;
        }

        /* Anything else arriving while the main loop holds the display's
         * bus would be written into the pool across that same bus, from
         * here, in the middle of whatever is being drawn. Only possible with
         * the inbox up, which is the only time this runs during a repaint.
         * A message that fits the deferral buffer is kept in main memory and
         * handled from the main loop, exactly as if it had arrived then; one
         * that does not - or that arrives while another is still waiting -
         * is refused whole rather than corrupting a picture, and counted. */
        if (packetNr == 0) {
            deferring = false;
            refusing = false;

            if (LinkInbox::isEnabled() && System::repaintActive
                && !deferredFromMainLoop) {
                if (deferredLength == 0) {
                    deferring = true;
                } else {
                    refusing = true;
                }
            }
        }

        if (deferring) {
            if ((deferredLength + sysExSize) > sizeof(deferred)) {
                deferring = false;
                refusing = true;
                deferredLength = 0;
            } else {
                memcpy(deferred + deferredLength, sysExData, sysExSize);
                deferredLength += sysExSize;
                deferredPort = portOverride;
                packetNr++;

                if (complete) {
                    packetNr = 0;
                    deferring = false;
                    deferredReady = true;
                }
                return;
            }
        }

        if (refusing) {
            packetNr++;

            if (complete) {
                packetNr = 0;
                refusing = false;
                LinkInbox::countRefused();
            }
            return;
        }

        if (packetNr == 0) {
            App::get()->enableMidi = false;

            /* A host may put a transaction id in front of the command, as
             * three bytes after the manufacturer id: a nought where a command
             * byte would be, then the id, fourteen bits, low seven first. No
             * command is nought, so the nought is what says it is there.
             *
             * It is taken off here and the rest of the firmware never sees
             * it. Without this the nought was read as the command and every
             * request carrying one did nothing at all - the editor's uploads
             * among them, and its queue filled with messages it was still
             * waiting for.
             *
             * Only in an Electra message. Everything on this port comes
             * through here, and another manufacturer's id can have a nought
             * in that byte for reasons of its own; stripping seven bytes off
             * one of those and putting an Electra header in front of what was
             * left would turn somebody else's message into nonsense. */
            hasTransactionId = (sysExSize >= 7) && (sysExData[1] == 0x00)
                               && (sysExData[2] == 0x21)
                               && (sysExData[3] == 0x45)
                               && (sysExData[4] == 0x00);

            transactionId = hasTransactionId
                                ? (uint16_t)(sysExData[5] | (sysExData[6] << 7))
                                : 0;
#ifdef DEBUG
            if (hasTransactionId) {
                System::logger.write(
                    LOG_ERROR, "handleSysEx: transaction id=%d", transactionId);
            }
#endif

            // if it is a upload of a file
            if ((commandByte(sysExData, 0)
                 == (uint8_t)ElectraCommand::Type::FileUpload)
                && ((commandByte(sysExData, 1)
                     == (uint8_t)ElectraCommand::Object::FilePresetLegacy)
                    || (commandByte(sysExData, 1)
                        == (uint8_t)ElectraCommand::Object::FilePreset)
                    || (commandByte(sysExData, 1)
                        == (uint8_t)ElectraCommand::Object::FileLua)
                    || (commandByte(sysExData, 1)
                        == (uint8_t)ElectraCommand::Object::FileUi))) {
                System::tasks.enableCtrlMidi();

                // write preset directly to the current preset file
                if (commandByte(sysExData, 1)
                    == (uint8_t)ElectraCommand::Object::FileUi) {
                    System::logger.write(
                        LOG_ERROR, "handleSysEx: toolkit transfer initiated");
                    filename = UI_FILE;
                    fileType = ElectraCommand::Object::FileUi;
                } else if (commandByte(sysExData, 1)
                           == (uint8_t)ElectraCommand::Object::FileLua) {
                    filename = System::context.getCurrentLuaFile();
                    fileType = ElectraCommand::Object::FileLua;
                    System::logger.write(
                        LOG_ERROR,
                        "handleSysEx: lua script transfer initiated: filename=%s",
                        filename);
                } else if ((commandByte(sysExData, 1)
                            == (uint8_t)ElectraCommand::Object::FilePreset)
                           || (commandByte(sysExData, 1)
                               == (uint8_t)
                                   ElectraCommand::Object::FilePresetLegacy)) {
                    filename = System::context.getCurrentPresetFile();
                    fileType = ElectraCommand::Object::FilePreset;
                    System::logger.write(
                        LOG_ERROR,
                        "handleSysEx: preset transfer initiated: filename=%s",
                        filename);
                } else {
                    System::logger.write(LOG_ERROR,
                                         "handleSysEx: unknown filetype");
                    fileType = ElectraCommand::Object::Unknown;
                }

                /*
    				     * Open the file
    				     */
                if (filename) {
                    file = Hardware::sdcard.createOutputStream(
                        filename, FILE_WRITE | O_CREAT | O_TRUNC);
                }

                // if file cannot be opened now, use sysexBlock
                if (file) {
                    writeToFile = true;

                    // strip the leading bytes
                    uint16_t header = headerLength();
                    sysExData += header; // remove sysex header
                    sysExSize -= header;

                    if (fileType == ElectraCommand::Object::FileUi) {
                        sysExData +=
                            2; // the ui toolkit header has two extra bytes
                        sysExSize -= 2;
                    }
                } else {
                    System::logger.write(LOG_ERROR,
                                         "handleSysEx: Read file failed: %s",
                                         filename);
                    writeToFile = false;
                }
            }

            /*
    		 * always create a sysex block
    		 */
            sysexBlock = SysexBlock(App::get()->sysexPool.openMemoryBlock());
        }

        /*
    	 * Process each packet according to if it is a preset transfer or other sysex message
    	 */
        /* The block is given the message without the transaction prefix, so
         * that everything downstream sees the same bytes whether a host used
         * one or not. */
        if (!writeToFile && (packetNr == 0) && hasTransactionId) {
            static const uint8_t header[4] = { 0xF0, 0x00, 0x21, 0x45 };

            sysexBlock.writeBytes(header, sizeof(header));
            sysExData += 7;
            sysExSize -= 7;
        }

        if (writeToFile) {
            // Check if it is a last packet with 0xF7. If so, trim it
            if (sysExData[sysExSize - 1] == 0xF7) {
                sysExSize -= 1; // remove sysex stop byte
            }

            // if it is a uitoolkitTransfer decode the base64 data
            if (fileType == ElectraCommand::Object::FileUi) {
                decodedDataSize = base64_decode(
                    (char *)decodedData, (char *)sysExData, sysExSize);
                file.write(decodedData, decodedDataSize);
            } else {
                file.write(sysExData, sysExSize);
            }
        } else {
            sysexBlock.writeBytes(sysExData, sysExSize);
        }
        packetNr++;

        /*
    	 * handle last packet of the sysex transfer
    	 */
        if (complete) {
            //bool callbackStatus = true;
            uint8_t portUsed = (portOverride != -1) ? portOverride : port;

            // Close the sysex block
            sysexBlock.close();

            System::tasks.disableMidi();

            if (writeToFile) {
                file.close();
                sysexQueue.push(SysexTransport(
                    LocalFile(filename), fileType, portUsed, transactionId));
            } else {
                sysexQueue.push(
                    SysexTransport(sysexBlock, portUsed, transactionId));
            }

            App::get()->enableMidi = true;
            System::tasks.enableMidi();

            if (fileType == ElectraCommand::Object::FilePreset) {
                // Loading the preset always (temporary fix) removes the Lua script
                // ie. the Lua script needs to be transferred afterwards
                Hardware::sdcard.deleteFile(
                    System::context.getCurrentLuaFile());
            }

            packetNr = 0;
            fileType = ElectraCommand::Object::Unknown;
            writeToFile = false;
            filename = nullptr;
            hasTransactionId = false;
        }
    }

private:
    /** Link::commandByte, spelled here so the interrupt side needs no
     *  protocol headers. */
    static constexpr uint8_t LinkCommandByte = 0x0A;

    /**
     * @brief One packet of a link message, into the inbox.
     *
     * The header is stripped from the first packet and the end byte from the
     * last, so what is queued is the opcode and the encoded payload and
     * nothing else - which is what the client wants.
     */
    void processLinkPacket(const uint8_t *sysExData,
                           uint16_t sysExSize,
                           bool complete,
                           int8_t portOverride)
    {
        if (packetNr == 0) {
            LinkInbox::begin((portOverride != -1) ? (uint8_t)portOverride
                                                  : port);
            sysExData += 5;
            sysExSize -= 5;
        }

        if (complete && (sysExSize > 0) && (sysExData[sysExSize - 1] == 0xF7)) {
            sysExSize--;
        }

        LinkInbox::append(sysExData, sysExSize);
        packetNr++;

        if (complete) {
            LinkInbox::commit();
            packetNr = 0;
            linkMessage = false;
        }
    }

    /** The command byte, or the object byte after it, wherever they are. */
    uint8_t commandByte(const uint8_t *sysExData, uint8_t index) const
    {
        return (sysExData[(hasTransactionId ? 7 : 4) + index]);
    }

    /** Everything in front of a file's own bytes. */
    uint16_t headerLength(void) const
    {
        return (hasTransactionId ? 9 : 6);
    }

    File file;
    ElectraCommand::Object fileType;
    bool writeToFile;
    uint8_t decodedData[USB_MIDI_SYSEX_MAX];
    const char *filename;
    bool hasTransactionId;
    uint16_t transactionId;

    /** Whether the message being received is going to the link inbox. */
    bool linkMessage;

    /** Whether the message being received is being thrown away. */
    bool refusing;

    /** Whether it is being kept in main memory for the main loop instead. */
    bool deferring;

    /** A kept message is whole and waiting for flushDeferred(). */
    volatile bool deferredReady;

    /** flushDeferred() is running: do not defer the message it is handing
     *  over, whatever the display is doing. */
    bool deferredFromMainLoop;

    /** Room for a request. Anything bigger is refused; a file upload was
     *  never going to be handled here. */
    static constexpr uint16_t DeferredCapacity = 512;

    uint8_t deferred[DeferredCapacity];
    uint16_t deferredLength;
    int8_t deferredPort;
};
