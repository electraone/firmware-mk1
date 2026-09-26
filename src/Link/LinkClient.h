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
 * @file LinkClient.h
 *
 * @brief The satellite's half of the Electra Satellite Link.
 *
 * Says hello, holds the view it is sent, and reports what the user did. It
 * decides nothing: a page the user picks is a request, and what is drawn does
 * not change until the primary sends the view for it.
 *
 * The records are read and written with the firmware's own codec, compiled
 * from src/Link, so the two ends cannot disagree about the format.
 */

#pragma once

#include "src/Link/LinkCodec.h"
#include "src/Link/LinkTransport.h"
#include "src/Link/LinkView.h"
#include <functional>
#include <string>
#include <vector>

class LinkClient
{
public:
    LinkClient();

    /** @brief Where log lines go. */
    using Logger = std::function<void(bool incoming, const std::string &)>;

    void setLogger(Logger newLogger);

    /**
     * @brief Called when a value the primary owns has moved.
     *
     * So that the drawing can flash the element without the caller having to
     * compare views.
     */
    using ValueChanged =
        std::function<void(Link::ElementId elementId, uint8_t handle)>;

    void setValueChanged(ValueChanged callback);

    /**
     * @brief Called when the primary's preset list has changed.
     *
     * Which presets are running, which are pinned and which one is on the
     * screen all change while a satellite is attached - somebody pins one on
     * the controller - and a chooser showing the old answer is showing
     * something that is no longer true. The list arrives one preset per
     * message, so this fires when the last of them lands and not before.
     */
    using PresetsChanged = std::function<void(void)>;

    void setPresetsChanged(PresetsChanged callback);

    /**
     * @brief Called when a custom control's captured paint has landed whole.
     *
     * The frame itself is in the shown view's store by the time this fires;
     * the callback is the poke to draw it. Fires only for the view being
     * shown - a frame for a view still assembling waits in that view and is
     * drawn with it.
     */
    using CustomFrameChanged = std::function<void(Link::ElementId elementId)>;

    void setCustomFrameChanged(CustomFrameChanged callback);

    /**
     * @brief Describe this satellite, and offer the session.
     *
     * The caller composes the record, because what a satellite is - its
     * screen, its knobs, the element types it can draw - is a fact about the
     * program holding this client and not about the link. The protocol
     * version is filled in here, being the one field this file owns.
     */
    void sayHello(const Link::HelloRecord &identity);

    /**
     * @brief Where this client's messages go.
     *
     * Set once, before saying hello. A client with no transport composes
     * nothing and sends nothing rather than faulting, because the program
     * around it may open its port after building it.
     */
    void setTransport(Link::Transport &newTransport);

    /**
     * @brief Act on one message from the primary.
     *
     * Taken already unwrapped - opcode and payload - rather than as raw
     * SysEx, because each program that holds a client already has code that
     * unwraps an Electra message and there is no reason to have a second.
     */
    void handleMessage(uint8_t opcode, const uint8_t *payload, size_t length);

    /** @brief Ask to be shown a page. The primary decides. */
    void requestPage(uint8_t pageNumber);

    /**
     * @brief Ask for the knobs to drive a row of the page: a control set,
     *  counted from one, or zero for whatever the primary would choose.
     */
    void requestControlSet(uint8_t controlSetId);

    /**
     * @brief Report that one of the context buttons was pressed.
     *
     * Which button, and nothing else: what it does belongs to the controller,
     * which is also the only end that can do it.
     */
    void pressContextButton(uint8_t index);

    /** @brief What the context buttons are called, in order. */
    const std::vector<std::string> &getContextButtons(void) const;

    /** @brief Ask to be shown one of the primary's live presets. */
    void requestPreset(uint8_t presetId);

    /** @brief Ask the controller to load a preset that is not running. */
    void requestPresetSwitch(uint8_t presetId);

