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

#include "src/Link/LinkCodec.h"
#include "Base64.h"
#include <cstring>

namespace Link
{
    Writer::Writer(uint8_t *newBuffer, size_t newCapacity)
        : buffer(newBuffer), capacity(newCapacity), length(0), overflowed(false)
    {
    }

    bool Writer::writeU8(uint8_t value)
    {
        if ((length + 1) > capacity) {
            overflowed = true;
            return (false);
        }

        buffer[length++] = value;

        return (true);
    }

    bool Writer::writeU16(uint16_t value)
    {
        return (writeU8((uint8_t)(value & 0xFF))
                && writeU8((uint8_t)((value >> 8) & 0xFF)));
    }

    bool Writer::writeI16(int16_t value)
    {
        return (writeU16((uint16_t)value));
    }

    bool Writer::writeU24(uint32_t value)
    {
        return (writeU8((uint8_t)(value & 0xFF))
                && writeU8((uint8_t)((value >> 8) & 0xFF))
                && writeU8((uint8_t)((value >> 16) & 0xFF)));
    }

    bool Writer::writeU32(uint32_t value)
    {
        return (writeU24(value) && writeU8((uint8_t)((value >> 24) & 0xFF)));
    }

    bool Writer::writeString(const char *text)
    {
        size_t textLength = (text != nullptr) ? strlen(text) : 0;

        if (textLength > maxStringLength) {
            textLength = maxStringLength;
        }

        if (!writeU8((uint8_t)textLength)) {
            return (false);
        }

        return (writeBytes((const uint8_t *)text, textLength));
    }

    bool Writer::writeBytes(const uint8_t *data, size_t bytesToWrite)
    {
        if (bytesToWrite == 0) {
            return (true);
        }

        if ((data == nullptr) || ((length + bytesToWrite) > capacity)) {
            overflowed = true;
            return (false);
        }

        memcpy(&buffer[length], data, bytesToWrite);
        length += bytesToWrite;

        return (true);
    }

    size_t Writer::getLength(void) const
    {
        return (length);
    }

    bool Writer::isValid(void) const
    {
        return (!overflowed);
    }

    Reader::Reader(const uint8_t *newBuffer, size_t newLength)
        : buffer(newBuffer), length(newLength), position(0), failed(false)
    {
        if (buffer == nullptr) {
            length = 0;
        }
    }

    bool Reader::readU8(uint8_t &value)
    {
        if (failed || ((position + 1) > length)) {
            failed = true;
            value = 0;
            return (false);
        }

        value = buffer[position++];

        return (true);
    }

    bool Reader::readU16(uint16_t &value)
    {
        uint8_t low = 0;
        uint8_t high = 0;

        if (!readU8(low) || !readU8(high)) {
            value = 0;
            return (false);
        }

        value = (uint16_t)(low | (high << 8));

        return (true);
    }

    bool Reader::readI16(int16_t &value)
    {
        uint16_t raw = 0;

        if (!readU16(raw)) {
            value = 0;
            return (false);
        }

        value = (int16_t)raw;

        return (true);
    }

    bool Reader::readU24(uint32_t &value)
    {
        uint8_t low = 0;
        uint8_t middle = 0;
        uint8_t high = 0;

        if (!readU8(low) || !readU8(middle) || !readU8(high)) {
            value = 0;
            return (false);
        }

        value =
            (uint32_t)low | ((uint32_t)middle << 8) | ((uint32_t)high << 16);

        return (true);
    }

    bool Reader::readU32(uint32_t &value)
    {
        uint32_t low = 0;
        uint8_t high = 0;

        if (!readU24(low) || !readU8(high)) {
            value = 0;
            return (false);
        }

        value = low | ((uint32_t)high << 24);

        return (true);
    }

