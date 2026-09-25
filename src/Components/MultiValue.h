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
 * @file MultiValue.h
 *
 * @brief A base class for multi-value components.
 */

#pragma once

#include "Component.h"
#include "Value.h"
#include <vector>

class MultiValue : virtual public Component
{
public:
    MultiValue();
    virtual ~MultiValue() = default;

    virtual void setValue(uint8_t handle, float newValue);
    virtual void applyValue(uint8_t handle, float delta);
    virtual float getValue(uint8_t handle);
    virtual void setMin(uint8_t handle, float newMin);
    virtual void setMax(uint8_t handle, float newMax);

    /**
     * @brief Tells whether the component owns the given handle.
     *
     * @param handle a handle to check
     * @return true when the handle addresses one of the component's values
     */
    bool hasHandle(uint8_t handle) const;

    void setActiveSegment(uint8_t newActiveSegment);
    void switchToNextActiveHandle(void);
    void switchToPreviousActiveHandle(void);
    uint8_t getActiveSegment(void) const;
    void showActiveSegment(bool shouldBeShown);

    virtual uint8_t getNumHandles(void) const = 0;
    virtual void paint(Graphics &g) override = 0;

protected:
    struct {
        uint8_t activeSegment : 6;
        bool activeSegmentIsShown : 1;
    };

    std::vector<Value> values;
};
