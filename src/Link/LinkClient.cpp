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

#include "src/Link/LinkClient.h"
#include <algorithm>
#include <cstring>

LinkClient::LinkClient()
    : transport(nullptr),
      connected(false),
      lastPingAt(0),
      heard(false),
      assemblingSince(0),
      lastResyncAt(0),
      unanswered(0),
      messagesReceived(0),
      sessionId(0),
      assembling(false),
      shownKind(Link::ViewKind::PresetPage),
      presetsComplete(false)
{
}

void LinkClient::setTransport(Link::Transport &newTransport)
{
    transport = &newTransport;
}

void LinkClient::setLogger(Logger newLogger)
{
    logger = newLogger;
}

void LinkClient::setValueChanged(ValueChanged callback)
{
    valueChanged = callback;
}

void LinkClient::setPresetsChanged(PresetsChanged callback)
{
    presetsChanged = callback;
}

void LinkClient::setCustomFrameChanged(CustomFrameChanged callback)
{
    customFrameChanged = callback;
}

void LinkClient::log(bool incoming_, const std::string &text)
{
    if (logger) {
        logger(incoming_, text);
    }
}

bool LinkClient::isConnected(void) const
{
    return (connected);
}

const Link::View &LinkClient::getView(void) const
{
    return (shown);
}

const std::vector<Link::LivePreset> &LinkClient::getPresets(void) const
{
    return (presets);
}

const std::string &LinkClient::getPrimaryName(void) const
{
    return (primaryName);
}

const std::string &LinkClient::getProjectId(void) const
{
    return (projectId);
}

void LinkClient::send(Link::Opcode opcode,
                      const uint8_t *payload,
                      size_t length)
{
    if (transport == nullptr) {
        return;
    }

    transport->sendLink((uint8_t)opcode, payload, length);
}

void LinkClient::sayHello(const Link::HelloRecord &identity)
{
    uint8_t payload[Link::maxPayloadLength];
    Link::Writer writer(payload, sizeof(payload));
    Link::HelloRecord hello = identity;

    hello.version = Link::protocolVersion;

    if (Link::write(writer, hello)) {
        send(Link::Opcode::Hello, payload, writer.getLength());
        log(false, "-> hello");
    }
}

uint32_t LinkClient::getMessagesReceived(void) const
{
    return (messagesReceived);
}

uint8_t LinkClient::getUnanswered(void) const
{
    return (unanswered);
}

/** How long a view may be assembling before it is asked for again. */
static constexpr uint32_t viewAssemblyTimeoutMs = 1500;

void LinkClient::keepAlive(uint32_t nowMs)
{
    if (!connected) {
        assemblingSince = 0;
        return;
    }

    /*
     * A view that has been assembling for too long is missing something. A
     * page's elements arrive back to back, so a second and a half of
     * incompleteness is a lost message, not a slow one - and the answer is
     * to ask for the view again, once, rather than wait for a knob to be
     * turned on a page that never comes.
     */
    if (!assembling) {
        assemblingSince = 0;
    } else if (assemblingSince == 0) {
        assemblingSince = nowMs;
    } else if (((nowMs - assemblingSince) > viewAssemblyTimeoutMs)
               && ((nowMs - lastResyncAt) > viewAssemblyTimeoutMs)) {
        lastResyncAt = nowMs;
        assemblingSince = nowMs;
        log(true, "<- view incomplete, asking again");
        requestResync();
    }

    if ((nowMs - lastPingAt) < Link::pingIntervalMs) {
        return;
    }

    lastPingAt = nowMs;

    if (heard) {
        heard = false;
        unanswered = 0;
    } else if (++unanswered >= Link::pingMissesAllowed) {
        /* Six seconds of silence. Long enough that a primary busy drawing a
         * page is not mistaken for one that has gone away. */
        connected = false;
        log(true, "<- primary stopped answering");
        return;
    }

    send(Link::Opcode::Ping, nullptr, 0);
}

