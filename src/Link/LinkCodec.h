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
 * @file LinkCodec.h
 *
 * @brief Reading and writing Electra Link records.
 *
 * Written against a buffer the caller owns, and allocates nothing, so that the
 * same code serves the controller, an Electra Link and a desktop application.
 *
 * Every field is little endian, and every read is bounds checked. A reader
 * that runs off the end does not read anything it should not; it sets its
 * failed flag and every later call answers false. A writer that runs out of
 * room does the same. That means a truncated or malformed message is caught by
 * testing one flag at the end rather than by testing every call - and, more to
 * the point, an Electra Link cannot be made to walk off a buffer by whatever
 * arrives on its MIDI port.
 */

#pragma once

#include "src/Link/LinkRecords.h"
#include <cstddef>
#include <cstdint>

namespace Link
{
    /** @brief Writes records into a buffer. */
    class Writer
    {
    public:
        Writer(uint8_t *newBuffer, size_t newCapacity);

        bool writeU8(uint8_t value);
        bool writeU16(uint16_t value);
        bool writeI16(int16_t value);
        bool writeU24(uint32_t value);
        bool writeU32(uint32_t value);

        /**
         * @brief Write a length byte and that many bytes of text.
         *
         * Text longer than maxStringLength is cut rather than refused: a name
         * too long to send is still worth sending the front of.
         */
        bool writeString(const char *text);

        bool writeBytes(const uint8_t *data, size_t length);

        /** @brief How much has been written. */
        size_t getLength(void) const;

        /** @brief Whether everything asked for fitted. */
        bool isValid(void) const;

    private:
        uint8_t *buffer;
        size_t capacity;
        size_t length;
        bool overflowed;
    };

    /** @brief Reads records out of a buffer. */
    class Reader
    {
    public:
        Reader(const uint8_t *newBuffer, size_t newLength);

        bool readU8(uint8_t &value);
        bool readU16(uint16_t &value);
        bool readI16(int16_t &value);
        bool readU24(uint32_t &value);
        bool readU32(uint32_t &value);

        /**
         * @brief Read a length byte and that many bytes of text.
         *
         * @param text where to put it, null terminated
         * @param capacity the size of that buffer, terminator included
         *
         * A string longer than the buffer is a malformed message rather than
         * something to truncate, because the sender agreed to maxStringLength.
         */
        bool readString(char *text, size_t capacity);

        bool readBytes(uint8_t *data, size_t length);

        /** @brief Step over bytes without reading them. */
        bool skip(size_t count);

        /** @brief How much has not been read yet. */
        size_t getRemaining(void) const;

        /** @brief Whether every read so far was satisfied. */
        bool isValid(void) const;

    private:
        const uint8_t *buffer;
        size_t length;
        size_t position;
        bool failed;
    };

    /*
     * The records. Strings are held inline so that a record can be read
     * without anything being allocated for it.
     */

    struct StringField {
        char text[maxStringLength + 1];
    };

    struct HelloRecord {
        uint8_t version;
        Model model;
        uint16_t screenWidth;
        uint16_t screenHeight;
        uint8_t colourDepth;
        uint8_t potCount;
        uint8_t buttonCount;
        uint8_t flags;
        uint32_t elementTypes;
        StringField name;
    };

    struct WelcomeRecord {
        uint8_t version;
        uint16_t sessionId;
        uint8_t flags;
        StringField name;
    };

    struct ViewHeaderRecord {
        uint16_t viewId;
        ViewKind kind;

        /** Which of the primary's preset slots this view is of. */
        uint8_t presetId;

        uint8_t pageNumber;
        uint8_t activeSet;
        StringField title;

        /**
         * What the preset calls itself across every controller it is loaded
         * on. An Electra Link keys anything it remembers about a preset - a map
         * from its own knobs, say - to this rather than to a slot number,
         * which says only where the preset happens to be sitting today.
         */
        StringField projectId;

        /** Page records follow the header, so the Electra Link can offer them. */
        uint8_t pageCount;

        /** Element messages follow, one per element. */
        uint16_t elementCount;
    };

    /** @brief One page an Electra Link may ask to be shown. */
    struct PageRecord {
        uint8_t id;
        StringField name;
    };

    /** @brief One of the primary's live presets. */
    struct PresetRecord {
        /**
         * Where this one sits in the list, and how long the list is.
         *
         * The list is sent one preset per message, and it is a *set* rather
         * than a stream of additions: a preset that stops running has to
         * disappear from the Electra Link's copy. Index zero is what tells the
         * Electra Link the list has started again, so it can replace rather than
         * accumulate.
         */
        uint8_t index;
        uint8_t count;

        uint8_t presetId;
        uint8_t bankNumber;
        uint8_t slot;

        /** PresetFlag. */
        uint8_t flags;

        StringField name;