    bool Reader::readString(char *text, size_t textCapacity)
    {
        uint8_t textLength = 0;

        if ((text == nullptr) || (textCapacity == 0)) {
            failed = true;
            return (false);
        }

        text[0] = '\0';

        if (!readU8(textLength)) {
            return (false);
        }

        /* The sender agreed to maxStringLength, so anything longer than the buffer
     * is a malformed message rather than something to shorten. */
        if ((size_t)textLength >= textCapacity) {
            failed = true;
            return (false);
        }

        if (!readBytes((uint8_t *)text, textLength)) {
            return (false);
        }

        text[textLength] = '\0';

        return (true);
    }

    bool Reader::readBytes(uint8_t *data, size_t bytesToRead)
    {
        if (bytesToRead == 0) {
            return (true);
        }

        if (failed || (data == nullptr)
            || ((position + bytesToRead) > length)) {
            failed = true;
            return (false);
        }

        memcpy(data, &buffer[position], bytesToRead);
        position += bytesToRead;

        return (true);
    }

    bool Reader::skip(size_t count)
    {
        if (failed || ((position + count) > length)) {
            failed = true;
            return (false);
        }

        position += count;

        return (true);
    }

    size_t Reader::getRemaining(void) const
    {
        return (failed ? 0 : (length - position));
    }

    bool Reader::isValid(void) const
    {
        return (!failed);
    }

    /*
 * The records.
 *
 * Each pair below is written to be read side by side: the write and the read
 * must name the fields in the same order, and a field added to one without the
 * other is the one mistake this format cannot survive.
 */

    bool write(Writer &writer, const HelloRecord &record)
    {
        writer.writeU8(record.version);
        writer.writeU8((uint8_t)record.model);
        writer.writeU16(record.screenWidth);
        writer.writeU16(record.screenHeight);
        writer.writeU8(record.colourDepth);
        writer.writeU8(record.potCount);
        writer.writeU8(record.buttonCount);
        writer.writeU8(record.flags);
        writer.writeU32(record.elementTypes);
        writer.writeString(record.name.text);

        return (writer.isValid());
    }

    bool read(Reader &reader, HelloRecord &record)
    {
        uint8_t model = 0;

        reader.readU8(record.version);
        reader.readU8(model);
        reader.readU16(record.screenWidth);
        reader.readU16(record.screenHeight);
        reader.readU8(record.colourDepth);
        reader.readU8(record.potCount);
        reader.readU8(record.buttonCount);
        reader.readU8(record.flags);
        reader.readU32(record.elementTypes);
        reader.readString(record.name.text, sizeof(record.name.text));

        record.model = (Model)model;

        return (reader.isValid());
    }

    bool write(Writer &writer, const WelcomeRecord &record)
    {
        writer.writeU8(record.version);
        writer.writeU16(record.sessionId);
        writer.writeU8(record.flags);
        writer.writeString(record.name.text);

        return (writer.isValid());
    }

    bool read(Reader &reader, WelcomeRecord &record)
    {
        reader.readU8(record.version);
        reader.readU16(record.sessionId);
        reader.readU8(record.flags);
        reader.readString(record.name.text, sizeof(record.name.text));

        return (reader.isValid());
    }

    bool write(Writer &writer, const ViewHeaderRecord &record)
    {
        writer.writeU16(record.viewId);
        writer.writeU8((uint8_t)record.kind);
        writer.writeU8(record.presetId);
        writer.writeU8(record.pageNumber);
        writer.writeU8(record.activeSet);
        writer.writeString(record.title.text);
        writer.writeString(record.projectId.text);
        writer.writeU8(record.pageCount);
        writer.writeU16(record.elementCount);

        return (writer.isValid());
    }

    bool read(Reader &reader, ViewHeaderRecord &record)
    {
        uint8_t kind = 0;

        reader.readU16(record.viewId);
        reader.readU8(kind);
        reader.readU8(record.presetId);
        reader.readU8(record.pageNumber);
        reader.readU8(record.activeSet);
        reader.readString(record.title.text, sizeof(record.title.text));
        reader.readString(record.projectId.text, sizeof(record.projectId.text));
        reader.readU8(record.pageCount);
        reader.readU16(record.elementCount);

        record.kind = (ViewKind)kind;

        return (reader.isValid());
    }