void LinkClient::handleMessage(uint8_t opcodeByte,
                               const uint8_t *payload,
                               size_t length)
{
    Link::Opcode opcode = (Link::Opcode)opcodeByte;

    /* Anything at all means the primary is there. Recorded as a flag rather
     * than a time, so that reading a message needs no clock. */
    heard = true;
    messagesReceived++;
    Link::Reader reader(payload, length);

    switch (opcode) {
        case Link::Opcode::Welcome:
            handleWelcome(reader);
            break;

        case Link::Opcode::View:
            handleView(reader);
            break;

        case Link::Opcode::Element:
            handleElement(reader);
            break;

        case Link::Opcode::Primitives:
            handlePrimitives(reader);
            break;

        case Link::Opcode::Overlay:
            handleOverlay(reader);
            break;

        case Link::Opcode::OverlayBitmap:
            handleOverlayBitmap(reader);
            break;

        case Link::Opcode::Banks:
            handleBanks(reader);
            break;

        case Link::Opcode::ContextButtons:
            handleContextButtons(reader);
            break;

        case Link::Opcode::Update:
            handleUpdate(reader);
            break;

        case Link::Opcode::SubscriptionUpdate:
            handleSubscriptionUpdate(reader);
            break;

        case Link::Opcode::Presets:
            handlePresets(reader);
            break;

        case Link::Opcode::Pong:
            break;

        case Link::Opcode::Bye:
            connected = false;
            log(true, "<- bye");
            break;

        default:
            break;
    }
}

void LinkClient::handlePrimitives(Link::Reader &reader)
{
    Link::PrimitivesHeaderRecord header = {};

    if (!Link::read(reader, header)) {
        return;
    }

    if (header.count > reader.getRemaining()) {
        return;
    }

    if (header.sequence == 0) {
        pendingFrame.clear();
        pendingFrameView = header.viewId;
        pendingFrameElement = header.elementId;
    } else if ((header.viewId != pendingFrameView)
               || (header.elementId != pendingFrameElement)
               || (header.sequence != (uint8_t)(pendingFrameSequence + 1))) {
        /* A piece of a frame whose start was never seen, or one arriving out
         * of order. The frame is dropped whole - the primary re-captures on
         * the next change, and half a drawing is worse than the last one. */
        pendingFrame.clear();
        return;
    }

    pendingFrameSequence = header.sequence;

    const size_t at = pendingFrame.size();

    if ((at + header.count) > Link::maxPrimitiveBytes) {
        pendingFrame.clear();
        return;
    }

    pendingFrame.resize(at + header.count);

    if (!reader.readBytes(pendingFrame.data() + at, header.count)) {
        pendingFrame.clear();
        return;
    }

    if (header.final == 0) {
        return;
    }

    /* Whole. A frame for the view still assembling waits inside it and is
     * drawn when the view is; one for the view being shown is drawn now. A
     * frame for neither is stale and goes nowhere. */
    if (assembling && (header.viewId == incoming.getViewId())) {
        incoming.setCustomFrame(
            header.elementId, pendingFrame.data(), pendingFrame.size());
    } else if (header.viewId == shown.getViewId()) {
        shown.setCustomFrame(
            header.elementId, pendingFrame.data(), pendingFrame.size());

        if (customFrameChanged) {
            customFrameChanged(header.elementId);
        }
    }

    pendingFrame.clear();
}

void LinkClient::handleWelcome(Link::Reader &reader)
{
    Link::WelcomeRecord welcome = {};

    if (!Link::read(reader, welcome)) {
        return;
    }

    connected = true;
    sessionId = welcome.sessionId;
    primaryName = welcome.name.text;
    primaryVersion = welcome.version;
    heard = true;
    unanswered = 0;

    /* A new session's primary has sent this Electra Link nothing yet, whatever
     * the last session left behind. */
    overlayStore.clear();

    log(true,
        std::string("<- welcome from ") + welcome.name.text + ", protocol v"
            + std::to_string(welcome.version));
}

void LinkClient::handleView(Link::Reader &reader)
{
    Link::ViewHeaderRecord header = {};

    if (!Link::read(reader, header)) {
        return;
    }

    incoming.beginView(header.viewId,
                       header.presetId,
                       header.pageNumber,
                       header.activeSet,
                       header.title.text,
                       header.elementCount);

    for (uint8_t i = 0; i < header.pageCount; i++) {
        Link::PageRecord page = {};

        if (!Link::read(reader, page)) {
            break;
        }

        Link::ViewPage viewPage;

        viewPage.id = page.id;
        viewPage.name = page.name.text;
        incoming.addPage(viewPage);
    }

    /* Seed the view with every overlay this session already holds for its
     * preset: a version-6 primary skips overlays it knows are here, so what
     * it skips has to come from the store. One that does arrive replaces the
     * seeded copy through its sequence-0 restart. */
    for (const StoredOverlay &stored : overlayStore) {
        if (stored.presetId == header.presetId) {
            incoming.putOverlay(stored.overlay);
        }
    }

    assembling = true;
    shownKind = header.kind;
    projectId = header.projectId.text;

    log(true,
        std::string("<- view ") + std::to_string(header.viewId) + " \""
            + header.title.text + "\" page " + std::to_string(header.pageNumber)
            + ", " + std::to_string(header.elementCount) + " elements");

    /* A page with nothing on it is complete the moment its header lands. */
    if (incoming.isComplete()) {
        shown = incoming;
        assembling = false;
    }
}

