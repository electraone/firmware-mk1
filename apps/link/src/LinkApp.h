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
 * @file LinkApp.h
 *
 * @brief The Electra Link program: an Electra One mk1 as a second screen.
 *
 * The third program in apps/, beside the demo and the controller. The unit
 * it runs on is not a controller: it holds no preset, no parameter values of
 * its own, no Lua state and no router, and it decides nothing. It is a screen
 * and twelve knobs that an Electra One mk2 drives over the Electra Link, and
 * every pixel on it is something the primary asked for.
 *
 * It is a separate image rather than a mode of the controller because that
 * is what makes the claim above checkable: what a satellite cannot do, it
 * cannot do because the code is not running, not because a flag says not to.
 *
 * The boot path is shared. main.cpp brings up the hardware, the display and
 * the MIDI interfaces before calling initialise() here, exactly as it does
 * for the controller. See apps/link/README.md.
 */

#pragma once

#include "App.h"
#include "MidiInputCallback.h"
#include "LinkProgram.h"
#include "LinkTransport.h"

class LinkWindow;

class LinkApp : public App, private MidiInputCallback
{
public:
    LinkApp();
    ~LinkApp() override = default;

    void initialise(void) override;

    const char *getApplicationName(void) const override
    {
        return ("Electra Link");
    }

    const char *getApplicationVersion(void) const override
    {
        return ("1.0.0");
    }

    /** Where the program would keep private data on the card: not the
     *  controller's area, on a unit that has been both. */
    const char *getApplicationSandbox(void) const override
    {
        return ("link");
    }

    /** @brief One pass of the program, from its task. */
    void tick(void);

    /** @brief The main loop is busy streaming a file: keep the session up. */
    void yield(void) override;

private:
    /**
     * @brief An Electra SysEx request the base firmware left to the app.
     *
     * A satellite answers what a host on a bench needs of it - a picture of
     * the screen and the files that holds it - and refuses the rest with a
     * NACK rather than silence, so a host's queue never stalls on it.
     */
    void handleElectraSysex(uint8_t port,
                            const SysexBlock &sysexBlock) override;

    /** @brief Plain MIDI. A satellite has no preset to play it into. */
    void handleIncomingMidiMessage(const MidiInput &midiInput,
                                   const MidiMessage &midiMessage) override;

    LinkTransport transport;
    LinkProgram program;
    LinkWindow *window;
};
