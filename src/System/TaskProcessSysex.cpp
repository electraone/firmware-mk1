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

/**
 * @brief Whether a request named a slot at all.
 *
 * A request for the loaded slot is six bytes and carries no bank or slot; one
 * for a particular slot carries both. A command built from a message that stops
 * early has zeros in those bytes, which is bank 0 slot 0 - a real slot - so the
 * two cannot be told apart by value. The length is what tells them apart.
 */
static bool namesASlot(const SysexBlock &sysexBlock)
{
    /* 0xF0, three manufacturer bytes, command, object, bank, slot. */
    return (sysexBlock.getLength() >= 8);
}

/** The preset file the request names, or the loaded one. */
static const char *requestedPresetFile(const SysexBlock &sysexBlock,
                                       ElectraCommand &cmd,
                                       char *buffer,
                                       size_t size)
{
    if (!namesASlot(sysexBlock)) {
        return (System::context.getCurrentPresetFile());
    }

    System::context.formatPresetFilename(
        buffer, size, (cmd.getByte1() * 12) + cmd.getByte2());

    return (buffer);
}

/** The Lua file the request names, or the loaded one. */
static const char *requestedLuaFile(const SysexBlock &sysexBlock,
                                    ElectraCommand &cmd,
                                    char *buffer,
                                    size_t size)
{
    if (!namesASlot(sysexBlock)) {
        return (System::context.getCurrentLuaFile());
    }

    System::context.formatLuaFilename(
        buffer, size, (cmd.getByte1() * 12) + cmd.getByte2());

    return (buffer);
}

/**
 * @brief Sends the file, or says it is not there.
 *
 * A request for an empty slot used to be answered with nothing at all: the send
 * failed, the failure was not reported, and the host sat waiting for a file that
 * was never coming with everything behind it in its queue. An editor asking
 * what is in each of seventy-two slots meets this on every empty one.
 */
static void answerFileRequest(uint8_t port,
                              const char *filename,
                              ElectraCommand::Object object,
                              const char *what)
{
    if (MidiOutput::sendSysExFile(port, filename, object)) {
        System::logger.write(LOG_INFO,
                             "answerFileRequest: %s sent to the host: %s",
                             what,
                             filename);
        return;
    }

    System::logger.write(
        LOG_ERROR, "answerFileRequest: no %s to send: %s", what, filename);
    MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
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
                /* The slot the host asked for, which used to be ignored: the
                 * current preset file went out whatever bank and slot the
                 * request named, so an editor reading slot five while slot one
                 * was loaded was handed the wrong preset and had no way to
                 * tell. */
                char filename[MAX_FILENAME_LENGTH];

                answerFileRequest(
                    port,
                    requestedPresetFile(
                        sysexBlock, cmd, filename, sizeof(filename)),
                    object,
                    "preset");
            } else if (object == ElectraCommand::Object::FileLua) {
                char filename[MAX_FILENAME_LENGTH];

                answerFileRequest(
                    port,
                    requestedLuaFile(
                        sysexBlock, cmd, filename, sizeof(filename)),
                    object,
                    "lua");
            } else if (object == ElectraCommand::Object::FileConfig) {
                answerFileRequest(port,
                                  System::context.getCurrentConfigFile(),
                                  object,
                                  "config");
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
        } else if (cmd.isExecute()
                   && ((object == ElectraCommand::Object::Function)
                       || (object == ElectraCommand::Object::FileLua))) {
            /* Only the two objects that mean "run this Lua". Every execute used
             * to be treated as one, so a host asking a slot to be reloaded had
             * its own request's bytes run as a Lua program. */
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
                /* Refused rather than ignored: the host is waiting for an
                 * answer and will hold everything behind it until one
                 * arrives. */
                MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
            }
        } else if (cmd.isRemove()) {
            uint8_t bankNumber = cmd.getByte1();
            uint8_t slot = cmd.getByte2();
            uint8_t fileNumber = bankNumber * 12 + slot;
            char filename[MAX_FILENAME_LENGTH];
            char *fileToRemove = nullptr;

            /* Clearing a slot and removing its preset are the same thing on
             * this model: a slot is its preset file and its script, and there
             * is nothing else in one to keep. The editor sends whichever of the
             * two it prefers, so both are answered. */
            if ((object == ElectraCommand::Object::FilePreset)
                || (object == ElectraCommand::Object::PresetSlot)) {
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
            } else if (object == ElectraCommand::Object::FileConfig) {
                /* The config, which has no slot. Removing it leaves the
                 * firmware on the defaults it boots with when there is no file
                 * - which is what the host asking for this wants, and what it
                 * had no way to ask for before. */
                fileToRemove = System::context.getCurrentConfigFile();

                if (Hardware::sdcard.exists(fileToRemove)) {
                    int status = Hardware::sdcard.deleteFile(fileToRemove);
                    System::logger.write(
                        LOG_ERROR,
                        "processElectraSysex: removing file %s: %s",
                        fileToRemove,
                        (status == true) ? "OK" : "fail");
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
            if (object == ElectraCommand::Object::RuntimeInfo) {
                /* Re-marks the unused memory the free-RAM figure is worked out
                 * from. What free RAM reports is bounded by the deepest the
                 * stack has ever been and the highest the heap has ever
                 * reached, and neither comes back down on its own, so a reading
                 * taken after a big preset has been and gone is about that
                 * preset rather than about what is loaded now. */
                Hardware::ram.initialize();
                System::logger.write(
                    LOG_ERROR, "processElectraSysex: memory markers reset");
                MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
            } else if ((uint8_t)object == 0x7D) {
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
            if (object == ElectraCommand::Object::Reboot) {
                System::logger.write(LOG_ERROR,
                                     "processElectraSysex: : rebooting");
                MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
                /* Answered before the reboot, not after: there is no after. */
                delay(200);
                _reboot_Teensyduino_();
            } else if ((uint8_t)object == 0x7F) {
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
            } else if (object == ElectraCommand::Object::AppState) {
                /* Nothing to flush. Every piece of state this model keeps is
                 * written when it changes: the configuration when its file is
                 * uploaded, the brightness and the logger setting straight into
                 * the EEPROM. The host is told so rather than refused - there is
                 * nothing wrong with the request, and a refusal would read as a
                 * failure to save. */
                System::logger.write(
                    LOG_ERROR,
                    "processElectraSysex: application state is already saved");
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
