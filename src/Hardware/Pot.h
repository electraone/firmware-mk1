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

#include <cstdint>

#include "settings.h"

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
 * That makes the pair an absolute angle sensor rather than an encoder. process()
 * recovers the angle as a phase in the range 0..revolution, and reports rotation
 * as the wrapped difference between consecutive phases. Nothing is thresholded
 * away: the fraction of a step left over after each report is carried into the
 * next one, so an arbitrarily slow turn still accumulates to full steps.
 *
 * This is the mk2's decoder, line for line apart from where the conversions
 * come from, so a knob behaves the same on both models: the counts per turn,
 * the backlash, the defences against a bad conversion and the learned
 * calibration were all measured there. A Pot does not select itself on the
 * multiplexer - the scan in TaskReadPots parks the multiplexer on a knob for a
 * whole tick before process() reads it.
 */
class Pot
{
public:
    Pot(uint8_t id);
    void process(void);
    void initialise(void);

    uint8_t id;
    int16_t step;

    /* True when the last reading moved the phase past the backlash gap, whether
     * or not that was enough to report a whole step. The scan uses it to decide
     * which knobs deserve the fast slots, so it has to react to the very first
     * hint of movement, well before a step comes out. */
    bool moving;

    bool active;
    bool touched;

private:
    void learnRange(int16_t rawA, int16_t rawB);
    void decayRange(void);
    bool readPhase(int32_t &phase);
    bool phaseOf(int32_t rawA, int32_t rawB, int32_t &phase) const;
    int32_t gateFor(void) const;
    static int32_t trimmedMean(int32_t *samples);
    int32_t wrapDelta(int32_t delta) const;

    /* Counts of phase per quadrant of the diamond, and per whole revolution.
     * 2048 per quadrant gives 8192 counts per turn, which is what a 12-bit
     * reading of both gangs can resolve. */
    static constexpr int32_t quadrant = 2048;
    static constexpr int32_t revolution = 4 * quadrant;

    /* Movement is reported in raw phase counts, 8192 to the revolution. The
     * listeners scale it by the travel a control is worth - see
     * ChangeAccumulator. Reporting the counts rather than whole detents is what
     * lets a fast turn arrive as a stream of small moves instead of a few large
     * ones. */
    static constexpr int32_t countsPerStep = 1;

    /* The listeners scale what they are given by POT_SWEEP_COUNTS, which is
     * derived from POT_COUNTS_PER_REVOLUTION, and this is where the counts come
     * from. The two have to say the same thing or every control is scaled by
     * the ratio between them. */
    static_assert(revolution == POT_COUNTS_PER_REVOLUTION,
                  "the pot reader and the settings disagree on counts per turn");

    /* Backlash applied to the tracked phase, in counts. A stationary knob dithers
     * by a couple of counts of ADC noise; holding the reference still until the
     * reading moves further than this keeps that dither off the wire, while a
     * genuinely slow turn drags the reference along behind it and loses only
     * this fixed amount on each reversal.
     *
     * It has to sit above the noise floor rather than merely at it. A knob at
     * rest is sampled hundreds of times a second, so a band the noise can cross
     * even occasionally lets through a trickle of movement all day - which is
     * exactly what a proportional control now shows, because it no longer scales
     * the movement down when the knob is barely turning. The floor is set by a
     * gang resting at a rail - the diamond's corner, where the phase is read
     * from the other gang's noise alone: at six counts an untouched mini knob
     * sitting there reached the gate once in an hour (a sample at exactly six,
     * carried movement zero), and one count more would have been a step. Eight
     * counts is a third of a degree of backlash, and about a tenth of a percent
     * of a 14-bit range. */
    static constexpr int32_t hysteresis = 8;

