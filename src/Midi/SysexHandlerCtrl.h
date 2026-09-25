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
          transactionId(0)
    {
    }

    virtual ~SysexHandlerCtrl() = default;

    void process(const uint8_t *sysExData,
                 uint16_t sysExSize,
                 bool complete,
                 int8_t portOverride = -1) override
    {
        uint16_t decodedDataSize = 0;

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
};