void LinkClient::handleElement(Link::Reader &reader)
{
    uint16_t viewId = 0;

    if (!reader.readU16(viewId)) {
        return;
    }

    /* An element of a view that has already been replaced would otherwise be
     * drawn into the new one. */
    if (!assembling || (viewId != incoming.getViewId())) {
        return;
    }

    Link::ElementRecord record = {};

    if (!Link::read(reader, record)) {
        return;
    }

    Link::ViewElement element;

    element.id = record.id;
    element.presetId = record.presetId;
    element.type = record.type;
    element.nativeType = record.nativeType;
    element.nativeVariant = record.nativeVariant;
    element.nativeMode = record.nativeMode;
    element.visible = ((record.flags & Link::ElementFlag::Visible) != 0);
    element.active = ((record.flags & Link::ElementFlag::Active) != 0);
    element.bounds.x = (int16_t)record.x;
    element.bounds.y = (int16_t)record.y;
    element.bounds.width = (int16_t)record.width;
    element.bounds.height = (int16_t)record.height;
    element.colour = record.colour;
    element.potId = record.potId;
    element.name = record.name.text;

    for (uint8_t i = 0; i < record.valueCount; i++) {
        Link::ValueRecord value = {};

        if (!Link::read(reader, value)) {
            break;
        }

        Link::ViewValue viewValue;

        viewValue.handle = value.handle;
        viewValue.overlayId = value.overlayId;
        viewValue.min = value.min;
        viewValue.max = value.max;
        viewValue.current = value.current;
        viewValue.known = ((value.flags & Link::ValueFlag::Known) != 0);
        viewValue.isSwitch = ((value.flags & Link::ValueFlag::Switch) != 0);

        if ((value.flags & Link::ValueFlag::HasLabel) != 0) {
            viewValue.label = value.label.text;
        }

        element.values.push_back(viewValue);
    }

    /* The pots as a mask, when the primary is new enough to say - a custom
     * control may take several knobs. Trailing deliberately: a reader that
     * predates it stopped at the values, and this reader falls back to the
     * single potId when the bytes are not there. */
    if (reader.getRemaining() >= 2) {
        reader.readU16(element.potMask);
    }

    incoming.addElement(element);

    if (incoming.isComplete()) {
        shown = incoming;
        assembling = false;
    }
}

const std::vector<Link::PresetBankInfo> &LinkClient::getBanks(void) const
{
    return (banks);
}

const std::vector<std::string> &LinkClient::getContextButtons(void) const
{
    return (contextButtons);
}

void LinkClient::handleContextButtons(Link::Reader &reader)
{
    Link::ContextButtonRecord record = {};

    while (Link::read(reader, record)) {
        if (contextButtons.size() != record.count) {
            contextButtons.assign(record.count, std::string());
        }

        if (record.index < contextButtons.size()) {
            contextButtons[record.index] = record.label.text;
        }
    }
}

void LinkClient::handleBanks(Link::Reader &reader)
{
    Link::BankRecord record = {};

    while (Link::read(reader, record)) {
        if (banks.size() != record.count) {
            banks.assign(record.count, Link::PresetBankInfo());
        }

        if (record.index < banks.size()) {
            banks[record.index].colour = record.colour;
            banks[record.index].name = record.name.text;
        }
    }
}

