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
 * @file LinkRecords.h
 *
 * @brief The vocabulary of the Electra Satellite Link.
 *
 * One controller - the primary - owns the preset, the parameter map, the Lua
 * state and the router. A satellite owns nothing: it draws the view it is
 * given and reports what the user did to it. A satellite may be another
 * Electra One running in slave mode, or an application on a computer.
 *
 * This header is deliberately in src/ rather than in controller/ or in the
 * desktop tool. Three programs have to agree on these numbers - the primary,
 * the satellite and the desktop application - and the only way to be sure they
 * do is for all three to compile the same file. It depends on nothing but
 * stdint for the same reason: a satellite links the framework and the widget
 * layer, and nothing of the controller at all.
 *
 * The wire format is described in developers/satellitelink.md, which also
 * carries the developer guide: sessions, views, custom controls, the Lua
 * API, and how to build a satellite or a primary of your own. When the
 * document and this header disagree, this header wins.
 */

#pragma once

#include <cstdint>

namespace Link
{
    /** @brief The protocol version this build speaks.
     *
     * 2 added the native fields to an element: what the control actually is
     * on the controller, rather than only what shape a satellite should make
     * of it. See ElementRecord.
     *
     * 3 added overlays, so that a list can draw its items.
     *
     * 4 added the preset banks - their names and colours - so that a
     * satellite's preset chooser is labelled the way the owner labelled it.
     *
     * 5 added the context buttons: their labels out, and which one was
     * pressed back.
     *
     * 6 made overlays cacheable per session: a primary may omit an overlay
     * the satellite already holds unchanged, so the satellite must keep the
     * overlays it was sent for as long as the session lasts. The wire format
     * of every message is unchanged - only how often overlays travel. A
     * primary only omits them for a satellite whose hello said 6 or newer.
     *
     * 7 added the drawing primitives: a control whose Lua paints it by hand
     * is captured on the primary as a list of drawing calls and replayed by
     * the satellite. The negotiated element type stays Opaque, so an older
     * satellite draws the named frame it always did. And the reply
     * direction: a gesture made on a satellite's custom control - a touch, a
     * knob, a knob's switch - travels back raw and is replayed into the
     * preset's own Lua callbacks, because a custom control's input, like its
     * look, is whatever the script says it is.
     *
     * 8 added the control set request, and the mk1 as a satellite model. A
     * satellite's page was always its own; which row of that page its knobs
     * drive is now its own as well, so the two panels can work two rows of
     * one page at once. A primary that predates this ignores the request,
     * and a satellite that predates it never sends one.
     */
    constexpr uint8_t protocolVersion = 8;

    /** @brief The oldest satellite that keeps overlays across views. */
    constexpr uint8_t overlayCacheMinVersion = 6;

    /** @brief The oldest satellite that replays drawing primitives. */
    constexpr uint8_t primitivesMinVersion = 7;

    /**
     * @brief The most bytes one captured paint may record.
     *
     * A frame past this is truncated and logged rather than refused: a
     * drawing missing its last calls is still mostly the drawing, and a
     * script painting pixel by pixel would otherwise silence itself.
     */
    constexpr uint16_t maxPrimitiveBytes = 1536;

    /**
     * @brief The most bytes one overlay bitmap may carry.
     *
     * Sized for the controller's 48 x 18 1-bpp items (108 bytes) with room
     * to spare; a message-level bound so the codec need not know the
     * display's geometry.
     */
    constexpr uint8_t maxOverlayBitmapBytes = 128;

    /**
     * @brief The SysEx command byte carrying every link message.
     *
     * F0 00 21 45 0A <opcode> <base64 payload> F7. The byte after the command
     * is an opcode, not an object - the Event command uses that position the
     * same way.
     */
    constexpr uint8_t commandByte = 0x0A;

    /** @brief What a message is. */
    enum class Opcode : uint8_t {
        None = 0x00,

        /** Satellite to primary: I am here, and this is what I am. */
        Hello = 0x01,

        /** Primary to satellite: session accepted. */
        Welcome = 0x02,

