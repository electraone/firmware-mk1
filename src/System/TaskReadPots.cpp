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
#include "TaskReadButtons.h"
#include "Hardware.h"
#include "System.h"
#include "PotListener.h"
#include "pins_electra.h"

#include "Profile.h"

#if defined(PROFILE) && PROFILE
volatile uint32_t profileSyntheticPotSteps = 0;
volatile uint8_t profileSyntheticPotId = 1;
#endif

/* millis() value up to which a knob keeps its place in the fast rota. */
static uint32_t potActiveUntil[NR_OF_HW_POTS];

/* The address the multiplexer rests on until the next tick, and which rota
 * chose it. */
static uint8_t parked = 0;
static bool parkedIsBackground = true;

static uint8_t backgroundAddress = 0;
static uint8_t activeAddress = 0;
static bool serveActiveNext = false;

static void park(uint8_t address)
{
    MUX_SET_ADDRESS(address);
    MUX_ENABLE;
}

/**
 * @brief Pick the multiplexer address to rest on for the next tick.
 *
 * The two rotas take turns while anything is moving, so a single knob under a
 * finger is read on every other tick and the rest of the panel keeps being
 * swept in between. With nothing moving the background rota takes every tick.
 */
static uint8_t chooseNextAddress(bool &isBackground)
{
    const uint32_t now = millis();
    uint8_t activeCount = 0;

    for (uint8_t address = 0; address < NR_OF_HW_POTS; address++) {
        if ((int32_t)(potActiveUntil[address] - now) > 0) {
            activeCount++;
        }
    }

    if ((activeCount > 0) && serveActiveNext) {
        for (uint8_t i = 0; i < NR_OF_HW_POTS; i++) {
            activeAddress = (uint8_t)((activeAddress + 1) % NR_OF_HW_POTS);

            if ((int32_t)(potActiveUntil[activeAddress] - now) > 0) {
                serveActiveNext = false;
                isBackground = false;
                return (activeAddress);
            }
        }
    }

    serveActiveNext = (activeCount > 0);
    isBackground = true;

    const uint8_t address = backgroundAddress;
    backgroundAddress = (uint8_t)((backgroundAddress + 1) % NR_OF_HW_POTS);

    return (address);
}

/**
 * @brief Read the knobs and the buttons behind the multiplexer.
 *
 * The mk2's scan. Only one multiplexer address is read per tick, and the
 * multiplexer is moved to the next address at the end of it, so every reading
 * is taken after the address has been held for a whole tick: switching the
 * multiplexer dumps the charge on its common node onto the selected wiper,
 * which needs time to recover, and a reading taken straight after selecting a
 * knob carries a little of the knob read before it.
 *
 * Addresses are handed out by two rotas. The background rota visits every knob
 * in turn and never skips one, so a knob is read whether or not anything is
 * touching it. The fast rota holds only the knobs that have moved recently and
 * is interleaved with the background one, which is what gives a knob being
 * turned far more readings than the panel as a whole gets. Nothing here
 * consults the touch sensor: a knob the sensor cannot see works just as well,
 * and the sensor only decides how wide its backlash is - see Pot::gateFor().
 *
 * The buttons share the multiplexer, so a button at the parked address is
 * read without moving it. Only the background rota does that, which keeps the
 * buttons on a cadence of their own instead of one that speeds up and slows
 * down with whichever knob is being turned.
 */
void readPots(void)
{
#if defined(PROFILE) && PROFILE
    if (profileSyntheticPotSteps > 0) {
        profileSyntheticPotSteps--;
        PROFILE_BEGIN(PotStep);
        PotListener::potBroadcaster.potChange(profileSyntheticPotId, 4);
        PROFILE_END(PotStep);
        return;
    }
#endif

    /* Something else moved the multiplexer since the last tick - the boot's
     * reads, or the button task while the controller slept. A reading now
     * would be taken without settling, or from the wrong knob, so this tick
     * only puts the multiplexer back. */
    if (!MUX_IS_PARKED_AT(parked)) {
        park(parked);
        return;
    }

    Pot &pot = Hardware::pots[parked];

    PROFILE_BEGIN(PotRead);
    pot.process();
    PROFILE_END(PotRead);

    if (pot.moving) {
        potActiveUntil[parked] = millis() + POT_ACTIVE_HOLD_MS;
    }

    if (pot.step != 0) {
        PROFILE_BEGIN(PotStep);
        PotListener::potBroadcaster.potChange(pot.id, pot.step);
        PROFILE_END(PotStep);
    }

    if (parkedIsBackground && (parked < NR_OF_HW_BUTTONS)) {
        HardwareButton *button = Hardware::buttons[parked];

        dispatchButtonEvent(button, button->sample());
    }

    /* A listener may have done anything, the boot's button reads included, so
     * the multiplexer is put where it should be rather than assumed there. */
    parked = chooseNextAddress(parkedIsBackground);
    park(parked);
}