void LinkClient::handleOverlay(Link::Reader &reader)
{
    Link::OverlayHeaderRecord header = {};

    if (!Link::read(reader, header)) {
        return;
    }

    std::vector<Link::OverlayItem> items;

    for (uint8_t i = 0; i < header.count; i++) {
        Link::OverlayItemRecord record = {};

        if (!Link::read(reader, record)) {
            break;
        }

        Link::OverlayItem item;

        item.value = record.value;
        item.colour = record.colour;
        item.colourIsSet = (record.colourIsSet != 0);
        item.label = record.label.text;

        items.push_back(item);
    }

    /* Onto the view being assembled: overlays arrive with a view, before the
     * elements that name them. */
    Link::View &target = assembling ? incoming : shown;

    target.addOverlayItems(header.id, header.sequence, items);

    /* Into the session store once whole, so the next view of this preset can
     * be seeded with it and the primary may skip sending it again. */
    if (header.final) {
        const Link::ViewOverlay *complete = nullptr;

        for (const Link::ViewOverlay &candidate : target.getOverlays()) {
            if (candidate.id == header.id) {
                complete = &candidate;
                break;
            }
        }

        if (complete != nullptr) {
            for (StoredOverlay &stored : overlayStore) {
                if ((stored.presetId == target.getPresetId())
                    && (stored.overlay.id == header.id)) {
                    stored.overlay = *complete;
                    return;
                }
            }

            overlayStore.push_back({ target.getPresetId(), *complete });
        }
    }
}

void LinkClient::handleUpdate(Link::Reader &reader)
{
    uint16_t viewId = 0;
    uint16_t changeCount = 0;

    if (!reader.readU16(viewId) || !reader.readU16(changeCount)) {
        return;
    }

    if (viewId != shown.getViewId()) {
        /* A change to a view we are not showing. Nothing to apply it to, and
         * the view we do have is about to be replaced anyway. */
        return;
    }

    for (uint16_t i = 0; i < changeCount; i++) {
        uint16_t elementId = 0;
        uint8_t kind = 0;

        if (!reader.readU16(elementId) || !reader.readU8(kind)) {
            return;
        }

        if ((Link::ChangeKind)kind != Link::ChangeKind::Value) {
            return;
        }

        Link::ValueRecord value = {};

        if (!Link::read(reader, value)) {
            return;
        }

        Link::ViewElement *element = shown.findElement(elementId);

        if (element == nullptr) {
            continue;
        }

        for (auto &existing : element->values) {
            if (existing.handle != value.handle) {
                continue;
            }

            existing.min = value.min;
            existing.max = value.max;
            existing.current = value.current;
            existing.known = ((value.flags & Link::ValueFlag::Known) != 0);
            existing.isSwitch = ((value.flags & Link::ValueFlag::Switch) != 0);
            existing.label = ((value.flags & Link::ValueFlag::HasLabel) != 0)
                                 ? value.label.text
                                 : "";

            if (valueChanged) {
                valueChanged(elementId, value.handle);
            }

            log(true,
                std::string("<- ") + element->name + " = "
                    + (existing.label.empty() ? std::to_string(existing.current)
                                              : existing.label));
            break;
        }
    }
}

void LinkClient::handlePresets(Link::Reader &reader)
{
    Link::PresetRecord record = {};

    if (!Link::read(reader, record)) {
        return;
    }

    /*
     * The list is a set, not a stream of additions. It arrives one preset per
     * message and index zero says a new one has started, so the old contents
     * go then - otherwise a preset that stopped running would linger, and the
     * Electra Link would offer something the controller no longer has.
     */
    if (record.index == 0) {
        presets.clear();
        presetsComplete = false;
    }

    if (record.count == 0) {
        presetsComplete = true;

        if (presetsChanged) {
            presetsChanged();
        }

        return;
    }

    Link::LivePreset preset;

    preset.presetId = record.presetId;
    preset.bankNumber = record.bankNumber;
    preset.slot = record.slot;
    preset.flags = record.flags;
    preset.name = record.name.text;
    preset.projectId = record.projectId.text;

    presets.push_back(preset);

    if ((record.index + 1) >= record.count) {
        presetsComplete = true;

        if (presetsChanged) {
            presetsChanged();
        }
    }
}

bool LinkClient::arePresetsComplete(void) const
{
    return (presetsComplete);
}

/* The one place a request is framed; the public methods name the intent. */
void LinkClient::sendRequest(Link::RequestKind kind,
                             uint8_t argument,
                             uint8_t argument2,
                             const std::string &note)
{
    uint8_t payload[8];
    Link::Writer writer(payload, sizeof(payload));
    Link::RequestRecord request = {};

    request.kind = kind;
    request.argument = argument;
    request.argument2 = argument2;

    if (Link::write(writer, request)) {
        send(Link::Opcode::Request, payload, writer.getLength());
        log(false, "-> " + note);
    }
}

void LinkClient::pressContextButton(uint8_t index)
{
    sendRequest(Link::RequestKind::ContextAction,
                index,
                0,
                "context " + std::to_string(index));
}