    /** @brief Ask for a preset to be kept running, or to stop being. */
    void requestPin(uint8_t presetId, bool shouldPin);

    /** @brief Ask for everything again. */
    void requestResync();

    /** @brief Ask to be shown the controls this satellite subscribed to. */
    void requestMasterPerformance();

    /**
     * @brief Tell the controller which controls to gather.
     *
     * Sent in pieces, because a full set does not fit one SysEx message. The
     * order is the addressing: an element of a master view carries its place
     * in this list as its id.
     */
    void sendSubscription(const std::vector<Link::SubscriptionEntry> &entries);

    /** @brief What the view being shown is: a page, or the assignments. */
    bool isMasterView(void) const;

    /**
     * @brief Say the session is over.
     *
     * Worth saying rather than just closing the port: while a satellite is
     * attached the controller keeps the parameter map of whatever is being
     * watched switched on, and a satellite that vanishes without a word would
     * leave that costing something for nothing.
     */
    void sayGoodbye();

    /** @brief Report that the user moved a value to somewhere. */
    void sendValue(Link::ElementId elementId, uint8_t handle, int32_t value);

    bool isConnected(void) const;

    /**
     * @brief Notice a primary that has gone away.
     *
     * A session has no other end to it: a primary that reboots, is unplugged
     * or simply stops does not say goodbye, and a satellite that took silence
     * for a working link would sit showing a page nobody is updating - which
     * is worse than showing nothing, because it looks right.
     *
     * So the link is pinged while it is up, and anything at all arriving from
     * the primary counts as it being there. When nothing has for long enough
     * the session is dropped, which is what lets the program around this
     * offer a new one.
     *
     * The clock is passed in rather than read: this file is compiled for a
     * controller and for a computer, and neither one's clock belongs here.
     *
     * @param nowMs a millisecond clock, monotonic within a run
     */
    void keepAlive(uint32_t nowMs);

    /** @brief How many messages the primary has sent, since boot. */
    uint32_t getMessagesReceived(void) const;

    /** @brief Pings sent with nothing heard back since. */
    uint8_t getUnanswered(void) const;

    /** @brief The view being shown, which may still be assembling. */
    const Link::View &getView(void) const;

    /** @brief The preset banks, named and coloured by the owner. */
    const std::vector<Link::PresetBankInfo> &getBanks(void) const;

    /** @brief The primary's live presets, as it last described them. */
    const std::vector<Link::LivePreset> &getPresets(void) const;

    /** @brief The controls being gathered, by their place in the subscription. */
    const std::vector<Link::SubscriptionValue> &
        getSubscriptionValues(void) const;

    /** @brief Move a subscribed control, wherever its preset is running. */
    void sendSubscriptionValue(uint16_t index, uint8_t handle, int32_t value);

    /**
     * @brief Report a switch going down or coming up.
     *
     * Both are sent, and neither says what the control should become: whether
     * it latches or follows the finger is the primary's to decide.
     */
    void sendButton(Link::ElementId elementId,
                    uint8_t handle,
                    bool pressed,
                    bool viaSubscription);

    /**
     * @brief Report a gesture made on a custom control, raw.
     *
     * A custom control's input, like its look, belongs to its preset's
     * script - so the touch, the turn or the press travels as itself and the
     * primary replays it into the Lua callbacks. Stamped with the view being
     * shown, and sent only to a primary whose welcome said version 7: an
     * older one does not know the opcode, and would log every finger stroke
     * as an error.
     *
     * Fill only the fields the kind uses; see Link::GestureKind.
     */
    void sendGesture(const Link::GestureRecord &gesture);

    /**
     * @brief Whether the whole preset list has arrived.
     *
     * It comes one preset per message, so acting on a half-read list means
     * acting on a wrong one - and resolving assignments against it asked the
     * controller to gather the wrong things and then asked again.
     */
    bool arePresetsComplete(void) const;

    const std::string &getPrimaryName(void) const;

