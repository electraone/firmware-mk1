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

#include "LinkProgram.h"
#include "LinkWindow.h"
#include "LinkInbox.h"
#include "ParameterMap.h"
#include "System.h"
#include "helpers.h"
#include <cstdio>
#include <cstring>

#if defined(LINK_BENCH) && LINK_BENCH
volatile uint32_t linkBenchInbox = 0;
volatile uint8_t linkBenchLastOpcode = 0;
volatile uint32_t linkBenchValueChanged = 0;
volatile uint32_t linkBenchValueStale = 0;
#endif

LinkProgram::LinkProgram(LinkTransport &newTransport)
    : transport(newTransport),
      link(),
      delegate(),
      view(delegate, *this),
      navigation(link),
      surface(link),
      window(nullptr),
      name{},
      drawnViewId(0),
      lastHelloAt(0),
      reported{},
      detailed{},
      sessions(0),
      wasConnected(false),
      hellos(0),
      touchedElement{}
{
    link.setTransport(transport);

    for (uint8_t i = 0; i < 12; i++) {
        touchedElement[i] = Link::NoElement;
    }

    /* One value moved on the primary. Applied to the one control, on the
     * view being drawn; a value for a view still assembling waits in it. */
    link.setValueChanged([this](Link::ElementId elementId, uint8_t handle) {
        const Link::View &shown = link.getView();

#if defined(LINK_BENCH) && LINK_BENCH
        linkBenchValueChanged++;
#endif

        if (shown.getViewId() != drawnViewId) {
#if defined(LINK_BENCH) && LINK_BENCH
            linkBenchValueStale++;
#endif
            return;
        }

        const Link::ViewElement *element = shown.findElement(elementId);

        if (element == nullptr) {
            return;
        }

        for (const Link::ViewValue &value : element->values) {
            if (value.handle == handle) {
                view.updateValue(
                    elementId,
                    handle,
                    (int16_t)value.current,
                    value.label.empty() ? nullptr : value.label.c_str());
                return;
            }
        }
    });

    /* A custom control's captured paint landed. */
    link.setCustomFrameChanged([this](Link::ElementId elementId) {
        const Link::View &shown = link.getView();

        if (shown.getViewId() != drawnViewId) {
            return;
        }

        auto entry = shown.getCustomFrames().find(elementId);

        if (entry != shown.getCustomFrames().end()) {
            view.applyCustomFrame(
                elementId, entry->second.data(), entry->second.size());
        }
    });

    /* Somebody pinned a preset on the primary, or one stopped running. */
    link.setPresetsChanged([this]() { navigation.refresh(); });

    /* What the controller's components do to a value when a finger moves
     * them on the screen. See forwardLocalChange(). */
    parameterMap.onChange = [this](LookupEntry *entry, Origin origin) {
        if ((entry != nullptr) && (origin == Origin::internal)) {
            forwardLocalChange(entry);
        }
    };
}

void LinkProgram::setName(const char *newName)
{
    copyString(name, (newName != nullptr) ? newName : "", sizeof(name) - 1);
}

void LinkProgram::attach(LinkWindow &newWindow)
{
    window = &newWindow;
}

LinkClient &LinkProgram::getLink(void)
{
    return (link);
}

const LinkClient &LinkProgram::getLink(void) const
{
    return (link);
}

/**
 * @brief Describe this machine to a primary.
 *
 * The screen size and the knob and button counts are the panel's own. The
 * primary lays out for exactly what is claimed here and the satellite never
 * scales or reflows.
 */
Link::HelloRecord LinkProgram::composeIdentity(const char *name)
{
    Link::HelloRecord identity = {};

    identity.model = Link::Model::Mk1;
    identity.screenWidth = 1024;
    identity.screenHeight = 560;
    identity.colourDepth = 16;
    identity.potCount = NR_OF_HW_POTS;
    identity.buttonCount = NR_OF_HW_BUTTONS;
    identity.flags = Link::Capability::TouchScreen;

    /* Every element type, because the drawing is the controller's own
     * components rather than a lookalike of them. */
    identity.elementTypes = Link::allElementTypes;

    copyString(identity.name.text,
               (name != nullptr) ? name : "",
               sizeof(identity.name.text) - 1);

    return (identity);
}

