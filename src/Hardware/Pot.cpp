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

Pot::Pot(uint8_t newId)
    : id(newId),
      step(0),
      moving(false),
      active(false),
      touched(false),
      A(0),
      B(0),
      minA(seedMid - seedAmplitude),
      maxA(seedMid + seedAmplitude),
      minB(seedMid - seedAmplitude),
      maxB(seedMid + seedAmplitude),
      decayCounter(0),
      trackedPhase(0),
      phaseValid(false),
      residual(0),
      recentPhase{ 0, 0 },
      recentCount(0)
{
}

/**
 * @brief Track the observed extremes of both gangs.
 *
 * The mid-point and swing of each gang are needed to place a reading on the
 * diamond, and neither is known in advance: they depend on the reference, on the
 * gang's end resistances and on how far the wiper actually travels. Both are
 * learned from the extremes seen so far, which converge after the knob has been
 * turned once all the way round. Widening is unconditional - a single stray
 * reading is cheap to accept - but only decayRange() narrows it back, and that
 * only runs on a sample already confirmed as real movement; see its comment.
 */
void Pot::learnRange(int16_t rawA, int16_t rawB)
{
    if (rawA < minA) {
        minA = rawA;
    }
    if (rawA > maxA) {
        maxA = rawA;
    }
    if (rawB < minB) {
        minB = rawB;
    }
    if (rawB > maxB) {
        maxB = rawB;
    }
}

/**
 * @brief Let a spuriously wide window creep back in, a count at a time.
 *
 * Only called once a sample has already cleared the hysteresis gate as genuine
 * movement - never on a knob sitting idle. Counted on every sample instead, it
 * ran from the background scan alone: measured on a mini, an untouched knob's
 * window lost a count a side every three seconds, from the seed down to
 * minRange over the better part of an hour. The calibration of a knob nobody
 * is moving should not move either. Most of that shrink is harmless - with
 * both bounds of both gangs creeping in together the mid-point stays put and
 * the phase, a ratio of the two gangs, does not change - but a gang resting
 * near a rail has one bound pinned by the reading, so its mid-point drifts
 * with the other, and the angle readPhase() recovers drifts with it. Gating on
 * real movement keeps the self-heal - a one-off wide reading narrows back down
 * once the knob is next turned - without ever touching the window of one left
 * alone.
 */
void Pot::decayRange(void)
{
    if (++decayCounter >= decayInterval) {
        decayCounter = 0;

        if ((maxA - minA) > minRange) {
            minA++;
            maxA--;
        }
        if ((maxB - minB) > minRange) {
            minB++;
            maxB--;
        }
    }
}

/**
 * @brief Read both gangs and convert them into an absolute phase.
 *
 * Each gang is normalised to a common amplitude first, so that a difference in
 * swing between the two gangs does not tilt the diamond. The quadrant follows
 * from the signs of the two normalised readings, and the position within the
 * quadrant is the normalised coordinate divided by |a| + |b|. Dividing by the
 * sum rather than assuming a fixed amplitude is what keeps the phase continuous
 * across the diamond's corners, which is exactly where one gang is at its
 * turning point and carries no usable direction information.
 *
 * @param[out] phase the recovered phase, 0 to revolution - 1
 *
 * @return false when the reading cannot be placed on the diamond
 */
bool Pot::readPhase(int32_t &phase)
{
    int32_t samplesA[oversample];
    int32_t samplesB[oversample];

    for (uint8_t i = 0; i < oversample; i++) {
        /* Gang A is on ADC0 and gang B on ADC1, so the two are converted at
         * the same instant rather than one after the other. */
        ADC::Sync_result sample = Hardware::adc.analogSynchronizedRead(14, 35);

        samplesA[i] = sample.result_adc0;
        samplesB[i] = sample.result_adc1;
    }

    A = (int16_t)trimmedMean(samplesA);
    B = (int16_t)trimmedMean(samplesB);

    learnRange(A, B);

    return (phaseOf(A, B, phase));
}

/**
 * @brief The backlash this reading has to clear before it counts as movement.
 *
 * Eight counts for a knob under a finger with both gangs clear of the
 * converter's rails; wideHysteresis for one at a corner or untouched - see the
 * two constants for why. Decided per reading, from the readings just taken
 * and the touch state readPotTouch() keeps in `touched`, so the gate
 * follows the knob into and out of a corner and the finger on and off the
 * cap. With knob touch turned off only the rail rule applies.
 */
int32_t Pot::gateFor(void) const
{
    const bool atRail = (A <= railMargin) || (A >= (adcFullScale - railMargin))
                        || (B <= railMargin)
                        || (B >= (adcFullScale - railMargin));

    /* The special boot that leaves knob touch off keeps `touched` false for
     * good, so the touch rule would widen every knob's gate. Without a sensor
     * the rail rule is all there is, as on an mk2 built without one. */
    const bool untouched = System::context.getTouchEnabled() && !touched;

    if (atRail || untouched) {
        return (wideHysteresis);
    }

    return (hysteresis);
}

/**
 * @brief The shortest way round the diamond.
 *
 * Anything further than half a revolution between two readings is the wrap,
 * not a jump.
 */
int32_t Pot::wrapDelta(int32_t delta) const
{
    if (delta > (revolution / 2)) {
        delta -= revolution;
    } else if (delta < -(revolution / 2)) {
        delta += revolution;
    }

    return (delta);
}