    bool write(Writer &writer, const PageRecord &record)
    {
        writer.writeU8(record.id);
        writer.writeString(record.name.text);

        return (writer.isValid());
    }

    bool read(Reader &reader, PageRecord &record)
    {
        reader.readU8(record.id);
        reader.readString(record.name.text, sizeof(record.name.text));

        return (reader.isValid());
    }

    bool write(Writer &writer, const PresetRecord &record)
    {
        writer.writeU8(record.index);
        writer.writeU8(record.count);
        writer.writeU8(record.presetId);
        writer.writeU8(record.bankNumber);
        writer.writeU8(record.slot);
        writer.writeU8(record.flags);
        writer.writeString(record.name.text);
        writer.writeString(record.projectId.text);

        return (writer.isValid());
    }

    bool read(Reader &reader, PresetRecord &record)
    {
        reader.readU8(record.index);
        reader.readU8(record.count);
        reader.readU8(record.presetId);
        reader.readU8(record.bankNumber);
        reader.readU8(record.slot);
        reader.readU8(record.flags);
        reader.readString(record.name.text, sizeof(record.name.text));
        reader.readString(record.projectId.text, sizeof(record.projectId.text));

        return (reader.isValid());
    }

    bool write(Writer &writer, const ElementRecord &record)
    {
        writer.writeU16(record.id);
        writer.writeU8(record.presetId);
        writer.writeU8((uint8_t)record.type);
        writer.writeU8(record.nativeType);
        writer.writeU8(record.nativeVariant);
        writer.writeU8(record.nativeMode);
        writer.writeU8(record.flags);
        writer.writeU16(record.x);
        writer.writeU16(record.y);
        writer.writeU16(record.width);
        writer.writeU16(record.height);
        writer.writeU24(record.colour);
        writer.writeU8(record.potId);
        writer.writeString(record.name.text);
        writer.writeU8(record.valueCount);

        return (writer.isValid());
    }

    bool read(Reader &reader, ElementRecord &record)
    {
        uint8_t type = 0;

        reader.readU16(record.id);
        reader.readU8(record.presetId);
        reader.readU8(type);
        reader.readU8(record.nativeType);
        reader.readU8(record.nativeVariant);
        reader.readU8(record.nativeMode);
        reader.readU8(record.flags);
        reader.readU16(record.x);
        reader.readU16(record.y);
        reader.readU16(record.width);
        reader.readU16(record.height);
        reader.readU24(record.colour);
        reader.readU8(record.potId);
        reader.readString(record.name.text, sizeof(record.name.text));
        reader.readU8(record.valueCount);

        record.type = (ElementType)type;

        return (reader.isValid());
    }

    bool write(Writer &writer, const ContextButtonRecord &record)
    {
        writer.writeU8(record.index);
        writer.writeU8(record.count);
        writer.writeString(record.label.text);

        return (writer.isValid());
    }

    bool read(Reader &reader, ContextButtonRecord &record)
    {
        reader.readU8(record.index);
        reader.readU8(record.count);
        reader.readString(record.label.text, sizeof(record.label.text));

        return (reader.isValid());
    }

    bool write(Writer &writer, const BankRecord &record)
    {
        writer.writeU8(record.index);
        writer.writeU8(record.count);
        writer.writeU24(record.colour);
        writer.writeString(record.name.text);

        return (writer.isValid());
    }

    bool read(Reader &reader, BankRecord &record)
    {
        reader.readU8(record.index);
        reader.readU8(record.count);
        reader.readU24(record.colour);
        reader.readString(record.name.text, sizeof(record.name.text));

        return (reader.isValid());
    }

    bool write(Writer &writer, const OverlayHeaderRecord &record)
    {
        writer.writeU8(record.id);
        writer.writeU8(record.sequence);
        writer.writeU8(record.final);
        writer.writeU8(record.count);

        return (writer.isValid());
    }

