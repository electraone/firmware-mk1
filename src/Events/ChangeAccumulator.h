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
 * @file ChangeAccumulator.h
 *
 * @brief Turns raw movement of an input into a change of the value it drives.
 */

#pragma once

#include <cstdint>
#include <math.h>

/**
 * @brief Turns raw movement of an input into a change of the value it drives.
 *
 * Movement arrives in whatever counts the input reports - phase counts from a
 * knob - and on its own it means nothing. How much of a control's range a given
 * amount of movement is worth depends only on what kind of control it is:
 *
 * A proportional control - a fader, a knob, an envelope segment - spans its
 * whole range in @p SweepCounts of movement, whether it holds 128 values or
 * 16384. One reach of the wrist takes it from minimum to maximum, and the step
 * between two messages is simply the range divided by the sweep. Nothing is
 * multiplied by how fast the knob is being turned. The acceleration tables that
 * used to do that reached five hundred times at the top, so the same gesture
 * could move a parameter by a hundredth of its range or by half of it depending
 * on how quickly a finger happened to be moving, which is what made them feel
 * harsh - and what made a 14-bit parameter unreachable without a flick.
 *
 * A discrete control - a list, a pad - reads like an encoder instead: one step
 * per @p DetentCounts of movement, so stepping through it feels the same
 * whatever its length.
 *
 * The two counts are template parameters rather than members because there is
 * one accumulator per Component, and there are a great many Components.
 * Carrying the constants per instance would cost eight bytes each to say the
 * same thing every instance already says.
 *
 * @tparam SweepCounts movement that takes a proportional control end to end
 * @tparam DetentCounts movement worth one step of a discrete control
 */
template <uint32_t SweepCounts, uint32_t DetentCounts>
class ChangeAccumulator
{
public:
    ChangeAccumulator() : stepCount(0), residualChange(0.0f)
    {
    }

    /** @brief Forget movement banked towards the next step. */
    void reset(void)
    {
        stepCount = 0;
        residualChange = 0.0f;
    }

    /**
     * @brief Turn movement since the last report into a change of the value.
     *
     * @param rawChange movement since the last report, in the input's counts
     * @param numValues how many distinct values the driven value holds
     * @param sensitivity scale factor, below one for a finer knob
     * @param encoderMode read every control as discrete, whatever its range
     *
     * @return the change to apply to the value
     */
    int16_t compute(int16_t rawChange,
                    uint16_t numValues,
                    float sensitivity,
                    bool encoderMode)
    {
        /* Discrete controls, and any listener that never declared a range,
         * step. */
        if (encoderMode || (numValues == 0)) {
            return (steps(rawChange, sensitivity));
        }

        residualChange += ((float)rawChange * (float)numValues * sensitivity)
                          / (float)SweepCounts;

        /* Whole values go out now and the fraction stays behind, which is what
         * lets a control with few values still move one value at a time under a
         * slow turn rather than rounding the turn away. */
        float whole = truncf(residualChange);
        residualChange -= whole;

        return (clampToReport(whole));
    }

private:
    /**
     * @brief Step once per detent of movement, encoder fashion.
     */
    int16_t steps(int16_t rawChange, float sensitivity)
    {
        int32_t detent = (int32_t)((float)DetentCounts / sensitivity);

        if (detent < 1) {
            detent = 1;
        }

        stepCount += rawChange;

        /* Every whole detent banked is reported, not just one of them, so that
         * a quick flick through a long list travels as far as the hand did.
         * Dividing rather than stepping keeps this bounded: a large banked
         * movement would otherwise spin here once per detent. Integer division
         * truncates towards zero, which leaves the remainder with the sign of
         * the movement. */
        int32_t wholeSteps = stepCount / detent;

        stepCount -= wholeSteps * detent;

        return (clampToReport((float)wholeSteps));
    }

    /**
     * @brief Fit a change into the width the event carries, without wrapping.
     *
     * Movement banked while the outgoing port was busy can be larger than an
     * int16_t holds. Truncating it would reverse its sign, which reads as the
     * knob jumping backwards; clamping simply takes the control to the end of
     * its range, which is where that much movement was heading anyway.
     */
    static int16_t clampToReport(float change)
    {
        if (change > (float)INT16_MAX) {
            return (INT16_MAX);
        }

        if (change < (float)INT16_MIN) {
            return (INT16_MIN);
        }

        return ((int16_t)change);
    }

    /* Movement banked towards the next step of a discrete control. A slow turn
     * arrives in pieces far smaller than a detent, so this has to hold a whole
     * detent's worth without overflowing - and a value asking for the finest
     * sensitivity stretches a detent to half a million counts. */
    int32_t stepCount;

    /* Fraction of a value left over from the last report, carried so that
     * nothing is lost to rounding however slowly the knob turns. */
    float residualChange;
};
