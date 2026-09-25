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
 * @file TaskProcessSysex.h
 *
 * @brief A Sysex processing task.
 */

#include "TaskProcessSysex.h"
#include "helpers.h"
#include "MidiOutput.h"
#include "luaExtensionBase.h"
#include "App.h"

CircularBuffer<SysexTransport, SYSEX_QUEUE_SIZE> sysexQueue;

void processSysex(void)
{
    while (sysexQueue.isEmpty() != true) {
        SysexTransport st = sysexQueue.shift();

        /* Published just before the message is answered, not when it arrived:
         * everything that replies from here reads it. */
        MidiOutput::setTransactionId(st.transactionId);

        if (st.transferType == SysexTransport::Type::File) {
            processSysexFile(st.port, st.file, st.type);
        } else if (st.transferType == SysexTransport::Type::Memory) {
            processSysexMemory(st.port, st.sysexBlock);
        }
    }
}

void processSysexFile(uint8_t port,
                      LocalFile &file,
                      ElectraCommand::Object type)
{
    bool status = true;

    App::get()->enableMidi = false;
    System::tasks.disableMidi();

    status = App::get()->handleCtrlFileReceived(port, file, type);

    System::tasks.enableMidi();
    App::get()->enableMidi = true;

    if (status == true) {
        MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
        ;
    } else {
        MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
    }
}

