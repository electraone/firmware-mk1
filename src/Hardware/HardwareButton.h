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
 * @file HardwareButton.h
 *
 * @brief A representation of a hardware Button.
 */

#pragma once

#include <functional>
#include <cstdint>

/**
 * Button class
 *
 * a representation of the hardware button.
 */
class HardwareButton
{
public:
    /**
     * List of possible hardware button events
     */
    enum Event {
        none, ///< no event
        press, ///< button has been pressed
        hold, ///< button has been held pressed for longer period
        release ///< button has been released
    };

    /**
     * A hardware button constructor
     *
     * @param id An numeric identifier of the button. The id is given by the
     *        hardware design.
     */
    HardwareButton(uint8_t id);

    /**
     * Test state of the button
     *
     * Evaluates the state of the button. If there is any change to the previous
     * state, the function will indicate it with the return value.
     *
     * @return true, if there is any state change.
     */
    Event process(void);

    /**
     * Test state of the button where the multiplexer already is
     *
     * The same as process(), for a caller that has already selected this
     * button's address and wants it left selected - the pot scan, which parks
     * the multiplexer on one address for a whole tick and reads the button
     * sharing it on the way past.
     *
     * @return the event, if there is any state change.
     */
    Event sample(void);

    /**
     * Read whether the button is down right now, without reporting an event
     *
     * For the boot, which asks once whether a button is held before anything
     * scans. process() cannot answer that any more: it believes a level only
     * once it has held for debouncePeriod, which a single call never sees. The
     * level read is taken as settled, so the scan that follows does not report
     * the same press again.
     *
     * @return true, when the button is down.
     */
    bool read(void);

    /**
     * Get id of the button
     *
     * @return Identifier that was assigned when the button was created.
     */
    uint8_t getId(void);

    /**
     * Provide information if the button is pressed
     *
     * @return true, when button is currently pressed.
     */
    bool isPressed(void);

    /**
     * Provide information is the button is pressed for longer time period
     *
     * @return true, when pressed for longer time period.
     */
    bool isLongHold(void);

    /**
     * onPress event handler
     *
     * Callback function to handle event when the button is pressed down.
     */
    std::function<void(void)> onPress;

    /**
     * onLongHold event handler
     *
     * Callback function to handle event when the button is pressed for
     * longer time period.
     */
    std::function<void(void)> onLongHold;

    /**
     * onRelease event handler
     *
     * Callback function to handle event when the button is released.
     */
    std::function<void(void)> onRelease;

private:
    uint8_t id;
    bool pressed;
    uint8_t pinState;
    uint8_t candidateState;
    uint32_t candidateSince;
    bool longHold;
    uint32_t pressStart;

    /* How long a raw level has to hold before it is believed, in milliseconds.
     * The buttons used to rely on the scan's own pace for this: each was read
     * once every ninety milliseconds, so contact bounce had always finished by
     * the next read and a single raw sample could be trusted. They are now read
     * by the pot scan, every twelve to twenty-four milliseconds, and debouncing
     * against the clock keeps them independent of whatever pace that is. The
     * mk2's figure. */
    static constexpr uint32_t debouncePeriod = 6;

    static constexpr uint16_t longHoldPeriod = 1000;
};
