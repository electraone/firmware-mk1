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
 * @file PotListener.cpp
 *
 * @brief A listener of hardware potentiometer events.
 */

#include "PotListener.h"
#include "helpers.h"

PotBroadcaster PotListener::potBroadcaster;

PotListener::PotListener()
    : numValues(0), potId(AllPots), encoderMode(false), ratePot(0)
{
}

void PotListener::stopPropagation(void)
{
    potBroadcaster.stopPropagation();
}

void PotListener::assignPot(uint8_t newPotId, uint16_t newNumValues)
{
    potId = newPotId;
    numValues = newNumValues;
    potBroadcaster.addListener(this);
}

void PotListener::assignAllPots(uint16_t newNumValues)
{
    potId = AllPots;
    numValues = newNumValues;
    potBroadcaster.addListener(this);
}

void PotListener::enableEncoderMode(bool newEncoderMode)
{
    encoderMode = newEncoderMode;
}

void PotListener::releasePot(void)
{
    potBroadcaster.removeListener(this);
}

uint8_t PotListener::getPotId(void)
{
    return (potId);
}

void PotListener::setNumValues(uint16_t newNumValues)
{
    numValues = newNumValues;
}

uint16_t PotListener::getNumValues(void)
{
    return (numValues);
}

void PotListener::resetStepCount(void)
{
    rate.reset();
}

/**
 * @brief Turn rotation into a change of the listener's value.
 *
 * Rotation arrives as raw phase counts, POT_COUNTS_PER_REVOLUTION to a turn of
 * the knob. What that is worth is decided by ChangeAccumulator; all this adds is
 * what the component in front of it has to say about the value being driven.
 *
 * @param[in] relativeChange rotation since the last report, in phase counts
 * @param[in] turnedPotId the knob that turned
 *
 * @return the change to apply to the listener's value
 */
int16_t PotListener::computeRate(int16_t relativeChange, uint8_t turnedPotId)
{
    if (turnedPotId != ratePot) {
        rate.reset();
        ratePot = turnedPotId;
    }

    return (rate.compute(relativeChange,
                         numValuesForPot(turnedPotId),
                         sensitivityForPot(turnedPotId),
                         encoderMode));
}