    /* The wider backlash for a reading that cannot be trusted to eight counts.
     *
     * Two situations earn it, and they are the two in which a knob nobody is
     * turning has been seen to move. One is a gang resting against a rail of
     * the converter: the readings of an mk2 at rest shift together by ten to
     * sixteen counts for seconds at a time, and mid-diamond that shift
     * cancels between the gangs, but a gang that is clipped at 0 or 4095
     * cannot follow it, so the whole shift lands on the phase. The other is
     * simply that nobody is touching the knob: a real turn arrives with a
     * finger on the cap, and every phantom move by definition does not. On
     * a knob at a corner, or an untouched one, the gate is this instead.
     *
     * It costs nothing a player can feel. A quick movement clears any gate
     * on its first sample, so there is no delay; a slow turn with the knob
     * untouched - a glove, a fingernail, a sensor that has lost its
     * calibration to wet fingers - gives up the first degree of travel and
     * then tracks as it always did. The sensor is never a condition for the
     * knob to work, only for how fine the first step is. And a sensor stuck
     * at "touched" leaves exactly today's behaviour, where the rail rule
     * still covers the geometry that actually drifts. */
    static constexpr int32_t wideHysteresis = 24;

    /* How close to 0 or to full scale a gang's reading has to sit for the
     * rail rule above to apply. Twelve counts is well inside the clipping
     * region the shifts were measured in and far outside the two or three
     * counts of noise a gang shows anywhere else on the diamond. */
    static constexpr int32_t railMargin = 12;
    static constexpr int32_t adcFullScale = 4095;

    /* Seeds for the learned calibration, used until the knob has been turned far
     * enough to observe its real extremes. A gang divides the reference across
     * the full track, so it sits near mid-scale and swings most of the way to
     * both rails. */
    static constexpr int32_t seedMid = 2048;
    static constexpr int32_t seedAmplitude = 1800;

    /* How many samples of confirmed movement pass before the learned extremes
     * are pulled in by one count - see decayRange(). Without this a single
     * noise spike would widen the range for good. */
    static constexpr uint16_t decayInterval = 512;

    /* Floor below which decayRange() must not shrink a gang's range further. A
     * knob worked within a narrow arc of its own travel never re-widens the
     * range, so left unbounded the decay eventually collapses it down to
     * whatever the current reading snaps it to - a handful of counts.
     * readPhase() then divides by that amplitude, so the same couple of counts
     * of ADC noise that six counts of hysteresis is meant to absorb instead maps
     * to a huge phase swing and reports a knob turning on its own. Half the
     * seeded amplitude stays far enough above the post-averaging noise floor to
     * keep that division safe, while still letting a genuinely spurious wide
     * reading decay back down. */
    static constexpr int32_t minRange = seedAmplitude / 2;

    /* How many conversions are taken for each reading, and how many of the
     * highest and the lowest are thrown away before the rest are averaged. The
     * multiplexer is parked on this knob for a whole tick, so the conversions
     * cost CPU time and nothing else - the scan tick sets the sample rate, not
     * the converter. The converter averages four in hardware, as the mk2's
     * does, so each of these is four of those; both gangs are converted at
     * once, one on each of the two converters. See Hardware::initialise().
     *
     * Trimming is what keeps a single bad conversion out of the reading. One
     * conversion in a batch lands 20 to 40 counts off its neighbours now and
     * then - measured on an mk2 at rest, roughly once in a few thousand
     * samples - and in a plain mean of four that moves the reading by six to
     * ten counts, enough to clear the backlash gate and report a knob nobody
     * is touching as turned. The same batches with the extremes discarded
     * never came within two counts of the gate. See trimmedMean(). Eight
     * rather than four: with four trimmed to the middle two, a knob at rest
     * on an mk2 cleared the gate dozens of times in ten minutes - a clear
     * moves the knob into the fast scan rota, and a two-deep average is
     * noisy enough to keep it there - where eight trimmed to four gave none.
     * The scan polls the converter, so the extra conversions are CPU taken
     * from everything below this thread; it measured within the noise of the
     * perf section's idle figure. */
    static constexpr uint8_t oversample = 8;
    static constexpr uint8_t trim = 2;

    int16_t A;
    int16_t B;

    int32_t minA, maxA, minB, maxB;
    uint16_t decayCounter;

    int32_t trackedPhase;
    bool phaseValid;
    int32_t residual;

    /* The two readings before the current one. A reading only moves the
     * reference when the median of the three agrees - see process(). */
    int32_t recentPhase[2];
    uint8_t recentCount;
};
