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

#include "Profile.h"

#if defined(PROFILE) && PROFILE

#include "System.h"

namespace Profile
{
    Stat stats[Count];

    const char *const names[Count] = { "potStep",    "midiIn",     "midiInIsr",
                                       "sendMessage", "pageSwitch",
                                       "repaint",    "presetLoad", "sysexMatch",
                                       "usbSend",    "sysexSend" };

    void dump(void)
    {
        for (uint8_t i = 0; i < Count; i++) {
            const Stat &stat = stats[i];

            if (stat.count == 0) {
                continue;
            }
            /* Cycles to microseconds at F_CPU. */
            const uint32_t perUs = F_CPU / 1000000;
            System::logger.write(LOG_ERROR,
                                 "profile: %s n=%lu avg=%luus worst=%luus "
                                 "best=%luus",
                                 names[i],
                                 (unsigned long)stat.count,
                                 (unsigned long)(stat.total / stat.count / perUs),
                                 (unsigned long)(stat.worst / perUs),
                                 (unsigned long)(stat.best / perUs));
        }
    }

    void reset(void)
    {
        for (auto &stat : stats) {
            stat = Stat{ 0, 0, 0, UINT32_MAX };
        }
    }
} // namespace Profile

#endif