    /** @brief What the preset being shown calls itself, across controllers. */
    const std::string &getProjectId(void) const;

private:
    void handleWelcome(Link::Reader &reader);
    void handleView(Link::Reader &reader);
    void handleElement(Link::Reader &reader);
    void handlePrimitives(Link::Reader &reader);
    void handleOverlay(Link::Reader &reader);
    void handleOverlayBitmap(Link::Reader &reader);
    void handleBanks(Link::Reader &reader);
    void handleContextButtons(Link::Reader &reader);
    void handleUpdate(Link::Reader &reader);
    void handlePresets(Link::Reader &reader);

    void sendRequest(Link::RequestKind kind,
                     uint8_t argument,
                     uint8_t argument2,
                     const std::string &note);
    void sendInput(Link::InputKind kind,
                   Link::ElementId elementId,
                   uint8_t handle,
                   int16_t a,
                   bool viaSubscription);

    void send(Link::Opcode opcode, const uint8_t *payload, size_t length);

    void log(bool incoming, const std::string &text);

    /*
     * Every whole overlay this session has been sent, kept across views: a
     * version-6 primary skips overlays it knows are held here, so forgetting
     * one would lose a list's words with no way to ask for them back. Grows
     * with the distinct overlays seen in one session and is cleared by the
     * next Welcome.
     */
    struct StoredOverlay {
        uint8_t presetId = 0;
        Link::ViewOverlay overlay;
    };

    std::vector<StoredOverlay> overlayStore;

    Link::Transport *transport;
    bool connected;

    /** When the last ping went out. */
    uint32_t lastPingAt;

    /** Set by any message from the primary, cleared by each ping. */
    bool heard;

    /**
     * When the view now assembling was first seen assembling, by the caller's
     * clock, or zero. A view whose elements stop arriving would otherwise be
     * assembled forever: nothing is drawn, every update names a view that is
     * not the one shown and is dropped, and the screen sits on the last
     * page as if nothing were wrong. Delivery is not acknowledged, so this
     * is where a lost element is noticed - and asked for again.
     */
    uint32_t assemblingSince;

    /** When a resync was last asked for, so a broken view asks once. */
    uint32_t lastResyncAt;

    /** Pings sent since the primary was last heard from. */
    uint8_t unanswered;

    /** Everything the primary has ever sent. On the satellite's screen, so
     *  that a link which looks alive can be told from one that is. */
    uint32_t messagesReceived;
    std::string primaryName;
    std::string projectId;
    uint16_t sessionId;

    /** What the primary's welcome said it speaks; gates the gestures. */
    uint8_t primaryVersion = 0;

    /*
     * Two views: the one being drawn, and the one being assembled. A view
     * arrives as a header and then one message per element, and swapping only
     * when the last element lands is what stops a page being seen to build
     * itself a control at a time.
     */
    Link::View shown;
    Link::View incoming;
    bool assembling;
    Link::ViewKind shownKind;

    std::vector<Link::LivePreset> presets;
    std::vector<Link::PresetBankInfo> banks;
    std::vector<std::string> contextButtons;

    /** Fed by the controller for every subscribed control, view or no view. */
    std::vector<Link::SubscriptionValue> subscriptionValues;

    void handleSubscriptionUpdate(Link::Reader &reader);
    bool presetsComplete;

    /*
     * The frame being reassembled. A frame larger than one message arrives
     * in pieces, and the primary sends one frame's pieces back to back, so
     * one buffer is enough - a new frame's sequence zero simply starts it
     * over.
     */
    std::vector<uint8_t> pendingFrame;
    uint16_t pendingFrameView = 0;
    Link::ElementId pendingFrameElement = 0;
    uint8_t pendingFrameSequence = 0;

    Logger logger;
    ValueChanged valueChanged;
    PresetsChanged presetsChanged;
    CustomFrameChanged customFrameChanged;
};
