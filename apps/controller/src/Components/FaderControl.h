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
    void paintFader(Graphics &g,
                    const Rectangle &bounds,
                    uint32_t colour,
                    int16_t min,
                    int16_t max,
                    int16_t val,
                    const ListData *items,
                    Control::Mode mode,
                    Control::Variant variant);

    void paintValueFixed(Graphics &g,
                         const Rectangle &bounds,
                         uint32_t colour,
                         int16_t val,
                         const ListData *items,
                         Control::Mode mode,
                         Control::Variant variant);

    void paintValueFloating(Graphics &g,
                            const Rectangle &bounds,
                            uint32_t colour,
                            uint16_t x,
                            int16_t val,
                            const ListData *items,
                            Control::Mode mode,
                            Control::Variant variant);
    bool isColorTooBright(uint16_t color, float brightnessThreshold);

    struct {
        uint16_t previousScreenX : 10;
        bool thresholdCrossed : 1;
    };

    static constexpr uint16_t textPositionFromRight = 0;
};
