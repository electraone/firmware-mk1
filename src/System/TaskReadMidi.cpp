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

#include "TaskReadMidi.h"
#include "Profile.h"
#include "MidiInputCallback.h"
#include "LinkInbox.h"
#include "App.h" // \todo To be removed.
#include "Trace.h"

#define USB_MIDI_PORT_CTRL 2

CircularBuffer<MidiMessageTransport, 100> incomingQueueL2;

/* How many messages each tick may take from the USB device port. Four is
 * the controller's figure, chosen against what the queue holds between two
 * runs of processMidi(). The link program raises it: a satellite's whole
 * conversation arrives on this one port, a view is a burst of forty SysEx
 * messages, and SysEx never reaches the queue at all - the reads go to the
 * inbox and cost the interrupt a copy each. */
uint8_t readMidiUsbDevBudget = 4;

/** This is used when reading of SysEx on CTRL port gets priority.
 */
void readCtrlMidi(void)
{
    MidiInterface::get(MidiInterface::Type::MidiIo)->readNoAction(0);
    MidiInterface::get(MidiInterface::Type::MidiIo)->readNoAction(1);
    MidiInterface::get(MidiInterface::Type::MidiUsbDev)->readNoAction(0);
    MidiInterface::get(MidiInterface::Type::MidiUsbHost)->readNoAction(0);
    MidiInterface::get(MidiInterface::Type::MidiUsbHost)->readNoAction(1);
}

// \todo Deduplicate reading from the physical interfaces.
/** process incomming MIDI messages
 */
void readMidi(void)
{
    PROFILE_BEGIN(MidiInIsr);

    /*
     * Not while a big SysEx is being streamed out, either - the main loop is
     * in that for as long as the file takes, and the controller has nothing
     * to gain from reading meanwhile. A satellite has: the primary keeps
     * talking to it the whole time, and a port that is not read is a port
     * the primary cannot write to. Reading costs the transfer nothing.
     */
    if (System::sysExBusy && !LinkInbox::isEnabled()) {
        return;
    }

    /*
     * A repaint holds the display's SPI bus from the main loop, and a SysEx
     * assembled here goes into the pool on the far side of that bus - so
     * nothing used to be read while one was on. That was the whole input
     * stalling for as long as a page took to draw, which a computer forgives
     * and a primary driving a satellite does not: its host port fills, and
     * what it could not send is gone. With the link inbox up, the USB device
     * port is read through a repaint - a link message lands in main memory
     * and never near the bus, and the SysEx handlers refuse anything else
     * that starts while the bus is busy. See LinkInbox.h.
     */
    const bool repainting = System::repaintActive;

    if (repainting && !LinkInbox::isEnabled()) {
        return;
    }

    // Skip if the interface unifying queue is not empty

    // Process inbound MIDI messages on USB MIDI IN port (virtual cables).
    /* Route a message to the other ports as configured and queue it for the
     * application. This is the whole of what the interrupt does with a
     * message; the rest happens in processMidi() on the main loop. SysEx is
     * handled by the callbacks in SysexCallbacks.cpp and never gets here as
     * a message. The clock used to be dispatched to the application and the
     * Lua script from this context, for latency, which put a lua_pcall and
     * heap allocations inside an interrupt while the main loop was in the
     * same interpreter and the same allocator. It goes through the queue
     * with everything else now, two milliseconds later at most. */
    auto forward = [](MidiMessageTransport &message) {
        if (message.getType() == MidiMessage::Type::SystemExclusive) {
            return;
        }

        MidiInput midiInput(message.getInterfaceType(), message.getPort());

        if (!runRouteMessageCallback(midiInput, message)) {
            System::logger.write(LOG_ERROR, "skipping the message");
            return;
        }
        incomingQueueL2.push(message);
    };

    /* Up to a few messages per interface per tick. One per tick, which is
     * what this was, capped USB input at two thousand messages a second,
     * and a host sending a bank of values arrives faster than that. Four
     * from each of five inputs is twenty a tick, eighty between two runs of
     * processMidi(), within what the queue holds. */
    constexpr uint8_t MaxReadsPerInterface = 4;

    MidiBase *midiInterface =
        MidiInterface::get(MidiInterface::Type::MidiUsbDev);
    for (uint8_t i = 0; (i < readMidiUsbDevBudget) && midiInterface->read(0);
         i++) {
        uint8_t port = midiInterface->getActivePort();
        MidiMessageTransport message(
            MidiInterface::Type::MidiUsbDev,
            port,
            midiInterface->getChannel(port),
            static_cast<MidiMessage::Type>(midiInterface->getType(port)),
            midiInterface->getData1(port),
            midiInterface->getData2(port));
        forward(message);
    }

    if (repainting) {
        PROFILE_END(MidiInIsr);
        return;
    }

    // Process inbound MIDI messages on USB host ports.
    for (uint8_t port = 0; port < 2; port++) {
        MidiBase *midiInterface =
            MidiInterface::get(MidiInterface::Type::MidiUsbHost);
        for (uint8_t i = 0;
             (i < MaxReadsPerInterface) && midiInterface->read(port);
             i++) {
            MidiMessageTransport message(
                MidiInterface::Type::MidiUsbHost,
                USBDevices[port].midiBus,
                USBDevices[port].midiDevice->getChannel(),
                static_cast<MidiMessage::Type>(
                    USBDevices[port].midiDevice->getType()),
                USBDevices[port].midiDevice->getData1(),
                USBDevices[port].midiDevice->getData2());
            forward(message);
        }
    }

    // Process inbound messages on MIDI DIN5 interfaces.
    for (uint8_t port = 0; port < 2; port++) {
        MidiBase *midiInterface =
            MidiInterface::get(MidiInterface::Type::MidiIo);
        for (uint8_t i = 0;
             (i < MaxReadsPerInterface) && midiInterface->read(port);
             i++) {
            MidiMessageTransport message(
                MidiInterface::Type::MidiIo,
                port,
                midiInterface->getChannel(port),
                static_cast<MidiMessage::Type>(midiInterface->getType(port)),
                midiInterface->getData1(port),
                midiInterface->getData2(port));
            forward(message);
        }
    }

    PROFILE_END(MidiInIsr);
}