        /**
         * What the preset calls itself, wherever it is loaded.
         *
         * Sent once per preset, and never again: an Electra Link keys the
         * assignments it saves to this, so that a map made for a preset finds
         * it again after a reboot or a move to another slot. Everything else
         * on the wire uses the one byte slot id this list pairs it with.
         */
        StringField projectId;
    };

    struct ElementRecord {
        /**
         * Opaque, and assigned by the primary. For a preset page it is the
         * control's own id; for a master performance it is the Electra Link's
         * subscription index, which is what lets the Electra Link match an
         * element back to the assignment that asked for it.
         */
        ElementId id;

        /**
         * Which preset the control lives in.
         *
         * The same as the view's for a preset page, and the whole point of the
         * record for a master performance, where every element may come from a
         * different one.
         */
        uint8_t presetId;

        ElementType type;

        /**
         * What the control is on the controller, for an Electra Link that can
         * draw the real thing.
         *
         * `type` above is the negotiated shape: the primary chose it from
         * what the Electra Link said it could draw, substituting the nearest
         * claimed type for one it does not know. That is what stops an
         * Electra Link and a primary needing to be updated in lockstep, and it
         * stays exactly as it was.
         *
         * These three are the control as the controller holds it -
         * Control::Type, Control::Variant, Control::Mode - and they are
         * ignorable. An Electra Link drawing shapes of its own reads `type` and
         * skips them. An Electra Link that is an Electra One builds the
         * controller's own control component from them, which is what makes
         * its screen the same picture rather than a resemblance: variant
         * decides a fader's bar height and inset, mode decides whether the
         * bar grows from the left or from the centre, and neither can be
         * recovered from a shape.
         *
         * Zero means the primary did not say, which is what an older one
         * sends.
         */
        uint8_t nativeType;
        uint8_t nativeVariant;
        uint8_t nativeMode;

        uint8_t flags;
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
        uint32_t colour;
        uint8_t potId;
        StringField name;
        uint8_t valueCount;
    };

    struct ValueRecord {
        uint8_t handle;

        /** Which overlay this value reads its words from, or zero. */
        uint8_t overlayId;
        int16_t min;
        int16_t max;
        int16_t current;
        uint8_t flags;
        StringField label;
    };

    /** @brief One subscribed control, by its place in the subscription. */
    /**
     * @brief The head of one overlay message: which overlay, and how much.
     *
     * `sequence` counts messages within one overlay, not items; an Electra Link
     * takes sequence 0 as "start this overlay again", so a view re-sent after
     * a page change does not append to what the last one left. `final` says
     * the overlay is complete.
     */
    struct OverlayHeaderRecord {
        uint8_t id;
        uint8_t sequence;
        uint8_t final;
        uint8_t count;
    };

    /** @brief One entry of an overlay. */
    struct OverlayItemRecord {
        int16_t value;

        /** Its own colour, when it declared one - an overlay entry may paint
         *  the control a different colour from the control's own. */
        uint32_t colour;
        uint8_t colourIsSet;
        StringField label;
    };

    /** @brief One overlay entry's bitmap, addressed by the entry's value. */
    struct OverlayBitmapRecord {
        uint8_t overlayId;

        /** Which entry of the overlay, by the value it stands for - the one
         *  key both ends already share. */
        int16_t value;

        uint8_t length;
        uint8_t bytes[maxOverlayBitmapBytes];
    };

    bool write(Writer &writer, const OverlayBitmapRecord &record);
    bool read(Reader &reader, OverlayBitmapRecord &record);

    /**
     * @brief The head of one Primitives message: whose picture, and how much
     *  of it.
     *
     * The op bytes follow the header in the same message, `count` of them. A
     * frame larger than one message travels in pieces: `sequence` counts the
     * messages of one frame, sequence zero starts the frame afresh, and
     * `final` is what commits it - an Electra Link draws nothing from a frame it
     * only has the front of. `viewId` is the view the frame was captured
     * for; a frame arriving after the view moved on is dropped rather than
     * painted onto whatever element shares the number.
     */
    struct PrimitivesHeaderRecord {
        uint16_t viewId;
        ElementId elementId;
        uint8_t sequence;
        uint8_t final;
        uint16_t count;
    };

    bool write(Writer &writer, const PrimitivesHeaderRecord &record);
    bool read(Reader &reader, PrimitivesHeaderRecord &record);

    /**
     * @brief One gesture made on an Electra Link's custom control.
     *
     * Every field is written whatever the kind, because a fixed record is
     * simpler than four and the whole thing is sixteen bytes. What each
     * field means by kind is documented on Link::GestureKind; a field a kind
     * has no use for is zero.
     */
    struct GestureRecord {
        uint16_t viewId;
        ElementId elementId;
        uint8_t kind;

        /** The event within the kind: a touch event type, a pot Down/Up, a
         *  switch press (1) or release (0). */
        uint8_t event;
        uint8_t potId;
        uint8_t touchId;
        int16_t x;
        int16_t y;
        int16_t downX;
        int16_t downY;
        int16_t delta;
    };

