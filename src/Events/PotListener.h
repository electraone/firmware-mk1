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
 * @file PotListener.h
 *
 * @brief A listener of hardware potentiometer events.
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <math.h>
#include "core_pins.h"
#include "compat.h"
#include "settings.h"
#include "ChangeAccumulator.h"
#include "PotBroadcaster.h"
#include "PotEvent.h"

/**
 * @brief provides callbacks for handling Pot events.
 */
class PotListener
{
public:
    /**
    * Constructor.
    */
    PotListener();

    /**
     * Destructor
     */
    virtual ~PotListener() = default;

    /**
     * Called when the potentiometer is turned.
     *
     * @param details about the pot activity, including information
     *        about the component assigned to the pot
     */
    virtual void onPotChange(const PotEvent &potEvent)
    {
    }

    /**
     * Called when the potentiometer is touched with a finger.
     *
     * @param details about the pot activity, including information
     *        about the component assigned to the pot
     */
    virtual void onPotTouchDown(const PotEvent &potEvent)
    {
    }

    /**
     * Called when a finger is lifted of the potentiometer.
     *
     * @param details about the pot activity, including information
     *        about the component assigned to the pot
     */
    virtual void onPotTouchUp(const PotEvent &potEvent)
    {
    }

    /**
     * Stop further propagation of current event
     *
     */
    void stopPropagation(void);

    /**
     * Assigns the pot to the listener object.
     *
     * @param an identifier of the pot. It is a number in range 0 to 11, or
     *        AllPots constant for catching events from all pots.
	 * @param a number of distict values within the pot range.
     */
    void assignPot(uint8_t newPotId = AllPots, uint16_t newNumValues = 0);

    /**
     * Assigns all pots to the listener object.
	 *
	 * @param a number of distict values within the pot range.
     */
    void assignAllPots(uint16_t newNumValues = 0);

    /**
     * Enable an encoder mode.
	 *
	 * @param true enables encoder style value reading, false keep the analog
     * pot reading.
     */
    void enableEncoderMode(bool newEncoderMode);

    /**
     * Stops listening for assigned pots.
     */
    void releasePot(void);

    /**
	 * Returns currently assigned pot.
	 */
    uint8_t getPotId(void);

    /**
     * @brief Whether events from this pot are this listener's to handle.
     *
     * The one assigned pot, or every pot. A listener driven by several knobs
     * answers for itself: that is a virtual call rather than a set of pots
     * kept here because only a Custom control is, and every Component on the
     * screen is one of these.
     */
    virtual bool listensToPot(uint8_t candidatePotId) const
    {
        return ((potId == AllPots) || (potId == candidatePotId));
    }

    /**
     * Set number of values to be associated with the pot
     *
     * @param a number of values to set the sensitivity
     */
    void setNumValues(uint16_t newNumValues);

    /**
	 * Returns number of distict values that the listener uses.
	 */
    uint16_t getNumValues(void);

    /**
     * @brief Turn rotation of the knob into a change of the value it drives.
     *
     * @param relativeChange rotation since the last report, in phase counts
     * @param turnedPotId the knob that turned
     */
    int16_t computeRate(int16_t relativeChange, uint8_t turnedPotId);

    void resetStepCount(void);

    static constexpr uint8_t AllPots = 255;
    static PotBroadcaster potBroadcaster;

protected:
    /**
     * @brief How many distinct values the knob's own value holds.
     *
     * The scaling of a turn is a property of what the knob drives, and what a
     * component drives can change while the knob is being held - an envelope
     * hands its knob to whichever segment is in focus, and a Custom control
     * has a value per knob. The default is the one range the listener was
     * assigned with.
     */
    virtual uint16_t numValuesForPot([[maybe_unused]] uint8_t turnedPotId) const
    {
        return (numValues);
    }

    /**
     * @brief The factor on the usual rate for turns of this knob.
     *
     * What the value the knob drives declares - see
     * ControlValue::setSensitivity().
     */
    virtual float sensitivityForPot(
        [[maybe_unused]] uint8_t turnedPotId) const
    {
        return (1.0f);
    }

private:
    /* Rotation banked towards the next report, in phase counts. Declared first
     * on purpose: it wants four byte alignment, and the three small members
     * below then fit in the padding the vtable pointer and it leave between
     * them. Every Component on the screen is one of these, so the four bytes
     * that ordering saves are four bytes a Component. */
    ChangeAccumulator<POT_SWEEP_COUNTS, POT_ENCODER_DETENT_COUNTS> rate;

    uint16_t numValues;
    uint8_t potId;
    bool encoderMode : 1;

    /* Which knob the banked rotation above belongs to, so that a control
     * driven by several does not spend one knob's remainder on the next. In
     * the byte encoderMode already took: one bank and its owner rather than a
     * bank per knob, on every Component on the screen. */
    uint8_t ratePot : 7;
};