void LinkClient::requestPage(uint8_t pageNumber)
{
    sendRequest(Link::RequestKind::Page,
                pageNumber,
                0,
                "page " + std::to_string(pageNumber));
}

void LinkClient::requestControlSet(uint8_t controlSetId)
{
    sendRequest(Link::RequestKind::ControlSet,
                controlSetId,
                0,
                "control set " + std::to_string((int)controlSetId));
}

void LinkClient::requestPreset(uint8_t presetId)
{
    sendRequest(Link::RequestKind::Preset,
                presetId,
                0,
                "preset " + std::to_string(presetId));
}

void LinkClient::requestPresetSwitch(uint8_t presetId)
{
    sendRequest(Link::RequestKind::SwitchPreset,
                presetId,
                0,
                "load preset " + std::to_string(presetId));
}

void LinkClient::requestPin(uint8_t presetId, bool shouldPin)
{
    sendRequest(Link::RequestKind::PinPreset,
                presetId,
                shouldPin ? 1 : 0,
                std::string(shouldPin ? "pin preset " : "unpin preset ")
                    + std::to_string(presetId));
}

bool LinkClient::isMasterView(void) const
{
    return (shownKind == Link::ViewKind::MasterPerformance);
}

void LinkClient::requestMasterPerformance()
{
    sendRequest(
        Link::RequestKind::MasterPerformance, 0, 0, "master performance");
}

void LinkClient::sendSubscription(
    const std::vector<Link::SubscriptionEntry> &entries)
{
    /* Room for the three byte header and as many four byte entries as will fit
     * what one message may carry. */
    const size_t perMessage = (Link::maxPayloadLength - 8) / 4;

    size_t sent = 0;
    uint8_t sequence = 0;

    do {
        size_t count = std::min(perMessage, entries.size() - sent);

        uint8_t payload[Link::maxPayloadLength];
        Link::Writer writer(payload, sizeof(payload));
        Link::SubscribeHeaderRecord header = {};

        header.sequence = sequence++;
        header.count = (uint8_t)count;
        header.final = ((sent + count) >= entries.size()) ? 1 : 0;

        if (!Link::write(writer, header)) {
            return;
        }

        for (size_t i = 0; i < count; i++) {
            Link::write(writer, entries[sent + i]);
        }

        if (!writer.isValid()) {
            return;
        }

        send(Link::Opcode::Subscribe, payload, writer.getLength());
        sent += count;
    } while (sent < entries.size());

    log(false, "-> subscribed " + std::to_string(entries.size()) + " controls");
}

void LinkClient::requestResync()
{
    sendRequest(Link::RequestKind::Resync, 0, 0, "resync");
}

void LinkClient::sayGoodbye()
{
    if (!connected) {
        return;
    }

    send(Link::Opcode::Bye, nullptr, 0);
    connected = false;
    log(false, "-> bye");
}

void LinkClient::handleSubscriptionUpdate(Link::Reader &reader)
{
    Link::SubscriptionUpdateRecord record = {};

    if (!Link::read(reader, record)) {
        return;
    }

    if (record.index >= Link::maxSubscriptions) {
        return;
    }

    if (subscriptionValues.size() <= record.index) {
        subscriptionValues.resize(record.index + 1);
    }

    Link::SubscriptionValue &entry = subscriptionValues[record.index];

    entry.controlId = record.controlId;
    entry.name = record.name.text;
    entry.value.handle = record.value.handle;
    entry.value.min = record.value.min;
    entry.value.max = record.value.max;
    entry.value.current = record.value.current;
    entry.value.known = ((record.value.flags & Link::ValueFlag::Known) != 0);
    entry.value.isSwitch =
        ((record.value.flags & Link::ValueFlag::Switch) != 0);
    entry.value.label = record.value.label.text;
    entry.known = true;

    /*
     * A subscribed control is often also one of the elements of the view
     * being shown, and then the feed is what moves it there: a knob on the
     * Electra Link drives the subscription and never touches the view, so
     * without this the displayed control would sit still until the whole
     * view was sent again.
     *
     * The two views address differently. On a master performance the
     * element ids ARE subscription indices, so the update maps straight to
     * its tile; a preset page's elements are control ids, and there the
     * preset has to match too, because control ids repeat across presets.
     */
    Link::ViewElement *element = nullptr;

    if (isMasterView()) {
        element = shown.findElement((Link::ElementId)record.index);
    } else if (shown.getPresetId() == record.presetId) {
        element = shown.findElement((Link::ElementId)record.controlId);
    }

    if (element == nullptr) {
        return;
    }

    for (Link::ViewValue &existing : element->values) {
        if (existing.handle != record.value.handle) {
            continue;
        }

        existing.min = record.value.min;
        existing.max = record.value.max;
        existing.current = record.value.current;
        existing.known = ((record.value.flags & Link::ValueFlag::Known) != 0);
        existing.isSwitch =
            ((record.value.flags & Link::ValueFlag::Switch) != 0);
        existing.label = ((record.value.flags & Link::ValueFlag::HasLabel) != 0)
                             ? record.value.label.text
                             : "";

        /* Reported the same way a view change is, so a control moved by a
         * knob highlights and lands in the log like any other. */
        if (valueChanged) {
            valueChanged(element->id, record.value.handle);
        }

        log(true,
            std::string("<- ") + element->name + " = "
                + (existing.label.empty() ? std::to_string(existing.current)
                                          : existing.label));
        break;
    }
}