        /** Primary to satellite: draw this, replacing everything. */
        View = 0x03,

        /** Primary to satellite: these things changed. */
        Update = 0x04,

        /**
         * Primary to satellite: one element of the view being sent.
         *
         * A view is sent as a header naming how many elements follow, then one
         * message per element. A satellite that is another Electra One reads
         * SysEx into a 512 byte buffer, so a whole page could not arrive as
         * one message however the primary composed it - and sending each
         * element on its own means no chunking protocol has to exist.
         */
        Element = 0x0B,

        /**
         * Primary to satellite: one custom control's picture, as the drawing
         * calls its Lua paint callback made.
         *
         * The 0x05 slot was reserved for drawing primitives from the first
         * design, and this is them - narrowed to the one place nothing else
         * can serve. A custom control is painted by the preset's script, and
         * the script runs only on the primary; every other element type is
         * rebuilt from its record, but there is no describing a hand-painted
         * control except by what it drew. The ops are the Lua graphics
         * vocabulary verbatim, in control-relative coordinates, so the
         * satellite replays them wherever its layout put the element.
         *
         * A frame larger than one message travels in pieces: sequence zero
         * starts a frame, `final` commits it, exactly as an overlay's items
         * do. Only sent to a satellite whose hello said version 7 or newer;
         * the negotiated element type stays Opaque, so an older satellite
         * still draws the named frame.
         */
        Primitives = 0x05,

        /** Primary to satellite: a device level instruction. */
        Command = 0x06,

        /** Satellite to primary: the user did this. */
        Input = 0x07,

        /** Satellite to primary: send me a view. */
        Request = 0x08,

        /** Primary to satellite: which presets are live and can be shown. */
        Presets = 0x0C,

        /**
         * Satellite to primary: the controls its own knobs are assigned to.
         *
         * The satellite holds the assignments - they describe its hardware,
         * not the preset - so it is the satellite that says which controls it
         * wants gathered. Sent in pieces, because a full set of assignments
         * does not fit one SysEx message.
         */
        Subscribe = 0x0D,

        /**
         * Primary to satellite: a subscribed control moved.
         *
         * A standing feed rather than part of a view: the satellite is told
         * about every control it asked to have gathered, whatever it is
         * showing at the time. Without it a knob could only pick up a value it
         * could see, so takeover for a preset running in the background had
         * nothing to take over from.
         */
        SubscriptionUpdate = 0x0E,

        /**
         * Primary to satellite: the items of one overlay.
         *
         * A list draws its items, not just the one it is on, and a fader with
         * an overlay shows a word where its number would be. Neither can be
         * had from a value: the whole set has to arrive.
         *
         * Sent with a view, before the elements that reference it, and in
         * pieces - an overlay may hold more entries than one message can
         * carry. A value names the overlay it uses, so overlays shared by
         * several controls are sent once.
         */
        Overlay = 0x0F,

        /**
         * Primary to satellite: what the preset banks are called, and their
         * colours.
         *
         * A bank's name and colour are the owner's configuration, not
         * anything a satellite can work out, and a chooser that labelled them
         * A to E would be showing something the person does not recognise.
         * All of them fit one message.
         */
        Banks = 0x10,

        /**
         * Primary to satellite: what its context buttons are called.
         *
         * Labels only. What any of them does is the controller's business and
         * the satellite is never told - it reports which was pressed, and the
         * controller runs the command. That is what keeps a satellite unable
         * to do anything its owner did not ask the controller to do.
         */
        ContextButtons = 0x11,

        /**
         * Primary to satellite: the picture one overlay entry draws.
         *
         * Sent after the overlay's items, one message per entry that has a
         * bitmap, and only to a satellite whose hello said version 6 or
         * newer - an older one does not know the opcode. The bytes are the
         * packed 1-bpp source exactly as a preset document carries them,
         * because a satellite paints with its own display and needs bits,
         * not an address in the primary's.
         */
        OverlayBitmap = 0x12,