    bool write(Writer &writer, const GestureRecord &record);
    bool read(Reader &reader, GestureRecord &record);

    /** @brief One context button, as the controller labels it. */
    struct ContextButtonRecord {
        uint8_t index;
        uint8_t count;
        StringField label;
    };

    /** @brief One preset bank, as the owner configured it. */
    struct BankRecord {
        uint8_t index;

        /** How many banks there are, so an Electra Link knows when it has them
         *  all without the message having to be split. */
        uint8_t count;
        uint32_t colour;
        StringField name;
    };

    struct SubscriptionUpdateRecord {
        uint16_t index;
        uint8_t presetId;
        uint16_t controlId;
        StringField name;
        ValueRecord value;
    };

    struct InputRecord {
        InputKind kind;

        /**
         * The view the Electra Link was looking at when the user did this.
         *
         * An element id means different things on different views - a
         * control's own id on a preset page, a place in the subscription on a
         * master performance - so an input that arrives after the view has
         * changed would be read with the wrong meaning and move whatever
         * happened to share the number. The controller discards an input that
         * does not name the view it is showing.
         */
        uint16_t viewId;

        /** @see Link::InputFlag */
        uint8_t flags;
        ElementId elementId;
        uint8_t handle;
        int16_t a;
        int16_t b;
    };

    /** @brief One control an Electra Link wants gathered into its master view. */
    struct SubscriptionEntry {
        uint8_t presetId;
        uint16_t controlId;
        uint8_t handle;
    };

    /**
     * @brief A piece of an Electra Link's subscription.
     *
     * A full set of assignments does not fit one SysEx message, so it arrives
     * in pieces. Sequence zero starts a new set - the primary replaces rather
     * than accumulates - and the last piece says so, which is when the master
     * view is composed.
     */
    struct SubscribeHeaderRecord {
        uint8_t sequence;
        uint8_t final;
        uint8_t count;
    };

    struct RequestRecord {
        RequestKind kind;
        uint8_t argument;

        /**
         * A second argument, for the requests that need one.
         *
         * Pinning names a preset and says which way, and a toggle would have
         * been the shorter thing to send and the wrong one: a request that
         * arrives twice would undo itself, and the Electra Link would be telling
         * the controller to change something rather than what to change it to.
         */
        uint8_t argument2;
    };

    bool write(Writer &writer, const HelloRecord &record);
    bool read(Reader &reader, HelloRecord &record);

    bool write(Writer &writer, const WelcomeRecord &record);
    bool read(Reader &reader, WelcomeRecord &record);

    bool write(Writer &writer, const ViewHeaderRecord &record);
    bool read(Reader &reader, ViewHeaderRecord &record);

    bool write(Writer &writer, const PageRecord &record);
    bool read(Reader &reader, PageRecord &record);

    bool write(Writer &writer, const PresetRecord &record);
    bool read(Reader &reader, PresetRecord &record);

    bool write(Writer &writer, const ElementRecord &record);
    bool read(Reader &reader, ElementRecord &record);

    bool write(Writer &writer, const ContextButtonRecord &record);
    bool read(Reader &reader, ContextButtonRecord &record);
    bool write(Writer &writer, const BankRecord &record);
    bool read(Reader &reader, BankRecord &record);
    bool write(Writer &writer, const OverlayHeaderRecord &record);
    bool read(Reader &reader, OverlayHeaderRecord &record);
    bool write(Writer &writer, const OverlayItemRecord &record);
    bool read(Reader &reader, OverlayItemRecord &record);
    bool write(Writer &writer, const ValueRecord &record);
    bool read(Reader &reader, ValueRecord &record);

    bool write(Writer &writer, const SubscriptionUpdateRecord &record);
    bool read(Reader &reader, SubscriptionUpdateRecord &record);

    bool write(Writer &writer, const InputRecord &record);
    bool read(Reader &reader, InputRecord &record);

    bool write(Writer &writer, const RequestRecord &record);
    bool read(Reader &reader, RequestRecord &record);

    bool write(Writer &writer, const SubscribeHeaderRecord &record);
    bool read(Reader &reader, SubscribeHeaderRecord &record);

    bool write(Writer &writer, const SubscriptionEntry &record);
    bool read(Reader &reader, SubscriptionEntry &record);

    /**
     * @brief Base64-decode one link payload into a bounded buffer.
     *
     * The one decode every program must use. The underlying base64 routine
     * writes without knowing the destination's size and appends a NUL past
     * the data, so it may never be pointed at a message-sized buffer
     * directly; this decodes into scratch sized for the worst legal message
     * and copies out only what fits.
     *
     * @return the decoded length, or -1 when the input is longer than the
     *         protocol allows or the result does not fit destCapacity.
     */
    int decodePayload(uint8_t *dest,
                      size_t destCapacity,
                      const char *encoded,
                      size_t encodedLength);
} // namespace Link
