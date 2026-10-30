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
 * @file PadControl.h
 *
 * @brief Implements a on-screen button for Controls.
 */

#pragma once

#include "Control.h"
#include "ControlComponent.h"
#include "Pad.h"

class PadControl : public ControlComponent, public Pad
{
public:
    PadControl(const Control &control, MainDelegate &newDelegate);
    virtual ~PadControl() override;

    virtual void onTouchDown(const TouchEvent &touchEvent) override;
    virtual void onTouchUp(const TouchEvent &touchEvent) override;
    virtual void onPotTouchDown(const PotEvent &potEvent) override;
    virtual void onPotChange(const PotEvent &potEvent) override;
    virtual void onPotTouchUp(const PotEvent &potEvent) override;
    void onTouchLongHold(const TouchEvent &touchEvent) override;

    virtual void onMidiValueChange(const ControlValue &value,
                                   int16_t midiValue,
                                   uint8_t handle = 0) override;

    void paint(Graphics &g) override;

    void emitValueChange(int16_t newDisplayValue,
                         const ControlValue &cv) override;

private:
    bool getState(void) const;
    void setState(bool newState);
    void emitOffValue(void);

    bool state;
};
