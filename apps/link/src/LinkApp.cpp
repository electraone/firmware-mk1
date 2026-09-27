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

#include "LinkApp.h"
#include "LinkWindow.h"
#include "LinkInbox.h"
#include "TaskReadMidi.h"
#include "FileTransfer.h"
#include "ScreenCapture.h"
#include "MidiOutput.h"
#include "System.h"

#if defined(LINK_BENCH) && LINK_BENCH
#include "ButtonListener.h"
#include "PotListener.h"

/*
 * The panel, driven from a debugger. A bench with the unit in a primary's
 * host socket has no way to press its buttons or turn its knobs from a
 * computer, so these are written over J-Link and acted on by the next pass:
 * a button number plus one, a knob and a turn in phase counts, a knob and a
 * finger down (1) or up (2). Built only with LINK_BENCH=1.
 */
volatile uint8_t linkBenchButton = 0;
volatile uint8_t linkBenchPotId = 0;
volatile int16_t linkBenchPotDelta = 0;
volatile uint8_t linkBenchPotTouch = 0;
#endif

namespace
{
    LinkApp *instance = nullptr;

    void linkTick(void)
    {
        if (instance != nullptr) {
            instance->tick();
        }
    }

    /*
     * The program's own pass, once a millisecond. The application's user task
     * runs four times a second, which is a knob turn's worth of lag on a link
     * that answers in one.
     */
    Task linkTask(1000, TASK_FOREVER, &linkTick);
} // namespace

LinkApp::LinkApp() : transport(), program(transport), window(nullptr)
{
    instance = this;
}

void LinkApp::initialise(void)
{
    /*
     * Forty-eight messages in main memory, twenty-one kilobytes: a preset
     * list and a page arrive together, faster than a pass can draw them,
     * and while a page is being drawn nothing is taken out - a message that
     * finds no room is a control that never appears. And the interrupt may
     * take twenty-four events from the USB port a tick, so a burst is off
     * the bus before the primary has to wait.
     */
    LinkInbox::enable(48);
    readMidiUsbDevBudget = 24;

    window = new LinkWindow(program);
    window->setActive(true);

    program.attach(*window);
    program.setName(System::runtimeInfo.getElectraInfoSerial());

    /* Every line of the conversation at LOG_TRACE: on the wire it is a
     * message a millisecond, and the logger's port is the one the primary
     * talks on. */
    program.getLink().setLogger([](bool incoming, const std::string &text) {
        System::logger.write(
            LOG_TRACE, "Link %s %s", incoming ? "<-" : "->", text.c_str());
    });

    System::tasks.addTask(linkTask);
    linkTask.enable();

    window->repaint();

    System::logger.write(LOG_ERROR, "Electra Link: initialisation completed");
}

void LinkApp::tick(void)
{
#if defined(LINK_BENCH) && LINK_BENCH
    if (linkBenchButton != 0) {
        const uint8_t buttonId = (uint8_t)(linkBenchButton - 1);

        linkBenchButton = 0;
        ButtonListener::buttonBroadcaster.buttonDown(buttonId);
        ButtonListener::buttonBroadcaster.buttonUp(buttonId);
    }

    if (linkBenchPotTouch != 0) {
        const uint8_t potId = linkBenchPotId;
        const bool down = (linkBenchPotTouch == 1);

        linkBenchPotTouch = 0;

        if (down) {
            PotListener::potBroadcaster.potTouchDown(potId);
        } else {
            PotListener::potBroadcaster.potTouchUp(potId);
        }
    }

    if (linkBenchPotDelta != 0) {
        const int16_t delta = linkBenchPotDelta;

        linkBenchPotDelta = 0;
        PotListener::potBroadcaster.potChange(linkBenchPotId, delta);
    }
#endif

    program.process(millis());
}

void LinkApp::yield(void)
{
    program.keepAlive(millis());
}

void LinkApp::handleIncomingMidiMessage(
    [[maybe_unused]] const MidiInput &midiInput,
    [[maybe_unused]] const MidiMessage &midiMessage)
{
}

void LinkApp::handleElectraSysex(uint8_t port, const SysexBlock &sysexBlock)
{
    ElectraCommand cmd = sysexBlock.getElectraSysexCommand();
    const ElectraCommand::Object object = cmd.getObject();
    MemoryBlock payload = sysexBlock.getElectraSysexPayload();

    /* An event - an acknowledgement, a notice - is not a request, and
     * answering one would be answering an answer. Dropped without a word. */
    if (cmd.isEvent()) {
        return;
    }

    if (cmd.isSystemCall() && (object == ElectraCommand::Object::Screenshot)) {
        char filename[FileTransfer::maxPathLength];

        if (ScreenCapture::saveNext(filename, sizeof(filename))) {
            MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
        } else {
            MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        }
        return;
    }

    if (cmd.isFileRequest() && (object == ElectraCommand::Object::Location)) {
        char directory[FileTransfer::maxPathLength];

        if (FileTransfer::resolveLocation(
                directory, sizeof(directory), payload)) {
            FileTransfer::sendListing(port, directory);
        } else {
            MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        }
        return;
    }

    if (cmd.isFileRequest() && (object == ElectraCommand::Object::File)) {
        char path[FileTransfer::maxPathLength];

        if (FileTransfer::resolve(path, sizeof(path), payload)
            && FileTransfer::sendFile(port, path)) {
            MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
        } else {
            MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        }
        return;
    }

    if (cmd.isRemove() && (object == ElectraCommand::Object::File)) {
        char path[FileTransfer::maxPathLength];

        if (FileTransfer::resolve(path, sizeof(path), payload)
            && Hardware::sdcard.deleteFile(path)) {
            MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
        } else {
            MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        }
        return;
    }

    if (cmd.isRemove() && (object == ElectraCommand::Object::Location)) {
        char directory[FileTransfer::maxPathLength];

        if (FileTransfer::resolveLocation(directory, sizeof(directory), payload)
            && FileTransfer::removeLocation(directory)) {
            MidiOutput::sendAck(MidiInterface::Type::MidiUsbDev, port);
        } else {
            MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
        }
        return;
    }

    /* Not something an Electra Link does. The NACK is the honest answer, and it
     * is what keeps a host's queue moving. */
    MidiOutput::sendNack(MidiInterface::Type::MidiUsbDev, port);
}

// This macro instructs main() routine to launch the app.
START_ELECTRA_APPLICATION(LinkApp)
