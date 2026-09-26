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
 * @file VerticalFader4Control.h
 *
 * @brief Implements an on-screen Control of four vertical faders.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"
#include "MultiValue.h"

/**
 * @brief Four vertical bars side by side, one per value.
 *
 * The knob turns whichever of the four the control's first input names, and
 * that one is outlined while a finger is on the knob.
 */
class VerticalFader4Control : public ControlComponent, public MultiValue
{
public:
    VerticalFader4Control(const Control &control, MainDelegate &newDelegate);
    virtual ~VerticalFader4Control() = default;

    void syncComponentProperties(void) override;

    void onTouchDown(const TouchEvent &touchEvent) override;
    void onTouchMove(const TouchEvent &touchEvent) override;
    void onTouchUp(const TouchEvent &touchEvent) override;
    void onPotTouchDown(const PotEvent &potEvent) override;
    void onPotChange(const PotEvent &potEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;
    void onMidiValueChange(const ControlValue &value,
                           int16_t midiValue,
                           uint8_t handle = 0) override;

    uint8_t getNumHandles(void) const override;
    void paint(Graphics &g) override;

    /**
     * @brief Where the middle of one bar sits, in the control's own
     *        coordinates.
     *
     * So that a detail window can put a dial under each bar rather than at
     * four numbers that happen to look about right.
     */
    uint16_t barCentreX(uint8_t index) const;

    static constexpr uint8_t numHandles = 4;

protected:
    /** How tall the bars are drawn. A control on a page leaves room for its
     *  name underneath; a detail uses the whole window. */
    virtual uint16_t getBarsHeight(void) const;

    /** The name and the active underline, which a detail does not draw. */
    virtual void paintDecorations(Graphics &g);

    /** Where the bars sit across the control, as fractions of one slot. */
    virtual float getBarWidthFraction(void) const;
    virtual float getBarGapFraction(void) const;

    void setValueRanges(void);

private:
    /** Where the row of bars starts and how it is spaced. */
    struct BarLayout {
        uint16_t rowX;
        uint16_t barWidth;
        uint16_t gap;
    };

    BarLayout barLayout(void) const;
};