void LinkProgram::process(uint32_t nowMs)
{
    drainInbox();
    link.keepAlive(nowMs);
    offerSession(nowMs);
    refreshView();
    reportStatus();
    reportDetail();
}

/*
 * Messages are taken in and answered, so the primary keeps hearing this
 * unit, and the view they may have brought waits for the next full pass:
 * applying it means building components, and the main loop is inside
 * something that must not be interrupted by a repaint.
 */
void LinkProgram::keepAlive(uint32_t nowMs)
{
    drainInbox();
    link.keepAlive(nowMs);
    offerSession(nowMs);
}

/*
 * Everything the interrupt banked since the last pass. The payload is decoded
 * here rather than in the interrupt, where it would cost the same time at a
 * higher priority for no gain.
 */
void LinkProgram::drainInbox(void)
{
    const LinkInbox::Message *message = nullptr;

    while ((message = LinkInbox::peek()) != nullptr) {
        if (message->length >= 1) {
            uint8_t payload[Link::maxPayloadLength];
            const uint8_t opcode = message->bytes[0];
            const int length =
                Link::decodePayload(payload,
                                    sizeof(payload),
                                    (const char *)(message->bytes + 1),
                                    message->length - 1);

#if defined(LINK_BENCH) && LINK_BENCH
            linkBenchInbox++;
            linkBenchLastOpcode = opcode;
#endif

            if (length >= 0) {
                link.handleMessage(opcode, payload, (size_t)length);
            } else {
                System::logger.write(LOG_ERROR,
                                     "LinkProgram: opcode %02x does not "
                                     "decode",
                                     opcode);
            }
        }

        LinkInbox::release();
    }
}

void LinkProgram::offerSession(uint32_t nowMs)
{
    constexpr uint32_t helloIntervalMs = Link::pingIntervalMs;

    if (link.isConnected()) {
        return;
    }

    if ((lastHelloAt != 0) && ((nowMs - lastHelloAt) < helloIntervalMs)) {
        return;
    }

    lastHelloAt = nowMs;
    hellos++;
    link.sayHello(composeIdentity(name));
}

void LinkProgram::refreshView(void)
{
    const Link::View &shown = link.getView();

    if ((window == nullptr) || !link.isConnected() || !shown.isComplete()
        || shown.isEmpty()) {
        return;
    }

    if (shown.getViewId() == drawnViewId) {
        return;
    }

    drawnViewId = shown.getViewId();

    const bool rebuilt = view.rebuild(shown, window->getContent());

    window->getContent().setShowingView(true);
    window->getContent().configureKnobs();

    /* A new view means a new page, and possibly a new preset: whatever a
     * chooser is showing about either is now out of date. */
    navigation.refresh();
    view.repaintBottomBar();

    System::logger.write(LOG_INFO,
                         "LinkProgram: %s view %d, %d controls",
                         rebuilt ? "drew" : "refreshed in place",
                         (int)drawnViewId,
                         (int)view.getCount());
}

void LinkProgram::turn(uint8_t potId, int16_t delta)
{
    surface.turn(potId, delta);
}

void LinkProgram::potTouch(uint8_t potId, bool down)
{
    if (potId >= 12) {
        return;
    }

    /* The line under the control a finger is on, as the controller draws
     * it. Taken off the control that was touched rather than the one under
     * the knob now, in case the view changed in between. */
    if (down) {
        const Link::ViewElement *element = surface.elementOnPot(potId);

        touchedElement[potId] =
            (element != nullptr) ? element->id : Link::NoElement;

        if (element != nullptr) {
            view.highlightElement(element->id, true);
        }
    } else if (touchedElement[potId] != Link::NoElement) {
        view.highlightElement(touchedElement[potId], false);
        touchedElement[potId] = Link::NoElement;
    }

    surface.potTouch(potId, down);
}

