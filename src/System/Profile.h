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
 * @file Profile.h
 *
 * @brief Cycle-counted timing of the paths a player feels: a knob step,
 *        an incoming MIDI message, a page switch, a preset load.
 *
 * Compiled in only with -DPROFILE (OPTIONS="... -DPROFILE", in its own
 * BUILDDIR). Without it every macro is nothing. With it each probe keeps
 * the count, the sum and the worst case of the cycles between PROFILE_BEGIN
 * and PROFILE_END, read from the Cortex-M4's DWT cycle counter, and the
 * ten-second memory monitor task writes them all to the log in
 * microseconds. A knob step is a few hundred cycles of bookkeeping, so the
 * probe costs less than what it measures, but it is not free, which is why
 * it is not in the production build.
 */

#pragma once

#include <cstdint>

#if defined(PROFILE) && PROFILE

#include "mk66fx1m0.h"

namespace Profile
{
    enum Probe : uint8_t {
        PotStep = 0, // readPots: one pot step through to the MIDI queue
        MidiIn, // processMidi: one incoming message, all consumers
        MidiInIsr, // readMidi: one pass of the timer interrupt
        SendMessage, // Midi::sendMessage: one message built and queued
        
        PageSwitch, // MainWindow::displayPage: the page rebuilt
        Repaint, // repaintGraphics: one pass over the queue
        PresetLoad, // Presets::loadPreset, parse to parameter map
                SysexMatch, // Midi::processSysex: one block against every template
        UsbSend, // MidiOutput::send: one channel message onto the wire
        SysexSend, // MidiOutput::send: one SysEx block onto the wire
        Count
    };

    struct Stat {
        uint32_t count;
        uint64_t total;
        uint32_t worst;
        uint32_t best;
    };

    extern Stat stats[Count];
    extern const char *const names[Count];

    inline void initialise(void)
    {
        ARM_DEMCR |= ARM_DEMCR_TRCENA;
        ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
        for (auto &stat : stats) {
            stat = Stat{ 0, 0, 0, UINT32_MAX };
        }
    }

    inline uint32_t now(void)
    {
        return (ARM_DWT_CYCCNT);
    }

    inline void record(Probe probe, uint32_t startedAt)
    {
        const uint32_t cycles = ARM_DWT_CYCCNT - startedAt;
        Stat &stat = stats[probe];
        stat.count++;
        stat.total += cycles;
        if (cycles > stat.worst) {
            stat.worst = cycles;
        }
        if (cycles < stat.best) {
            stat.best = cycles;
        }
    }

    void dump(void);
    void reset(void);
} // namespace Profile

#define PROFILE_INIT() Profile::initialise()
#define PROFILE_BEGIN(name) const uint32_t profileStart_##name = Profile::now()
#define PROFILE_END(name) Profile::record(Profile::name, profileStart_##name)
#define PROFILE_DUMP() Profile::dump()

#else

#define PROFILE_INIT() ((void)0)
#define PROFILE_BEGIN(name) ((void)0)
#define PROFILE_END(name) ((void)0)
#define PROFILE_DUMP() ((void)0)

#endif