        /**
         * Satellite to primary: a gesture made on a custom control.
         *
         * Not an Input. An input names a value and what to make of it, and
         * the primary applies it by its own rules - but a custom control's
         * behaviour is its script's, and the script wants what its callbacks
         * are given on the controller: the raw touch with its coordinates,
         * the knob's turn, the switch's press. So the gesture travels as
         * itself, in control-relative coordinates, and the primary replays
         * it into the preset's Lua. Only a version 7 satellite sends it, and
         * only for elements whose native type is custom.
         */
        Gesture = 0x13,

        Ping = 0x09,
        Pong = 0x0A,

        /** Primary to satellite: the session is over. */
        Bye = 0x7F
    };

    /** @brief What kind of machine a satellite is. */
    enum class Model : uint8_t {
        Unknown = 0,
        Mk2 = 1,
        Mini = 2,
        Host = 3,

        /** The original Electra One, running its own link program. */
        Mk1 = 4
    };

    /**
     * @brief What a satellite draws an element as.
     *
     * These are not Control::Type. A satellite has no preset and no controls;
     * it has shapes to draw. The primary decides which shape describes each of
     * its controls, and that is also where a control type the satellite does
     * not know is turned into the nearest one it does.
     */
    enum class ElementType : uint8_t {
        None = 0x00,
        Fader = 0x01,
        Vfader = 0x02,
        List = 0x03,
        Pad = 0x04,
        Dial = 0x05,
        Envelope = 0x06,
        XyPad = 0x07,
        Group = 0x08,

        /**
         * A labelled box, drawn with no graphics at all.
         *
         * What a master performance is made of. Its controls come from
         * different presets and are gathered by the knob that drives them
         * rather than by anything they have in common, so drawing them as
         * faders and dials would be borrowing an appearance that means
         * nothing here. A box says which preset, which control, what value,
         * and which knob - which is the whole of what the view is for.
         */
        Tile = 0x09,

        /**
         * Something the satellite cannot draw - a control whose Lua paints it
         * by hand. Drawn as a named frame with its values shown plainly, and
         * still playable, because its values are sent like anyone else's.
         */
        Opaque = 0x7F
    };

    /** @brief One bit per ElementType, for the capabilities in Hello. */
    constexpr uint32_t elementTypeBit(ElementType type)
    {
        return ((type == ElementType::Opaque)
                    ? (uint32_t)(1u << 31)
                    : (uint32_t)(1u << ((uint8_t)type - 1)));
    }

    /**
     * @brief Every drawable element type's bit, for a satellite that draws
     *  with the controller's own control components and so can draw them all.
     *
     * Tile is deliberately absent: it is what a satellite *invents* for a
     * master performance, not something a primary chooses for a control.
     */
    constexpr uint32_t allElementTypes =
        elementTypeBit(ElementType::Fader) | elementTypeBit(ElementType::Vfader)
        | elementTypeBit(ElementType::List) | elementTypeBit(ElementType::Pad)
        | elementTypeBit(ElementType::Dial)
        | elementTypeBit(ElementType::Envelope)
        | elementTypeBit(ElementType::XyPad)
        | elementTypeBit(ElementType::Group)
        | elementTypeBit(ElementType::Opaque);

    /** @brief What a satellite says it is able to do. */
    namespace Capability
    {
        constexpr uint8_t TouchScreen = 1 << 0;
        /* bit 1 was "accepts drawing primitives", now reserved. */
    } // namespace Capability

    /** @brief Element record flags. */
    namespace ElementFlag
    {
        constexpr uint8_t Visible = 1 << 0;

        /** In the control set the satellite's pots currently drive. */
        constexpr uint8_t Active = 1 << 1;
    } // namespace ElementFlag

    /** @brief Preset record flags. */
    namespace PresetFlag
    {
        /**
         * Loaded and running.
         *
         * The list carries every preset stored on the controller, not only
         * the running ones, so that a satellite can offer them all and ask for
         * one to be loaded. Clear means it is on the card and nothing more.
         */
        constexpr uint8_t Live = 1 << 0;

        /** Kept running although it is not the one on the primary's screen. */
        constexpr uint8_t Pinned = 1 << 1;