uint16_t LinkProgram::valuesOnPot(uint8_t potId) const
{
    return (surface.valuesOnPot(potId));
}

bool LinkProgram::isDiscreteOnPot(uint8_t potId) const
{
    return (surface.isDiscreteOnPot(potId));
}

void LinkProgram::buttonDown(uint8_t buttonId)
{
    navigation.handleButtonDown(buttonId);
}

void LinkProgram::buttonUp(uint8_t buttonId)
{
    navigation.handleButtonUp(buttonId);
}

void LinkProgram::touchGesture(uint16_t elementId, const TouchEvent &touchEvent)
{
    Link::GestureRecord record = {};

    record.elementId = elementId;
    record.kind = (uint8_t)Link::GestureKind::Touch;
    record.event = (uint8_t)touchEvent.getEvent();
    record.touchId = touchEvent.getId();
    record.x = (int16_t)touchEvent.getX();
    record.y = (int16_t)touchEvent.getY();
    record.downX = (int16_t)touchEvent.getTouchDownX();
    record.downY = (int16_t)touchEvent.getTouchDownY();

    link.sendGesture(record);
}

void LinkProgram::forwardLocalChange(LookupEntry *entry)
{
    if (!link.isConnected()) {
        return;
    }

    for (ControlValue *value : entry->getDestinations()) {
        const Control *control = value->getControl();

        if (control == nullptr) {
            continue;
        }

        const Link::ViewElement *element =
            link.getView().findElement(control->getId());

        if (element == nullptr) {
            continue;
        }

        link.sendValue(control->getId(),
                       (uint8_t)value->getHandle(),
                       value->translateMidiValue(entry->getMidiValue()));
    }
}

void LinkProgram::reportStatus(void)
{
    char status[sizeof(reported)];
    const bool connected = link.isConnected();

    if (connected && !wasConnected) {
        sessions++;
    }

    if (!connected && wasConnected) {
        /* The primary is gone. Whatever is on the screen is a page nobody is
         * updating, and the choosers offer things that cannot be asked for. */
        navigation.closeAll();

        if (window != nullptr) {
            view.clear();
            window->getContent().setShowingView(false);
        }

        drawnViewId = 0;
    }

    wasConnected = connected;

    if (!connected) {
        copyString(status, "waiting for a primary", sizeof(status) - 1);
    } else {
        const Link::View &shown = link.getView();

        if (shown.isEmpty()) {
            snprintf(status,
                     sizeof(status),
                     "linked to %s - session %d",
                     link.getPrimaryName().c_str(),
                     (int)sessions);
        } else {
            snprintf(status,
                     sizeof(status),
                     "preset %d page %d, %d elements - session %d",
                     (int)shown.getPresetId(),
                     (int)shown.getPageNumber(),
                     (int)shown.getElements().size(),
                     (int)sessions);
        }
    }

    if ((window == nullptr) || (strcmp(status, reported) == 0)) {
        return;
    }

    copyString(reported, status, sizeof(reported) - 1);
    window->setStatus(status);
}

void LinkProgram::reportDetail(void)
{
    char detail[64];

    snprintf(detail,
             sizeof(detail),
             "rx %d - tx %d - missed %d - hellos %d - dropped %d",
             (int)link.getMessagesReceived(),
             (int)transport.getMessagesSent(),
             (int)link.getUnanswered(),
             (int)hellos,
             (int)(LinkInbox::getDropped() + LinkInbox::getRefused()));

    if ((window == nullptr) || (strcmp(detail, detailed) == 0)) {
        return;
    }

    copyString(detailed, detail, sizeof(detailed) - 1);
    window->setDetail(detail);
}
