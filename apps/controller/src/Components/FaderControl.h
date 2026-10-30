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
 * @file FaderControl.h
 *
 * @brief Implements a horizontal fader-style type of Control.
 *
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"
#include "BarHorizontal.h"
#include "AssignableList.h"

class FaderControl : public ControlComponent, public BarHorizontal
{
public:
    FaderControl(const Control &control, MainDelegate &newDelegate);
    virtual ~FaderControl() = default;

    virtual void syncComponentProperties(void) override;
    virtual void onTouchDown(const TouchEvent &touchEvent) override;
    virtual void onTouchMove(const TouchEvent &touchEvent) override;
    virtual void onPotTouchDown(const PotEvent &potEvent) override;
    virtual void onPotChange(const PotEvent &potEvent) override;
    virtual void onPotTouchUp(const PotEvent &potEvent) override;
    virtual void onMidiValueChange(const ControlValue &value,
                                   int16_t midiValue,
                                   uint8_t handle = 0) override;
    void paint(Graphics &g) override;

    AssignableList list;

private:
    void paintDial(Graphics &g,
                   const Rectangle &bounds,
                   uint32_t colour,
                   int16_t min,
                   int16_t max,
                   int16_t val,
                   const ListData *items,
                   Control::Mode mode);

    void paintFader(Graphics &g,
                    const Rectangle &bounds,
                    uint32_t colour,
                    int16_t min,
                    int16_t max,
                    int16_t val,
                    const ListData *items,
                    Control::Mode mode,
                    Control::Variant variant);

    /**
     * @brief The value above the track, centred.
     *
     * `graphicsTop` is what it stands on - the top of the track, or the
     * height of the knob for a dial, whose value goes in the hole in the ring
     * instead.
     */
    void paintValueFixed(Graphics &g,
                         const Rectangle &bounds,
                         uint32_t colour,
                         int16_t val,
                         const ListData *items,
                         Control::Variant variant,
                         uint16_t graphicsTop);

    /**
     * @brief The value inside the bar, right aligned short of its end.
     *
     * Centred on the track's height by the digits' ink, and split where the
     * bar ends: over the filled part in a colour that reads on the fader's
     * own colour, over the track in one that reads on the track - through the
     * middle of a glyph if that is where the end of the bar is.
     */
    void paintValueFloating(Graphics &g,
                            const Rectangle &bounds,
                            uint32_t colour,
                            int16_t barX,
                            int16_t barWidth,
                            uint16_t trackY,
                            uint16_t trackHeight,
                            int16_t val,
                            const ListData *items);

    /**
     * @brief What the control has to say about its value, as text.
     *
     * Answers false when the item carries a bitmap instead, which the caller
     * paints rather than prints.
     */
    bool resolveValueText(char *text,
                          size_t size,
                          int16_t val,
                          const ListDataItem &item,
                          const ListData *items,
                          bool padNegative) const;

    struct {
        uint16_t previousScreenX : 10;
        bool thresholdCrossed : 1;
    };

    /** How far a full-size fader's number ends short of the right edge.
     *  v3.5.4 measured every string as sixteen pixels past the start of its
     *  last character, so a number - whose digits advance seven - stopped
     *  nine short of the edge; measuring exactly put it against the edge. */
    static constexpr uint16_t textPositionFromRight = 9;
};