/**
 * @brief Reduce one batch of conversions to a reading, extremes discarded.
 *
 * A mean lets every conversion vote, and a knob at rest casts a bad one now
 * and then: one conversion in a batch 20 to 40 counts away from the others,
 * with the rest within a count or two of each other. Averaged in, it moves the
 * reading by a quarter of its error - enough to reach the backlash gate - and
 * because the gate drags the reference along, that one sample becomes a step
 * the knob never made. The old threshold decoder could not report movement
 * from a single sample; this one can, so the sample has to be sound. Sorting
 * the batch and dropping the ends on both sides leaves the outlier out of the
 * vote entirely, whichever way it went, and the middle four still average the
 * ordinary noise down as four conversions did before.
 *
 * @param samples the batch, reordered in place
 */
int32_t Pot::trimmedMean(int32_t *samples)
{
    for (uint8_t i = 1; i < oversample; i++) {
        int32_t key = samples[i];
        int8_t j = (int8_t)(i - 1);

        while ((j >= 0) && (samples[j] > key)) {
            samples[j + 1] = samples[j];
            j--;
        }

        samples[j + 1] = key;
    }

    int32_t sum = 0;

    for (uint8_t i = trim; i < (oversample - trim); i++) {
        sum += samples[i];
    }

    return (sum / (oversample - (2 * trim)));
}

/**
 * @brief Place one pair of gang readings on the diamond.
 *
 * Kept apart from the reading itself so that a pair can be scored without
 * being taken - the bench instrument that found the outliers compared several
 * reductions of the same batch this way.
 *
 * @param rawA gang A, in converter counts
 * @param rawB gang B, in converter counts
 * @param[out] phase the recovered phase, 0 to revolution - 1
 *
 * @return false when the reading cannot be placed on the diamond
 */
bool Pot::phaseOf(int32_t rawA, int32_t rawB, int32_t &phase) const
{
    int32_t amplitudeA = (maxA - minA) / 2;
    int32_t amplitudeB = (maxB - minB) / 2;

    if ((amplitudeA <= 0) || (amplitudeB <= 0)) {
        return (false);
    }

    int32_t a = ((rawA - ((maxA + minA) / 2)) * quadrant) / amplitudeA;
    int32_t b = ((rawB - ((maxB + minB) / 2)) * quadrant) / amplitudeB;

    int32_t sum = ((a < 0) ? -a : a) + ((b < 0) ? -b : b);

    /* On the diamond the sum is the normalised amplitude. A reading well inside
     * it cannot be a real position - the wiper is crossing the gap in the track,
     * or has lifted - and its phase would be meaningless. */
    if (sum < (quadrant / 2)) {
        return (false);
    }

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
    moving = false;

    if (!readPhase(phase)) {
        return;
    }

    if (!phaseValid) {
        trackedPhase = phase;
        phaseValid = true;
        return;
    }

    /* One reading on its own is not believed. The gate below is eight counts
     * and the trimmed reading of a knob at rest stays within six of its
     * reference for an hour at a time - and then, for a few minutes, single
     * readings land a count or two past the gate and each one would walk the
     * reference along. What brings those minutes on was not found (neither
     * CPU load on the host nor traffic on the USB port does it); the decoder
     * has to ride them out regardless. A real turn carries every reading
     * with it; a bad one is on its own. So the movement acted on is the
     * median of the last three readings' distances from the reference: a
     * lone outlier is outvoted, two in a row still count, and a turn loses
     * nothing but one scan slot of latency. */
    int32_t delta = wrapDelta(phase - trackedPhase);

    if (recentCount == 2) {
        int32_t a = delta;
        int32_t b = wrapDelta(recentPhase[0] - trackedPhase);
        int32_t c = wrapDelta(recentPhase[1] - trackedPhase);

        if ((a <= b && b <= c) || (c <= b && b <= a)) {
            delta = b;
        } else if ((b <= a && a <= c) || (c <= a && a <= b)) {
            delta = a;
        } else {
            delta = c;
        }
    }

    recentPhase[1] = recentPhase[0];
    recentPhase[0] = phase;

    if (recentCount < 2) {
        recentCount++;
    }

    const int32_t gate = gateFor();

    if ((delta > -gate) && (delta < gate)) {
        return;
    }

    moving = true;
    decayRange();

    /* Drag the reference along behind the reading, keeping the backlash gap, so
     * that the movement beyond it is carried rather than discarded. */
    delta += (delta > 0) ? -gate : gate;

    trackedPhase += delta;

    if (trackedPhase < 0) {
        trackedPhase += revolution;
    } else if (trackedPhase >= revolution) {
        trackedPhase -= revolution;
    }

    /* Whole steps go out now, the fraction stays behind for the next reading. */
    residual += delta;

    int32_t steps = residual / countsPerStep;

    if (steps != 0) {
        residual -= steps * countsPerStep;
        step = (int16_t)steps;
    }
}

void Pot::initialise(void)
{
    step = 0;
    moving = false;
    touched = false;
    active = false;

    phaseValid = false;
    residual = 0;
    decayCounter = 0;
    recentCount = 0;

    minA = seedMid - seedAmplitude;
    maxA = seedMid + seedAmplitude;
    minB = seedMid - seedAmplitude;
    maxB = seedMid + seedAmplitude;

    int32_t phase;

    if (readPhase(phase)) {
        trackedPhase = phase;
        phaseValid = true;
    }
}