/** Process MIDI messages form the L2 queue.
 */
void processMidi(void)
{
    while (incomingQueueL2.isEmpty() != true) {
        /* The interrupt pushes and this side shifts, and the buffer keeps one
         * count both sides update. Taking a message with interrupts masked
         * is what keeps the two updates from interleaving and the count
         * from drifting off by one for good. */
        __disable_irq();
        MidiMessageTransport message = incomingQueueL2.shift();
        __enable_irq();
        PROFILE_BEGIN(MidiIn);

        // Indicate incoming data on the display
        App::get()->statusBar.indicate(message.getInterfaceType(),
                                       message.getPort(),
                                       Direction::in,
                                       message.getType());

#ifdef DEBUG
        System::logger.write(
            ERROR,
            "queueL2 [%d]: received: interface=%d, port=%d, channel=%d, "
            "type=%d, data1=%d, data2=%d",
            incomingQueueL2.size(),
            message.getInterfaceType(),
            message.getPort(),
            message.getChannel(),
            message.getType(),
            message.getData1(),
            message.getData2());
#endif

        if (App::get()->enableMidi == true) {
            TRACE_MIDI_IN(MidiInterface::getName(message.getInterfaceType()),
                          message.getPort(),
                          message.getType(),
                          message.getChannel(),
                          message.getData1(),
                          message.getData2());

            MidiInput midiInput(message.getInterfaceType(), message.getPort());
            MidiInputCallback::deviceManager.handleIncomingMidiMessage(
                midiInput, message);
            runOptionalCallbacks(midiInput,
                                 message.getChannel(),
                                 message.getType(),
                                 message.getData1(),
                                 message.getData2());
        }
        PROFILE_END(MidiIn);
    }
}

/** Call optional callback function if it is linked.
 */
bool runRouteMessageCallback(MidiInput &midiInput, MidiMessage &midiMessage)
{
    if (MidiInputCallback::routeMessageCallback) {
        return (
            MidiInputCallback::routeMessageCallback(midiInput, midiMessage));
    }
    return (true);
}

