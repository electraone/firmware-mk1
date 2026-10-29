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

#include "HeldNotes.h"

/* The controllers that end notes by definition. */
static bool releasesNotes(uint8_t controller)
{
    return ((controller == 120) || (controller >= 123));
}

HeldNotes::HeldNotes() : notes{}, count(0)
{
}

bool HeldNotes::process(MidiInterface::Type interface,
                        uint8_t port,
                        MidiMessage::Type type,
                        uint8_t channel,
                        uint8_t data1,
                        uint8_t data2,
                        uint32_t nowMs)
{
    const uint8_t input = (uint8_t)interface;

    switch (type) {
        case MidiMessage::Type::NoteOn:
            if (data2 > 0) {
                return (press(Note{ .interface = input,
                                    .port = port,
                                    .channel = channel,
                                    .note = data1,
                                    .velocity = data2,
                                    .timeMs = nowMs }));
            }
            return (release(input, port, channel, data1));

        case MidiMessage::Type::NoteOff:
            return (release(input, port, channel, data1));

        case MidiMessage::Type::ControlChange:
            return (releasesNotes(data1)
                    && releaseChannel(input, port, channel));

        case MidiMessage::Type::SystemReset:
            return (releaseInput(input, port));

        default:
            break;
    }

    return (false);
}

uint8_t HeldNotes::size(void) const
{
    return (count);
}

const HeldNotes::Note &HeldNotes::at(uint8_t index) const
{
    return (notes[index]);
}

void HeldNotes::clear(void)
{
    count = 0;
}

int16_t HeldNotes::find(uint8_t interface,
                        uint8_t port,
                        uint8_t channel,
                        uint8_t note) const
{
    for (uint8_t i = 0; i < count; i++) {
        const Note &held = notes[i];

        if ((held.note == note) && (held.channel == channel)
            && (held.port == port) && (held.interface == interface)) {
            return ((int16_t)i);
        }
    }

    return (-1);
}

void HeldNotes::removeAt(uint8_t index)
{
    for (uint8_t i = index; (i + 1) < count; i++) {
        notes[i] = notes[i + 1];
    }

    count--;
}

/* Pressed again without a release in between is a press all the same, and the
 * newest, so it moves to the end. A note past Capacity is dropped. */
bool HeldNotes::press(const Note &note)
{
    const int16_t index =
        find(note.interface, note.port, note.channel, note.note);

    if (index >= 0) {
        removeAt((uint8_t)index);
    } else if (count >= Capacity) {
        return (false);
    }

    notes[count++] = note;

    return (true);
}

bool HeldNotes::release(uint8_t interface,
                        uint8_t port,
                        uint8_t channel,
                        uint8_t note)
{
    const int16_t index = find(interface, port, channel, note);

    if (index < 0) {
        return (false);
    }

    removeAt((uint8_t)index);

    return (true);
}

bool HeldNotes::releaseChannel(uint8_t interface, uint8_t port, uint8_t channel)
{
    bool released = false;

    for (uint8_t i = count; i > 0; i--) {
        const Note &held = notes[i - 1];

        if ((held.channel == channel) && (held.port == port)
            && (held.interface == interface)) {
            removeAt((uint8_t)(i - 1));
            released = true;
        }
    }

    return (released);
}

bool HeldNotes::releaseInput(uint8_t interface, uint8_t port)
{
    bool released = false;

    for (uint8_t i = count; i > 0; i--) {
        const Note &held = notes[i - 1];

        if ((held.port == port) && (held.interface == interface)) {
            removeAt((uint8_t)(i - 1));
            released = true;
        }
    }

    return (released);
}
