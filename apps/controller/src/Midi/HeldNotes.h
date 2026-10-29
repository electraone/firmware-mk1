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
 * @file HeldNotes.h
 *
 * @brief Which notes are held down, on every input and channel.
 */

#pragma once

#include "MidiInterface.h"
#include "MidiMessage.h"
#include <cstdint>

/**
 * @brief The keys that are down right now, every input and channel together.
 *
 * Kept whether or not a preset asked, as the mk2's NoteTracker is: a script
 * that starts waiting for a chord has to be told about the keys already held,
 * not only the ones pressed after it began.
 *
 * The mk2 keeps two copies and a ring between them, because its MIDI arrives
 * on a thread of its own and its scripts run on another. Here both happen in
 * the main loop - processMidi() hands each message over in the order it came -
 * so one set, changed message by message, is already the view a script needs:
 * a drum pad's note on is seen with the note held and its note off after.
 *
 * What releases a note: a Note Off, a Note On with velocity 0, and on its
 * channel All Sound Off (120), All Notes Off (123) and the mode messages (124
 * to 127), which the MIDI spec says imply All Notes Off. A System Reset
 * releases everything the input was holding. The sustain pedal does not:
 * these are the keys that are down.
 */
class HeldNotes
{
public:
    /** @brief Notes held at once, every input and channel together. */
    static constexpr uint8_t Capacity = 128;

    struct Note {
        uint8_t interface; /**< MidiInterface::Type */
        uint8_t port;
        uint8_t channel; /**< 1 to 16 */
        uint8_t note;
        uint8_t velocity;
        uint32_t timeMs; /**< when it was pressed, on millis() */
    };

    HeldNotes();

    /**
     * @brief One incoming MIDI message. Anything that is not a note, a
     *        note-ending controller or a System Reset is ignored.
     *
     * @returns whether the held notes changed.
     */
    bool process(MidiInterface::Type interface,
                 uint8_t port,
                 MidiMessage::Type type,
                 uint8_t channel,
                 uint8_t data1,
                 uint8_t data2,
                 uint32_t nowMs);

    uint8_t size(void) const;

    /** @brief The held notes, in the order they were pressed. */
    const Note &at(uint8_t index) const;

    void clear(void);

private:
    int16_t find(uint8_t interface,
                 uint8_t port,
                 uint8_t channel,
                 uint8_t note) const;
    void removeAt(uint8_t index);
    bool press(const Note &note);
    bool
        release(uint8_t interface, uint8_t port, uint8_t channel, uint8_t note);
    bool releaseChannel(uint8_t interface, uint8_t port, uint8_t channel);
    bool releaseInput(uint8_t interface, uint8_t port);

    Note notes[Capacity];
    uint8_t count;
};
