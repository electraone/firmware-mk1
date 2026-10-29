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

#include "SysexCallbacks.h"
#include "SysexHandler.h"
#include "SysexHandlerCtrl.h"

/* The handlers fill and copy SysEx buffers from the MIDI interrupt, so they
 * are kept below the SRAM bank boundary at 0x20000000, where an access
 * spanning SRAM_L and SRAM_U hard faults. See the linker script's
 * .bss.lowram. */
#define LOWRAM __attribute__((section(".bss.lowram")))

static LOWRAM SysexHandler handlerMidiIo1(MidiInterface::Type::MidiIo, 0);
static LOWRAM SysexHandler handlerMidiIo2(MidiInterface::Type::MidiIo, 1);
static LOWRAM SysexHandler handlerMidiUsbDev1(MidiInterface::Type::MidiUsbDev, 0);
static LOWRAM SysexHandler handlerMidiUsbDev2(MidiInterface::Type::MidiUsbDev, 1);
static LOWRAM SysexHandler handlerMidiUsbHost1(MidiInterface::Type::MidiUsbHost, 0);
static LOWRAM SysexHandler handlerMidiUsbHost2(MidiInterface::Type::MidiUsbHost, 1);
/*
 * One Electra command handler per USB device cable, not one for all three.
 * An Electra message arriving on Port 1 or Port 2 used to be handed to the
 * CTRL cable's handler, with one flag saying "an Electra message is in
 * progress" - which assumed that two of them never interleave. They do: a
 * primary drives the Electra Link on the CTRL cable in messages of several
 * packets, and a request bridged in on Port 1 that landed between two of
 * those packets was appended to the link message and lost, and the link
 * message with it. Each cable now keeps its own message and its own flag.
 */
static LOWRAM SysexHandlerCtrl handlerMidiUsbDevCtrl0(MidiInterface::Type::MidiUsbDev,
                                               0);
static LOWRAM SysexHandlerCtrl handlerMidiUsbDevCtrl1(MidiInterface::Type::MidiUsbDev,
                                               1);
static LOWRAM SysexHandlerCtrl handlerMidiUsbDevCtrl(MidiInterface::Type::MidiUsbDev,
                                              2);
static LOWRAM SysexHandlerCtrl handlerMidiUsbHostCtrl(MidiInterface::Type::MidiUsbHost,
                                               2);

/* From the main loop: messages the CTRL handlers kept back during a repaint.
 * See SysexHandlerCtrl::flushDeferred(). */
void flushDeferredCtrlSysex(void)
{
    handlerMidiUsbDevCtrl0.flushDeferred();
    handlerMidiUsbDevCtrl1.flushDeferred();
    handlerMidiUsbDevCtrl.flushDeferred();
    handlerMidiUsbHostCtrl.flushDeferred();
}

void handleMidiIoSysExPort0(const uint8_t *sysExData,
                            uint16_t sysExSize,
                            bool complete)
{
    if (MidiInputCallback::routePartialSysexCallback) {
        MidiInput midiInput(MidiInterface::Type::MidiIo, 0);
        MidiInputCallback::routePartialSysexCallback(
            midiInput, sysExData, sysExSize, complete);
    }
    handlerMidiIo1.process(sysExData, sysExSize, complete);
}

void handleMidiIoSysExPort1(const uint8_t *sysExData,
                            uint16_t sysExSize,
                            bool complete)
{
    if (MidiInputCallback::routePartialSysexCallback) {
        MidiInput midiInput(MidiInterface::Type::MidiIo, 1);
        MidiInputCallback::routePartialSysexCallback(
            midiInput, sysExData, sysExSize, complete);
    }
    handlerMidiIo2.process(sysExData, sysExSize, complete);
}

void handleMidiUsbDevSysEx(uint8_t cable,
                           const uint8_t *sysExData,
                           uint16_t sysExSize,
                           bool complete)
{
    // \todo accepting Electra SysEx messages on non-CTRL port will be replaced
    // with message routing. This is a quick fix for now, to allow @jhh working
    // on his ableton preset.
    static bool electraCommand[3] = { false, false, false };
    static SysexHandlerCtrl *const ctrlHandler[3] = { &handlerMidiUsbDevCtrl0,
                                                      &handlerMidiUsbDevCtrl1,
                                                      &handlerMidiUsbDevCtrl };

    if (cable > 2) {
        return;
    }

    if (sysExData[0] == 0xF0 && sysExData[1] == 0x00 && sysExData[2] == 0x21
        && sysExData[3] == 0x45) {
        electraCommand[cable] = true;
    }

    if (!electraCommand[cable] && (cable == 0 || cable == 1)) {
        if (MidiInputCallback::routePartialSysexCallback) {
            MidiInput midiInput(MidiInterface::Type::MidiUsbDev, cable);
            MidiInputCallback::routePartialSysexCallback(
                midiInput, sysExData, sysExSize, complete);
        }
    }

    if (electraCommand[cable]) {
        ctrlHandler[cable]->process(sysExData, sysExSize, complete, cable);
    } else if (cable == 0) {
        handlerMidiUsbDev1.process(sysExData, sysExSize, complete);
    } else if (cable == 1) {
        handlerMidiUsbDev2.process(sysExData, sysExSize, complete);
    } else {
        handlerMidiUsbDevCtrl.process(sysExData, sysExSize, complete);
    }

    if (complete) {
        electraCommand[cable] = false;
    }
}

void handleMidiUsbHostSysExPort0(uint8_t cable,
                                 const uint8_t *sysExData,
                                 uint16_t sysExSize,
                                 bool complete)
{
    if (cable != 2 && MidiInputCallback::routePartialSysexCallback) {
        MidiInput midiInput(MidiInterface::Type::MidiUsbHost, 0);
        MidiInputCallback::routePartialSysexCallback(
            midiInput, sysExData, sysExSize, complete);
    }
    if (cable == 2) {
        handlerMidiUsbHostCtrl.process(sysExData, sysExSize, complete);
    } else {
        handlerMidiUsbHost1.process(sysExData, sysExSize, complete);
    }
}

void handleMidiUsbHostSysExPort1(uint8_t cable,
                                 const uint8_t *sysExData,
                                 uint16_t sysExSize,
                                 bool complete)
{
    if (cable != 2 && MidiInputCallback::routePartialSysexCallback) {
        MidiInput midiInput(MidiInterface::Type::MidiUsbHost, 1);
        MidiInputCallback::routePartialSysexCallback(
            midiInput, sysExData, sysExSize, complete);
    }
    if (cable == 2) {
        handlerMidiUsbHostCtrl.process(sysExData, sysExSize, complete);
    } else {
        handlerMidiUsbHost2.process(sysExData, sysExSize, complete);
    }
}
