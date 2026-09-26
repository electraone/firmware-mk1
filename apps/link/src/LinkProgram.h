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
 * @file LinkProgram.h
 *
 * @brief Being a satellite: everything that is true of one wherever it runs.
 *
 * The program says hello until somebody answers, draws the view it is sent,
 * moves the controls its knobs are under, opens the choosers on the panel's
 * own buttons, and says on the screen what the link is doing. It is the
 * mk2's SatelliteProgram with the machine underneath swapped: one thread
 * instead of several, so nothing is queued between them, and an inbox in
 * main memory instead of the SysEx pool.
 *
 * Every call arrives on the main loop, which is the only thread there is.
 */

#pragma once

#include "LinkGestureSink.h"
#include "LinkNavigation.h"
#include "LinkPageView.h"
#include "LinkSurface.h"
#include "LinkTransport.h"
#include "LinkDelegate.h"
#include "src/Link/LinkClient.h"

class LinkWindow;

class LinkProgram : public LinkGestureSink
{
public:
    explicit LinkProgram(LinkTransport &newTransport);
    ~LinkProgram() override = default;

    /** @brief Take up residence in a window. */
    void attach(LinkWindow &newWindow);

    /**
     * @brief One pass: take what arrived, offer a session if there is none,
     *        draw a view if one has arrived, and keep the status line true.
     */
    void process(uint32_t nowMs);

    /**
     * @brief The part of a pass that keeps the session alive, and nothing
     *        that draws: for the moments the main loop is busy elsewhere.
     */
    void keepAlive(uint32_t nowMs);

    /** @brief A knob was turned, in display steps. */
    void turn(uint8_t potId, int16_t delta);

    /** @brief A finger arrived on a knob, or left it. */
    void potTouch(uint8_t potId, bool down);

    /** @brief How many values the control under a knob holds. */
    uint16_t valuesOnPot(uint8_t potId) const;

    /** @brief Whether the control under a knob steps rather than sweeps. */
    bool isDiscreteOnPot(uint8_t potId) const;

    void buttonDown(uint8_t buttonId);
    void buttonUp(uint8_t buttonId);

    /** @brief A touch on a custom control, raw. */
    void touchGesture(uint16_t elementId, const TouchEvent &touchEvent) override;

    LinkClient &getLink(void);
    const LinkClient &getLink(void) const;

    /** @brief What this unit calls itself in a hello. */
    void setName(const char *newName);

    /** @brief Which panel the primary is laying out for. */
    static Link::HelloRecord composeIdentity(const char *name);

private:
    /** @brief Hand every message in the inbox to the client. */
    void drainInbox(void);

    /** @brief Offer a session, until one is accepted. */
    void offerSession(uint32_t nowMs);

    /** @brief Draw the view when a new one has finished arriving. */
    void refreshView(void);

    /** @brief Say on the screen what the link is doing. */
    void reportStatus(void);

    /** @brief And what it has actually done, in numbers. */
    void reportDetail(void);

    /**
     * @brief A value moved here first: a finger on the screen.
     *
     * The controller's components apply a touch to their own value before
     * anything else hears of it. On a satellite the value is the primary's,
     * so what moved here is sent there, and the two agree again a message
     * later.
     */
    void forwardLocalChange(LookupEntry *entry);

    LinkTransport &transport;
    LinkClient link;
    LinkDelegate delegate;
    LinkPageView view;
    LinkNavigation navigation;
    LinkSurface surface;
    LinkWindow *window;

    char name[24];
    uint16_t drawnViewId;
    uint32_t lastHelloAt;
    char reported[64];
    char detailed[64];
    uint16_t sessions;
    bool wasConnected;
    uint16_t hellos;

    /** The pot under a finger, per element, so the highlight can be taken
     *  off the right control when the finger leaves. */
    uint16_t touchedElement[12];
};