        /** The one the primary is showing. */
        constexpr uint8_t OnScreen = 1 << 2;

        /** The one this satellite is being sent. */
        constexpr uint8_t Shown = 1 << 3;
    } // namespace PresetFlag

    /** @brief Value record flags. */
    namespace ValueFlag
    {
        /**
         * The primary knows what this value is. Clear means it does not
         * either, and the satellite should show that rather than a number it
         * would be inventing.
         */
        constexpr uint8_t Known = 1 << 0;

        /** A label follows, and is what the value should be shown as. */
        constexpr uint8_t HasLabel = 1 << 1;

        /**
         * A switch rather than a quantity: it is pressed and released, not
         * set.
         *
         * The satellite reports the press and the release and nothing else -
         * whether that makes the control latch or follow the finger is the
         * primary's to decide, because the primary is where the mode and the
         * current state live. A satellite that worked it out for itself would
         * be a second implementation of the same rules, free to disagree.
         */
        constexpr uint8_t Switch = 1 << 2;
    } // namespace ValueFlag

    namespace InputFlag
    {
        /**
         * The element id is a place in the subscription rather than anything
         * on a view.
         *
         * Addressing is a separate question from what the user did, so it is a
         * flag rather than a kind of its own - otherwise every kind of input
         * would need a subscription-addressed twin.
         */
        constexpr uint8_t ViaSubscription = 1 << 0;
    } // namespace InputFlag

    /** @brief What a View is showing. */
    enum class ViewKind : uint8_t {
        None = 0,
        PresetPage = 1,
        ControllerScreen = 2,

        /**
         * Every control the satellite's own knobs are assigned to, gathered
         * from whichever presets they live in.
         *
         * It is not a page of anything: the controls come from several presets
         * at once, including ones running in the background that nothing is
         * displaying, and the controller lays them out afresh for the
         * satellite's screen.
         *
         * Showing them is also what makes driving them correct. Takeover has
         * to know where a value sits, and a satellite only knows the values it
         * has been sent - so a knob can pick up a control in a background
         * preset precisely because that control is on this view.
         */
        MasterPerformance = 3
    };

    /** @brief What one change record changes. */
    enum class ChangeKind : uint8_t {
        None = 0,
        Value = 1,
        Properties = 2,
        Added = 3,
        Removed = 4
    };

    /** @brief What the user did to a satellite. */
    enum class InputKind : uint8_t {
        None = 0,
        PotTurned = 1,
        PotTouched = 2,
        TouchDown = 3,
        TouchMoved = 4,
        TouchUp = 5,
        /**
         * A switch went down or came up, in `a`: 1 pressed, 0 released.
         *
         * Both are reported and neither says what the control should become.
         * A pad may latch or follow the finger, and only the primary knows
         * which and what it currently is.
         */
        Button = 6,

        /**
         * The value the user arrived at, rather than how far they moved.
         * For satellites with no pots, where a mouse drag knows the answer.
         */
        ValueSet = 7
    };

    /** @brief What a gesture on a custom control was. */
    enum class GestureKind : uint8_t {
        None = 0,

        /** The knob under the element turned; `delta` says how far. */
        PotTurn = 1,

        /** A finger arrived on that knob, or left it; `event` says which. */
        PotTouch = 2,

        /** The knob's switch went down or came up; `event` says which. */
        Switch = 3,

        /**
         * A finger on the element itself. `event` is the touch event type as
         * the controller's own gesture system names it, and the coordinates
         * are control-relative - the satellite's layout and the primary's
         * need not agree about where the element sits.
         */
        Touch = 4
    };

    /** @brief What a Request asks for. */
    enum class RequestKind : uint8_t {
        None = 0,
        CurrentView = 1,
        Page = 2,
        Resync = 3,

        /**
         * Show a different one of the primary's live presets.
         *
         * The argument is a preset slot id. A satellite may ask; the primary
         * decides, and refuses a slot that is not live. This is what points a
         * satellite at a preset pinned in the background while the primary's
         * own screen stays on the one being performed.
         */
        Preset = 4,