    bool read(Reader &reader, OverlayHeaderRecord &record)
    {
        reader.readU8(record.id);
        reader.readU8(record.sequence);
        reader.readU8(record.final);
        reader.readU8(record.count);

        return (reader.isValid());
    }

    bool write(Writer &writer, const OverlayItemRecord &record)
    {
        writer.writeI16(record.value);
        writer.writeU24(record.colour);
        writer.writeU8(record.colourIsSet);
        writer.writeString(record.label.text);

        return (writer.isValid());
    }

    bool read(Reader &reader, OverlayItemRecord &record)
    {
        reader.readI16(record.value);
        reader.readU24(record.colour);
        reader.readU8(record.colourIsSet);
        reader.readString(record.label.text, sizeof(record.label.text));

        return (reader.isValid());
    }

    bool write(Writer &writer, const ValueRecord &record)
    {
        writer.writeU8(record.handle);
        writer.writeU8(record.overlayId);
        writer.writeI16(record.min);
        writer.writeI16(record.max);
        writer.writeI16(record.current);
        writer.writeU8(record.flags);

        if ((record.flags & ValueFlag::HasLabel) != 0) {
            writer.writeString(record.label.text);
        }

        return (writer.isValid());
    }

    bool read(Reader &reader, ValueRecord &record)
    {
        reader.readU8(record.handle);
        reader.readU8(record.overlayId);
        reader.readI16(record.min);
        reader.readI16(record.max);
        reader.readI16(record.current);
        reader.readU8(record.flags);

        record.label.text[0] = '\0';

        if ((record.flags & ValueFlag::HasLabel) != 0) {
            reader.readString(record.label.text, sizeof(record.label.text));
        }

        return (reader.isValid());
    }

    bool write(Writer &writer, const SubscriptionUpdateRecord &record)
    {
        writer.writeU16(record.index);
        writer.writeU8(record.presetId);
        writer.writeU16(record.controlId);
        writer.writeString(record.name.text);

        if (!write(writer, record.value)) {
            return (false);
        }

        return (writer.isValid());
    }

    bool read(Reader &reader, SubscriptionUpdateRecord &record)
    {
        reader.readU16(record.index);
        reader.readU8(record.presetId);
        reader.readU16(record.controlId);
        reader.readString(record.name.text, sizeof(record.name.text));

        if (!read(reader, record.value)) {
            return (false);
        }

        return (reader.isValid());
    }

    bool write(Writer &writer, const InputRecord &record)
    {
        writer.writeU8((uint8_t)record.kind);
        writer.writeU8(record.flags);
        writer.writeU16(record.viewId);
        writer.writeU16(record.elementId);
        writer.writeU8(record.handle);
        writer.writeI16(record.a);
        writer.writeI16(record.b);

        return (writer.isValid());
    }

    bool read(Reader &reader, InputRecord &record)
    {
        uint8_t kind = 0;

        reader.readU8(kind);
        reader.readU8(record.flags);
        reader.readU16(record.viewId);
        reader.readU16(record.elementId);
        reader.readU8(record.handle);
        reader.readI16(record.a);
        reader.readI16(record.b);

        record.kind = (InputKind)kind;

        return (reader.isValid());
    }

    bool write(Writer &writer, const RequestRecord &record)
    {
        writer.writeU8((uint8_t)record.kind);
        writer.writeU8(record.argument);
        writer.writeU8(record.argument2);

        return (writer.isValid());
    }

    bool read(Reader &reader, RequestRecord &record)
    {
        uint8_t kind = 0;

        reader.readU8(kind);
        reader.readU8(record.argument);
        reader.readU8(record.argument2);

        record.kind = (RequestKind)kind;

        return (reader.isValid());
    }

    bool write(Writer &writer, const SubscribeHeaderRecord &record)
    {
        writer.writeU8(record.sequence);
        writer.writeU8(record.final);
        writer.writeU8(record.count);

        return (writer.isValid());
    }

