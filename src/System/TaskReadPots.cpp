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

#include "TaskReadPots.h"
#include "Hardware.h"
#include "System.h"
#include "PotListener.h"

/*
 * read HW control
 */
#include "Profile.h"

#if defined(PROFILE) && PROFILE
/* A knob turned from the debugger: while this is non-zero, every scan
 * reports one step of the pot named below instead of reading the ADC, and
 * counts down. Written over J-Link, so a knob path can be measured without
 * a hand on the instrument. */
volatile uint32_t profileSyntheticPotSteps = 0;
volatile uint8_t profileSyntheticPotId = 1;
#endif

void readPots(void)
{
    Pot *pot = NULL;

#if defined(PROFILE) && PROFILE
    if (profileSyntheticPotSteps > 0) {
        profileSyntheticPotSteps--;
        PROFILE_BEGIN(PotStep);
        PotListener::potBroadcaster.potChange(profileSyntheticPotId, 4);
        PROFILE_END(PotStep);
        return;
    }
#endif

    if (System::context.getTouchEnabled() == false) {
        pot = Hardware::pots.getNextPot();
    } else {
        pot = Hardware::pots.getNextActivePot();
    }

    if (pot == NULL) {
        return;
    }

    pot->process();

    if (pot->step != 0) {
#ifdef DEBUG
        System::logger.write(LOG_ERROR,
                             "readPots: pot moved: potId=%d, step=%d",
                             pot->id,
                             pot->step);
#endif /* DEBUG */
                PROFILE_BEGIN(PotStep);
        PotListener::potBroadcaster.potChange(pot->id, pot->step);
        PROFILE_END(PotStep);

    }
}
