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
 * @file HardwareButton.cpp
 *
 * @brief A representation of a hardware Button.
 */

#include "core_pins.h"
#include "helpers.h"
#include "HardwareButton.h"
#include "pins_electra.h"

HardwareButton::HardwareButton(uint8_t newId)
    : id(newId),
      pressed(false),
      pinState(1),
      candidateState(1),
      candidateSince(0),
      longHold(false),
      pressStart(0)
{
}

bool HardwareButton::read(void)
{
    MUX_SET_ADDRESS(id);
    MUX_ENABLE;
    const uint8_t level = GET_BUTTON_STATE;
    MUX_DISABLE;

    /* Taken as settled, so that the scan neither reports this press again nor
     * a release that never happened once it starts. */
    pinState = level;
    candidateState = level;
    candidateSince = millis();
    pressed = (level == 0);

    if (pressed) {
        longHold = false;
        pressStart = millis();
    }

    return (pressed);
}

HardwareButton::Event HardwareButton::process(void)
{
    MUX_SET_ADDRESS(id);
    MUX_ENABLE;
    Event eventDetected = sample();
    MUX_DISABLE;

    return (eventDetected);
}

HardwareButton::Event HardwareButton::sample(void)
{
    Event eventDetected = none;

    /* Believe a level only once it has held still for debouncePeriod. A level
     * that differs from the one being timed restarts the clock, so bounce
     * never reaches the state machine below. */
    const uint8_t rawState = GET_BUTTON_STATE;
    const uint32_t now = millis();

    if (rawState != candidateState) {
        candidateState = rawState;
        candidateSince = now;
    } else if ((pinState != candidateState)
               && ((now - candidateSince) >= debouncePeriod)) {
        pinState = candidateState;
    }

    if ((pinState == 0) && (pressed == false)) {
        eventDetected = press;
        pressed = true;
        longHold = false;
        pressStart = millis();

        if (onPress) {
            onPress();
        }
    } else if (pinState == 0) {
        if (((millis() - pressStart) > longHoldPeriod) && (longHold == false)) {
            eventDetected = hold;
            longHold = true;

            if (onLongHold) {
                onLongHold();
            }
        }
    } else if ((pinState == 1) && (pressed == true)) {
        pressed = false;
        pressStart = 0;
        eventDetected = release;

        if (onRelease) {
            onRelease();
        }
    }

    return (eventDetected);
}

uint8_t HardwareButton::getId(void)
{
    return (id);
}

bool HardwareButton::isPressed(void)
{
    return (pressed);
}

bool HardwareButton::isLongHold(void)
{
    return (longHold);
}