    bool read(Reader &reader, SubscribeHeaderRecord &record)
    {
        reader.readU8(record.sequence);
        reader.readU8(record.final);
        reader.readU8(record.count);

        return (reader.isValid());
    }

    bool write(Writer &writer, const SubscriptionEntry &record)
    {
        writer.writeU8(record.presetId);
        writer.writeU16(record.controlId);
        writer.writeU8(record.handle);

        return (writer.isValid());
    }

    bool read(Reader &reader, SubscriptionEntry &record)
    {
        reader.readU8(record.presetId);
        reader.readU16(record.controlId);
        reader.readU8(record.handle);

        return (reader.isValid());
    }

    bool write(Writer &writer, const OverlayBitmapRecord &record)
    {
        if (record.length > maxOverlayBitmapBytes) {
            return (false);
        }

        writer.writeU8(record.overlayId);
        writer.writeI16(record.value);
        writer.writeU8(record.length);
        writer.writeBytes(record.bytes, record.length);

        return (writer.isValid());
    }

    bool read(Reader &reader, OverlayBitmapRecord &record)
    {
        reader.readU8(record.overlayId);
        reader.readI16(record.value);
        reader.readU8(record.length);

        if (record.length > maxOverlayBitmapBytes) {
            return (false);
        }

        reader.readBytes(record.bytes, record.length);

        return (reader.isValid());
    }

    bool write(Writer &writer, const PrimitivesHeaderRecord &record)
    {
        writer.writeU16(record.viewId);
        writer.writeU16(record.elementId);
        writer.writeU8(record.sequence);
        writer.writeU8(record.final);
        writer.writeU16(record.count);

        return (writer.isValid());
    }

    bool read(Reader &reader, PrimitivesHeaderRecord &record)
    {
        reader.readU16(record.viewId);
        reader.readU16(record.elementId);
        reader.readU8(record.sequence);
        reader.readU8(record.final);
        reader.readU16(record.count);

        return (reader.isValid());
    }

    bool write(Writer &writer, const GestureRecord &record)
    {
        writer.writeU16(record.viewId);
        writer.writeU16(record.elementId);
        writer.writeU8(record.kind);
        writer.writeU8(record.event);
        writer.writeU8(record.potId);
        writer.writeU8(record.touchId);
        writer.writeI16(record.x);
        writer.writeI16(record.y);
        writer.writeI16(record.downX);
        writer.writeI16(record.downY);
        writer.writeI16(record.delta);

        return (writer.isValid());
    }

    bool read(Reader &reader, GestureRecord &record)
    {
        reader.readU16(record.viewId);
        reader.readU16(record.elementId);
        reader.readU8(record.kind);
        reader.readU8(record.event);
        reader.readU8(record.potId);
        reader.readU8(record.touchId);
        reader.readI16(record.x);
        reader.readI16(record.y);
        reader.readI16(record.downX);
        reader.readI16(record.downY);
        reader.readI16(record.delta);

        return (reader.isValid());
    }

    int decodePayload(uint8_t *dest,
                      size_t destCapacity,
                      const char *encoded,
                      size_t encodedLength)
    {
        if (encodedLength == 0) {
            return (0);
        }

        if (encodedLength > maxEncodedLength) {
            return (-1);
        }

        /* base64_decode emits up to three bytes per full quad, up to two for
         * a trailing partial quad, and one NUL past the end. Scratch covers
         * that worst case for the longest input allowed through above. */
        uint8_t scratch[((maxEncodedLength / 4) * 3) + 3];

        int decodedLength =
            base64_decode((char *)scratch, (char *)encoded, (int)encodedLength);

        if ((decodedLength < 0) || ((size_t)decodedLength > destCapacity)) {
            return (-1);
        }

        memcpy(dest, scratch, decodedLength);

        return (decodedLength);
    }
} // namespace Link
