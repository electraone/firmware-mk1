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
 * @file TextBoxControl.h
 *
 * @brief Draws a Control whose value is text rather than a number.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"

/**
 * @brief A box showing the text its parameter carries.
 *
 * It owns no text of its own. What it draws is the value's label - the same
 * override a fader prints instead of its number - which is what lets a patch
 * name arrive from a script or from the SysEx API and reach the screen by the
 * same route in both cases.
 *
 * Which face it draws in comes from the Control's font. A monospaced face is
 * what makes a column of patch names line up.
 *
 * Three shapes, chosen by the variant, which is what a variant is for now
 * that the face has a home of its own:
 *
 *   default    an outlined box with the control's name beneath it
 *   valueOnly  the text and nothing else - no outline, no name, the whole
 *              height to itself
 *   baseline   no outline, the name beneath, and the text standing on the
 *              line every other control's value stands on - a list's, a thin
 *              fader's - so a text box in a row of them reads as one of the
 *              row rather than as a box among them
 *
 * Nothing on this controller edits it. The detail window that opens the
 * on-screen keyboard is a separate piece of work, as it is on the mk2, and
 * until it lands a press does nothing.
 */
class TextBoxControl : public ControlComponent
{
public:
    TextBoxControl(const Control &control, MainDelegate &newDelegate);
    virtual ~TextBoxControl() = default;

    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

    void onTouchLongHold(const TouchEvent &touchEvent) override;

    void paint(Graphics &g) override;

private:
    /** Whether this control draws neither its outline nor its name. */
    bool isValueOnly(void) const;

    /** Whether it draws no outline and puts its text on the shared value
     *  line. */
    bool isBaseline(void) const;

    /** The top of the strip a list would draw under its value, which is what
     *  the shared value line is measured from. */
    uint16_t valueStripTop(void) const;

    /** What is drawn when the parameter carries no text. */
    static constexpr const char *emptyPlaceholder = "-";

    /** Left and right padding inside the box. */
    static constexpr uint8_t textMargin = 8;
};