void processSysexMemory(uint8_t port, const SysexBlock &sysexBlock)
{
    if (sysexBlock.isElectraSysex()) {
        ElectraCommand cmd = sysexBlock.getElectraSysexCommand();
        ElectraCommand::Object object = cmd.getObject();
        MemoryBlock sysexPayload = sysexBlock.getElectraSysexPayload();

        if (cmd.isFileUpload()) {
            LocalFile file;

            if (object == ElectraCommand::Object::FilePreset) {
                file.setFilepath(System::context.getCurrentPresetFile());
            } else if (object == ElectraCommand::Object::FileConfig) {
                file.setFilepath(System::context.getCurrentTempFile());
            } else if (object == ElectraCommand::Object::FileLua) {
                file.setFilepath(System::context.getCurrentLuaFile());
            } else {
                file.setFilepath(System::context.getCurrentTempFile());
            }

            file.writeAll(sysexPayload);

            if (object == ElectraCommand::Object::FilePreset) {
                MidiOutput::sendPresetSlotChanged(
                    MidiInterface::Type::MidiUsbDev, port);
            }

            if (App::get()->handleCtrlFileReceived(port, file, object)
                == true) {
                System::logger.write(
                    LOG_ERROR,
                    "processElectraSysex::handleElectraSysex: sending ACK");
                MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
                ;
            } else {
                System::logger.write(
                    LOG_ERROR,
                    "processElectraSysex::handleElectraSysex: sending NACK");
                MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
            }
        } else if (cmd.isFileRequest()) {
            if (object == ElectraCommand::Object::FilePreset) {
                if (MidiOutput::sendSysExFile(
                        port, System::context.getCurrentPresetFile(), object)) {
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex::handleElectraSysex: "
                        "preset sysex sent to the host");
                }
            } else if (object == ElectraCommand::Object::FileLua) {
                if (MidiOutput::sendSysExFile(
                        port, System::context.getCurrentLuaFile(), object)) {
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex::handleElectraSysex: "
                        "lua sysex sent to the host");
                }
            } else if (object == ElectraCommand::Object::FileConfig) {
                if (MidiOutput::sendSysExFile(
                        port, System::context.getCurrentConfigFile(), object)) {
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex::handleElectraSysex: "
                        "config sysex sent to the host");
                }
            } else if (object == ElectraCommand::Object::RuntimeInfo) {
                MidiOutput::sendRuntimeInfo(MidiInterface::Type::MidiUsbDev,
                                            port);
                System::logger.write(LOG_ERROR,
                                     "processElectraSysex::handleElectraSysex: "
                                     "runtimeInfo sysex sent to the host");
            } else if (object == ElectraCommand::Object::AppInfo) {
                MidiOutput::sendAppInfo(MidiInterface::Type::MidiUsbDev, port);
                System::logger.write(LOG_ERROR,
                                     "processElectraSysex::handleElectraSysex: "
                                     "appInfo sysex sent to the host");
            } else if (object == ElectraCommand::Object::ElectraInfo) {
                MidiOutput::sendElectraInfo(
                    MidiInterface::Type::MidiUsbDev,
                    port,
                    System::runtimeInfo.getElectraInfoSerial(),
                    System::runtimeInfo.getElectraInfoHwRevision());
                System::logger.write(LOG_ERROR,
                                     "processElectraSysex::handleElectraSysex: "
                                     "electraInfo sysex sent to the host");
            } else {
                App::get()->handleElectraSysex(
                    port,
                    sysexBlock); // this is to call application based requests
            }
        } else if (cmd.isExecute()) {
            if (L) {
                System::logger.write(LOG_ERROR,
                                     "processElectraSysex::handleElectraSysex: "
                                     "execute function");
                uint8_t buffer[sysexPayload.getLength()];
                sysexPayload.readBytes(buffer, sysexPayload.getLength());

                buffer[sysexPayload.getLength()] = '\0';

                System::logger.write(LOG_ERROR,
                                     "processElectraSysex::handleElectraSysex: "
                                     "execute Lua: \"%s\"",
                                     buffer);
                runLuaString((char *)buffer);
                MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
                ;
            } else {
                System::logger.write(LOG_ERROR,
                                     "processElectraSysex::handleElectraSysex: "
                                     "Lua script is not available");
            }
        } else if (cmd.isRemove()) {
            uint8_t bankNumber = cmd.getByte1();
            uint8_t slot = cmd.getByte2();
            uint8_t fileNumber = bankNumber * 12 + slot;
            char filename[MAX_FILENAME_LENGTH];
            char *fileToRemove = nullptr;

            if (object == ElectraCommand::Object::FilePreset) {
                System::context.formatPresetFilename(
                    filename, MAX_FILENAME_LENGTH, fileNumber);
                fileToRemove = filename;
                if (Hardware::sdcard.exists(fileToRemove)) {
                    int status = Hardware::sdcard.deleteFile(fileToRemove);
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex: removing file %s: %s",
                        fileToRemove,
                        (status == true) ? "OK" : "fail");
                }
                System::context.formatLuaFilename(
                    filename, MAX_FILENAME_LENGTH, fileNumber);
                fileToRemove = filename;
                if (Hardware::sdcard.exists(fileToRemove)) {
                    int status = Hardware::sdcard.deleteFile(fileToRemove);
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex: removing file %s: %s",
                        fileToRemove,
                        (status == true) ? "OK" : "fail");
                }
                MidiOutput::sendPresetSlotChanged(
                    MidiInterface::Type::MidiUsbDev, port);
            } else if (object == ElectraCommand::Object::FileLua) {
                System::context.formatLuaFilename(
                    filename, MAX_FILENAME_LENGTH, fileNumber);
                fileToRemove = filename;
                if (Hardware::sdcard.exists(fileToRemove)) {
                    int status = Hardware::sdcard.deleteFile(fileToRemove);
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex: removing file %s: %s",
                        fileToRemove,
                        (status == true) ? "OK" : "fail");
                    MidiOutput::sendPresetSlotChanged(
                        MidiInterface::Type::MidiUsbDev, port);
                }
            } else {
                System::logger.write(
                    LOG_ERROR,
                    "processElectraSysex: "
                    "file type not handled by the base firmware");
                fileToRemove = nullptr;
            }

            if (fileToRemove) {
                if (App::get()->handleCtrlFileRemoved(bankNumber, slot, object)
                    == true) {
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex::handleElectraSysex: sending ACK");
                    MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
                    ;
                } else {
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex::handleElectraSysex: sending NACK");
                    MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
                }
            } else {
                System::logger.write(
                    LOG_ERROR,
                    "processElectraSysex: removed command passed to the app");
                App::get()->handleElectraSysex(port, sysexBlock);
            }
        } else if (cmd.isUpdateRuntime()) {
            if ((uint8_t)object == 0x7D) {
                uint8_t newLoggerPort = cmd.getByte1();
                System::logger.write(
                    LOG_ERROR,
                    "processElectraSysex: : switch logger port to: %d",
                    newLoggerPort);
                System::logger.setPort(newLoggerPort);
                MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
            } else {
                App::get()->handleElectraSysex(port, sysexBlock);
            }
        } else if (cmd.isSystemCall()) {
            if ((uint8_t)object == 0x7F) {
                System::logger.write(
                    LOG_ERROR,
                    "processElectraSysex: : switch to the update mode");
                System::tasks.displayUpdateModeScreen();
                MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
                delay(200);
                _reboot_Teensyduino_();
            } else if (object == ElectraCommand::Object::Logger) {
                if (cmd.getByte1() == 0) {
                    System::logger.write(
                        LOG_ERROR, "processElectraSysex::logger disabled");
                    System::logger.disable();
                } else {
                    uint8_t logLevel = cmd.getByte2();
                    System::logger.enable();
                    System::logger.setLevel(logLevel);
                    System::logger.write(LOG_ERROR,
                                         "processElectraSysex::logger enabled: "
                                         "logLevel=%d",
                                         logLevel);
                }
                System::runtimeInfo.setLoggerStatus(loggerEnabled);
                MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
            } else if (object == ElectraCommand::Object::Window) {
                if (cmd.getByte1() == 0) {
                    System::tasks.stopWindowRepaint();
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex::Window repaint stopped");
                } else {
                    System::tasks.resumeWindowRepaint();
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex::Window repaint resumed");
                }
                MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
            } else {
                /* A system call this does not know is the application's, the
                 * same as a file request it does not know. Without this the
                 * chain simply ran out and the host waited for a reply that
                 * was never going to come. */
                App::get()->handleElectraSysex(port, sysexBlock);
            }
        } else {
            App::get()->handleElectraSysex(port, sysexBlock);
        }
    } else {
        System::logger.write(LOG_ERROR,
                             "processElectraSysex: "
                             "message ignored. Not an Electra.One message");
    }
}