/*
 * A bitmap names its entry by (overlay, value) - the key both ends already
 * share - and arrives after the overlay's items, so the entry it decorates
 * exists in the view and in the session store by the time it lands. It is
 * attached in both places: the view is what draws now, the store is what
 * seeds the next view when the primary skips the unchanged overlay.
 */
void LinkClient::handleOverlayBitmap(Link::Reader &reader)
{
    Link::OverlayBitmapRecord record = {};

    if (!Link::read(reader, record)) {
        return;
    }

    Link::View &target = assembling ? incoming : shown;

    target.attachOverlayBitmap(
        record.overlayId, record.value, record.bytes, record.length);

    for (StoredOverlay &stored : overlayStore) {
        if ((stored.presetId != target.getPresetId())
            || (stored.overlay.id != record.overlayId)) {
            continue;
        }

        for (Link::OverlayItem &item : stored.overlay.items) {
            if (item.value == record.value) {
                item.bitmap.assign(record.bytes, record.bytes + record.length);
                break;
            }
        }

        break;
    }
}

const std::vector<Link::SubscriptionValue> &
    LinkClient::getSubscriptionValues(void) const
{
    return (subscriptionValues);
}

/*
 * The one place an input is framed. An input addressed via a subscription
 * needs no view and is not checked against one - the control it names is
 * wherever its preset is running; anything else names an element of the
 * view being shown.
 */
void LinkClient::sendInput(Link::InputKind kind,
                           Link::ElementId elementId,
                           uint8_t handle,
                           int16_t a,
                           bool viaSubscription)
{
    uint8_t payload[Link::maxPayloadLength];
    Link::Writer writer(payload, sizeof(payload));
    Link::InputRecord input = {};

    input.kind = kind;
    input.flags = viaSubscription ? Link::InputFlag::ViaSubscription : 0;
    input.viewId = viaSubscription ? 0 : shown.getViewId();
    input.elementId = elementId;
    input.handle = handle;
    input.a = a;
    input.b = 0;

    writer.writeU8(1);

    if (Link::write(writer, input)) {
        send(Link::Opcode::Input, payload, writer.getLength());
    }
}

void LinkClient::sendSubscriptionValue(uint16_t index,
                                       uint8_t handle,
                                       int32_t value)
{
    sendInput(Link::InputKind::ValueSet,
              (Link::ElementId)index,
              handle,
              (int16_t)value,
              true);
}

void LinkClient::sendButton(Link::ElementId elementId,
                            uint8_t handle,
                            bool pressed,
                            bool viaSubscription)
{
    sendInput(Link::InputKind::Button,
              elementId,
              handle,
              pressed ? 1 : 0,
              viaSubscription);
}

void LinkClient::sendGesture(const Link::GestureRecord &gesture)
{
    if (!connected || (primaryVersion < Link::primitivesMinVersion)) {
        return;
    }

    Link::GestureRecord record = gesture;

    /* Stamped here rather than by the caller: the components know their
     * element and their gesture, and which view the session is on is this
     * file's business. */
    record.viewId = shown.getViewId();

    uint8_t payload[Link::maxPayloadLength];
    Link::Writer writer(payload, sizeof(payload));

    if (Link::write(writer, record)) {
        send(Link::Opcode::Gesture, payload, writer.getLength());
    }
}

void LinkClient::sendValue(Link::ElementId elementId,
                           uint8_t handle,
                           int32_t value)
{
    sendInput(
        Link::InputKind::ValueSet, elementId, handle, (int16_t)value, false);
}