        /** Send the list of presets. */
        PresetList = 5,

        /** Show the controls this satellite subscribed to. */
        MasterPerformance = 8,

        /**
         * Load a preset onto the controller.
         *
         * Distinct from Preset, which only chooses among what is already
         * running. This is the satellite asking the controller to change what
         * it is doing, which is a bigger thing to ask and is worth saying
         * plainly rather than inferring from a request it could not otherwise
         * honour. The satellite is then shown it once it is up.
         */
        SwitchPreset = 6,

        /**
         * Keep a preset running when it is not the one on the screen, or stop
         * doing so. The argument is the preset slot, the second argument is
         * whether to pin it.
         *
         * Pinning is what makes a background preset possible at all, and a
         * satellite is the main reason to want one - so it is the satellite
         * that most needs to be able to ask for it.
         */
        PinPreset = 7,

        /**
         * Run the controller command a satellite's context button stands for.
         *
         * The argument is which button, counted from zero. The satellite does
         * not know what it does, which is why this says "the button" rather
         * than naming a command: a satellite that could name one could ask
         * for anything.
         */
        ContextAction = 9,

        /**
         * Drive a different row of the page being shown.
         *
         * The argument is a control set, counted from one; zero asks for
         * whatever the primary would send on its own - its own set for the
         * preset on its screen, every element playable for one in the
         * background. A satellite's knobs are as many as a row has, so this
         * is what lets a second panel work a row the primary is not on.
         */
        ControlSet = 10
    };

    /** @brief What a Command instructs. */
    enum class CommandKind : uint8_t {
        None = 0,
        Brightness = 1,
        Sleep = 2,
        Message = 3,
        Clear = 4
    };

    /** @brief The most overlays one view may carry.
     *
     * A page's controls between them read from a handful; the cap exists so
     * that noting which have been sent costs a fixed array rather than an
     * allocation on the controller. */
    constexpr uint8_t maxOverlaysPerView = 32;

    /** @brief The most controls a satellite may subscribe to.
     *
     * One per CC, which is what a satellite can address. */
    constexpr uint8_t maxSubscriptions = 128;

    /** @brief The longest string any record carries, not counting its length
     *  byte. Names, titles and value labels are all bounded by it. */
    constexpr uint8_t maxStringLength = 63;

    /**
     * @brief The largest payload any one message carries, before encoding.
     *
     * A satellite that is another Electra One reads SysEx into a 512 byte
     * buffer, and base64 costs a third again, so the message a primary
     * composes has to stay well inside that. This is why a view is sent as a
     * header and one message per element rather than as one large message: no
     * single record comes anywhere near this, and no chunking has to exist.
     */
    constexpr uint16_t maxPayloadLength = 320;

    /** @brief Room for maxPayloadLength once base64 has had it. */
    constexpr uint16_t maxEncodedLength = ((maxPayloadLength + 2) / 3) * 4 + 4;

    /**
     * @brief How often a connected satellite pings, in milliseconds.
     *
     * The keepalive is symmetric in vocabulary but asymmetric in duty: the
     * satellite pings, the primary answers. Both ends time against these
     * shared numbers so neither can misjudge the other.
     */
    constexpr uint32_t pingIntervalMs = 2000;

    /** @brief Missed pings before a satellite gives the primary up. */
    constexpr uint8_t pingMissesAllowed = 3;

    /**
     * @brief Silence, in milliseconds, before a primary gives a satellite up.
     *
     * A connected satellite pings every pingIntervalMs, so this allows three
     * lost pings and slack before the primary closes the session and lets go
     * of what it was holding for it - a shown preset's parameter map stays
     * enabled for as long as a satellite is watching it.
     */
    constexpr uint32_t livenessTimeoutMs = 8000;

    /** @brief The id a primary gives an element.
     *
     * Opaque to the satellite. It is not a control id and nothing may be read
     * into it; it exists so that an Input or an Update can name an element,
     * and only the primary knows what it refers to.
     */
    using ElementId = uint16_t;

    /** @brief No element. */
    constexpr ElementId NoElement = 0;
} // namespace Link
