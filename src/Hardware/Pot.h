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
 * @file Pot.h
 *
 * @brief A representation of a hardware Potentiometer.
 */

#pragma once

#include <ADC.h>

#include "settings.h"

extern ADC *adc;

/*
 * Pot class
 *
 * A representation of the physical turning knob.
 *
 * The knobs are dual-gang continuous-rotation potentiometers. The two gangs sit
 * a quarter turn apart on the shaft, so their wipers produce a pair of
 * quadrature triangle waves. Plotted against each other those two waves trace a
 * diamond: with a and b measured from the electrical mid-point of each gang,
 * |a| + |b| is constant, and one revolution of the knob walks once around the
 * diamond's perimeter.
 *
 * That makes the pair an absolute angle sensor rather than an encoder.
 * process() recovers the angle as a phase in the range 0..revolution and
 * reports rotation as the wrapped difference between consecutive phases.
 * Nothing is thresholded away beyond the backlash below: what used to happen
 * here was a divide by sixteen whose remainder was thrown on the floor, so a
 * slow turn lost up to half of itself before anything downstream saw it.
 */
class Pot
{
public:
    Pot(uint8_t id);
    void process(void);
    void initialise(void);

    /**
     * @brief Forget where the knob was, so the next reading is the reference.
     *
     * A knob is only read while a finger is on it, so it can be turned while
     * nothing is watching - by a hand the capacitive sensor did not register, or
     * in transit. The tracked phase is then stale by up to half a revolution,
     * and the first reading of the next touch would report all of it as one
     * movement. Called when a knob is touched: the reading after it reports
     * nothing and becomes the new reference.
     */
    void resync(void);

    uint8_t id;

    /* Rotation since the last reading, in phase counts. The listeners divide it
     * by the travel a control is worth - see ChangeAccumulator - so reporting
     * the counts rather than whole steps is what lets the same turn mean one
     * value of a short list and a hundred of a 14-bit parameter. */
    int16_t step;

    bool active;
    bool touched;

private:
    bool readPhase(int32_t &phase);

    /* Counts of phase per quadrant of the diamond, and per whole revolution.
     * A quadrant traverses one gang from end to end, so the converter's ten
     * bits are what there is to resolve it with: 1024 per quadrant, 4096 to the
     * turn, which is POT_COUNTS_PER_REVOLUTION. */
    static constexpr int32_t quadrant = 1024;
    static constexpr int32_t revolution = 4 * quadrant;

    /* The electrical mid-point of a gang and its swing to either side of it, in
     * converter counts. A gang divides the reference across its whole track and
     * this converter reads ten bits, so it sits at mid-scale and swings to both
     * rails - which is what the reading this replaces already assumed when it
     * tested a gang against 0, 512 and 1023.
     *
     * Nothing is learned at runtime, unlike on the newer hardware: the phase is
     * recovered by dividing by |a| + |b|, so a swing that is not quite the full
     * range cancels out of the division. Only a difference between the two gangs
     * tilts the diamond, and that shows as a fraction of a degree of angle
     * rather than as anything a hand would notice. */
    static constexpr int32_t mid = 512;
    static constexpr int32_t amplitude = 512;

    /* Backlash applied to the tracked phase, in counts. A stationary knob
     * dithers by a count or two of converter noise; holding the reference still
     * until the reading moves further than this keeps that dither off the wire,
     * while a genuinely slow turn drags the reference along behind it and loses
     * only this fixed amount on each reversal.
     *
     * Eight counts is four counts of the converter and about three quarters of a
     * degree. It has to sit above the noise floor rather than merely at it,
     * because a proportional control no longer scales small movements down: a
     * band the noise can cross even occasionally would let a parameter creep on
     * its own. Measured on the bench, a stationary knob's recovered phase wanders
     * by two counts peak to peak, so this leaves four times that in hand. The
     * reading this replaces needed sixteen counts of the converter before it
     * reported anything at all, which is four times coarser than this.
     *
     * What it costs is backlash on a reversal, paid once: eight counts of four
     * thousand and ninety six, which is a fifth of one percent of a control's
     * range - a fiftieth of a value on a 7-bit parameter, and thirty eight of the
     * sixteen thousand on a 14-bit one. */
    static constexpr int32_t hysteresis = 8;

    /* The listeners scale what they are given by POT_SWEEP_COUNTS, which is
     * derived from POT_COUNTS_PER_REVOLUTION, and this is where the counts come
     * from. The two have to say the same thing or every control is scaled by
     * the ratio between them. */
    static_assert(revolution == POT_COUNTS_PER_REVOLUTION,
                  "the pot reader and the settings disagree on counts per turn");

    uint8_t address;
    int16_t trackedPhase;
    bool phaseValid;
};
