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

#include "Hardware.h"
#include "System.h"
#include "Pot.h"
#include "pins_electra.h"

/** @todo: extract platform specific code to a ADC driver. */

Pot::Pot(uint8_t newId)
    : id(newId),
      step(0),
      active(false),
      touched(false),
      address(newId),
      trackedPhase(0),
      phaseValid(false)
{
}

/**
 * @brief Read both gangs and convert them into an absolute phase.
 *
 * Each gang is measured from its electrical mid-point and normalised to a common
 * amplitude, so that a difference in swing between the two gangs does not tilt
 * the diamond. The quadrant follows from the signs of the two normalised
 * readings, and the position within the quadrant is the normalised coordinate
 * divided by |a| + |b|. Dividing by the sum rather than assuming a fixed
 * amplitude is what keeps the phase continuous across the diamond's corners,
 * which is exactly where one gang is at its turning point and carries no usable
 * direction information.
 *
 * @param[out] phase the recovered phase, 0 to revolution - 1
 *
 * @return false when the reading cannot be placed on the diamond
 */
bool Pot::readPhase(int32_t &phase)
{
    MUX_SET_ADDRESS(this->address);
    MUX_ENABLE;
    const int32_t rawA = Hardware::adc.adc0->analogRead(14);
    const int32_t rawB = Hardware::adc.adc1->analogRead(35);
    MUX_DISABLE;

    const int32_t a = ((rawA - mid) * quadrant) / amplitude;
    const int32_t b = ((rawB - mid) * quadrant) / amplitude;

    const int32_t sum = ((a < 0) ? -a : a) + ((b < 0) ? -b : b);

    /* On the diamond the sum is the normalised amplitude. A reading well inside
     * it cannot be a real position - the wiper is crossing the gap in the track,
     * or has lifted - and its phase would be meaningless. A quarter of nominal
     * is the bar rather than a half, so that a gang which does not quite reach
     * both rails is still read rather than being declared dead. */
    if (sum < (quadrant / 4)) {
        return (false);
    }

    /* The walk starts where the first gang is at its positive end and goes the
     * way the reading this replaces called forwards, so a knob turns the same
     * way it always did. */
    if (a >= 0) {
        phase = (b >= 0) ? ((b * quadrant) / sum)
                         : (3 * quadrant) + ((a * quadrant) / sum);
    } else {
        phase = (b >= 0) ? quadrant + ((-a * quadrant) / sum)
                         : (2 * quadrant) + ((-b * quadrant) / sum);
    }

    return (true);
}

void Pot::process(void)
{
    int32_t phase;

    step = 0;

    if (!readPhase(phase)) {
        return;
    }

    if (!phaseValid) {
        trackedPhase = (int16_t)phase;
        phaseValid = true;
        return;
    }

    /* The shortest way round the diamond is the real one: anything further than
     * half a revolution between two readings is the wrap, not a jump. */
    int32_t delta = phase - (int32_t)trackedPhase;

    if (delta > (revolution / 2)) {
        delta -= revolution;
    } else if (delta < -(revolution / 2)) {
        delta += revolution;
    }

    if ((delta > -hysteresis) && (delta < hysteresis)) {
        return;
    }

    /* Drag the reference along behind the reading, keeping the backlash gap, so
     * that the movement beyond it is carried rather than discarded. */
    delta += (delta > 0) ? -hysteresis : hysteresis;

    int32_t tracked = (int32_t)trackedPhase + delta;

    if (tracked < 0) {
        tracked += revolution;
    } else if (tracked >= revolution) {
        tracked -= revolution;
    }

    trackedPhase = (int16_t)tracked;
    step = (int16_t)delta;
}

void Pot::resync(void)
{
    step = 0;
    phaseValid = false;
}

void Pot::initialise(void)
{
    step = 0;
    touched = false;
    active = false;
    phaseValid = false;

    int32_t phase;

    if (readPhase(phase)) {
        trackedPhase = (int16_t)phase;
        phaseValid = true;
    }
}
