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
 * @file LinkSurface.h
 *
 * @brief This unit's knobs, driving the primary's controls.
 *
 * A knob here moves a control there. Which control is not guessed: every
 * element of the view says which pot drives it, and among several on one pot
 * - a page has three rows and twelve knobs - the one in the active control
 * set wins, which is what the primary marked it for.
 *
 * The knobs are relative, so what is sent is the value the primary last said
 * plus the turn. There is nothing to catch up with and nothing can jump; a
 * knob turned while the link is down does nothing, which is the honest
 * outcome. A custom control's knob is different: its turn travels raw as a
 * gesture, because what a knob does to one is its script's business and the
 * script runs on the primary.
 *
 * The same rules the mk2's satellite applies, in the same order.
 */

#pragma once

#include "src/Link/LinkClient.h"

class LinkSurface
{
public:
    explicit LinkSurface(LinkClient &newLink);

    /** @brief A knob was turned, in display steps. True when it moved
     *  something. */
    bool turn(uint8_t potId, int16_t delta);

    /**
     * @brief How many distinct values the control under a knob holds.
     *
     * Zero for a knob with nothing under it, or a discrete control, which
     * leaves the knob stepping a detent at a time.
     */
    uint16_t valuesOnPot(uint8_t potId) const;

    /** @brief Whether the control under a knob is stepped rather than swept:
     *  a list or a pad. */
    bool isDiscreteOnPot(uint8_t potId) const;

    /** @brief A finger arrived on a knob, or left it. Only a custom control
     *  is told. */
    bool potTouch(uint8_t potId, bool down);

    /** @brief The element on that pot, or nullptr. */
    const Link::ViewElement *elementOnPot(uint8_t potId) const;

    /** @brief Whether an element is a custom control. */
    static bool isCustom(const Link::ViewElement &element);

private:
    LinkClient &link;
};