/** Call optional callback function if it is linked.
 */
void runOptionalCallbacks(MidiInput midiInput,
                          uint8_t channel,
                          MidiMessage::Type type,
                          uint8_t data1,
                          uint8_t data2)
{
    // \todo optional callbacks (Replace with an indexed table???)
    if (type == MidiMessage::Type::Clock) {
        if (MidiInputCallback::onMidiClockCallback) {
            MidiInputCallback::onMidiClockCallback(midiInput);
        }
    } else if (type == MidiMessage::Type::Start) {
        if (MidiInputCallback::onMidiStartCallback) {
            MidiInputCallback::onMidiStartCallback(midiInput);
        }
    } else if (type == MidiMessage::Type::Stop) {
        if (MidiInputCallback::onMidiStopCallback) {
            MidiInputCallback::onMidiStopCallback(midiInput);
        }
    } else if (type == MidiMessage::Type::Continue) {
        if (MidiInputCallback::onMidiContinueCallback) {
            MidiInputCallback::onMidiContinueCallback(midiInput);
        }
    } else if (type == MidiMessage::Type::ActiveSensing) {
        if (MidiInputCallback::onMidiActiveSensingCallback) {
            MidiInputCallback::onMidiActiveSensingCallback(midiInput);
        }
    } else if (type == MidiMessage::Type::SystemReset) {
        if (MidiInputCallback::onMidiSystemResetCallback) {
            MidiInputCallback::onMidiSystemResetCallback(midiInput);
        }
    } else if (type == MidiMessage::Type::TuneRequest) {
        if (MidiInputCallback::onMidiTuneRequestCallback) {
            MidiInputCallback::onMidiTuneRequestCallback(midiInput);
        }
    } else if (type == MidiMessage::Type::ProgramChange) {
        if (MidiInputCallback::onMidiProgramChangeCallback) {
            MidiInputCallback::onMidiProgramChangeCallback(
                midiInput, channel, data1);
        }
    } else if (type == MidiMessage::Type::AfterTouchChannel) {
        if (MidiInputCallback::onMidiAfterTouchChannelCallback) {
            MidiInputCallback::onMidiAfterTouchChannelCallback(
                midiInput, channel, data1);
        }
    } else if (type == MidiMessage::Type::SongSelect) {
        if (MidiInputCallback::onMidiSongSelectCallback) {
            MidiInputCallback::onMidiSongSelectCallback(midiInput, data1);
        }
    } else if (type == MidiMessage::Type::PitchBend) {
        if (MidiInputCallback::onMidiPitchBendCallback) {
            int value = data1 | (data2 << 7);
            MidiInputCallback::onMidiPitchBendCallback(
                midiInput, channel, value);
        }
    } else if (type == MidiMessage::Type::SongPosition) {
        if (MidiInputCallback::onMidiSongPositionCallback) {
            int position = data1 | (data2 << 7);
            MidiInputCallback::onMidiSongPositionCallback(midiInput, position);
        }
    } else if (type == MidiMessage::Type::ControlChange) {
        if (MidiInputCallback::onMidiControlChangeCallback) {
            MidiInputCallback::onMidiControlChangeCallback(
                midiInput, channel, data1, data2);
        }
    } else if (type == MidiMessage::Type::NoteOn) {
        if (MidiInputCallback::onMidiNoteOnCallback) {
            MidiInputCallback::onMidiNoteOnCallback(
                midiInput, channel, data1, data2);
        }
    } else if (type == MidiMessage::Type::NoteOff) {
        if (MidiInputCallback::onMidiNoteOffCallback) {
            MidiInputCallback::onMidiNoteOffCallback(
                midiInput, channel, data1, data2);
        }
    } else if (type == MidiMessage::Type::AfterTouchPoly) {
        if (MidiInputCallback::onMidiAfterTouchPolyCallback) {
            MidiInputCallback::onMidiAfterTouchPolyCallback(
                midiInput, channel, data1, data2);
        }
    }

    if (MidiInputCallback::onMidiMessageCallback) {
        MidiMessage midiMessage(channel, type, data1, data2);
        MidiInputCallback::onMidiMessageCallback(midiInput, midiMessage);
    }
}
