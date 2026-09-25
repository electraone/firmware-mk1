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
 * @file ControlValue.h
 *
 * @brief Implements a representation of a Display value in the Control
 */

#pragma once

#include "Message.h"
#include "Overlay.h"
#include "Message.h"
#include "Macros.h"
#include "luaHooks.h"
#include <cstdint>
#include <cstring>
#include <string>

class Control;

class ControlValue
{
public:
    static constexpr int MaxLabelLength = 15;

    ControlValue();
    ControlValue(Control *newControl,
                 const char *newValueId,
                 uint8_t newIndex,
                 int16_t newDefaultValue,
                 int16_t newMin,
                 int16_t newMax,
                 uint8_t newOverlayId,
                 Message(newMessage),
                 uint8_t newFormatter,
                 uint8_t newFunction,
                 Overlay *newOverlay);

    virtual ~ControlValue() = default;

    void setControl(Control *newControl);
    Control *getControl(void) const;
    int16_t getHandle(void) const;
    int16_t getIndex(void) const;
    const char *getId(void) const;
    void setDefault(int16_t newDefaultValue);
    int16_t getDefault(void) const;
    void setMin(int16_t newMin);
    int16_t getMin(void) const;
    void setMax(int16_t newMax);
    int16_t getMax(void) const;
    void setOverlayId(uint8_t newOverlayId);
    uint8_t getOverlayId(void) const;
    void setOverlay(Overlay *newOverlay);
    Overlay *getOverlay(void) const;
    uint16_t getNumSteps(void) const;
    bool isFunctionAssigned(void) const;
    bool isFormatterAssigned(void) const;
    bool hasLua(void) const;
    const char *getFunction(void) const;
    const std::string getFormatter(void) const;
    void setLabel(const char *newLabel);
    void resetLabel(void);
    const char *getLabel(void) const;
    bool isLabelSet(void) const;
    void setValue(int16_t newValue);
    int16_t getValue(void) const;
    void callFormatter(int16_t value);
    void callFunction(int16_t value) const;
    const char *translateId(uint8_t id) const;
    uint8_t translateId(const char *handle) const;
    void print(uint8_t logLevel = LOG_TRACE) const;

    int16_t translateMidiValue(uint16_t midiValue) const;

    /**
     * @brief Sets the flag that Control is using relative MIDI messages
     * 
     * @param shouldBeRelative use true when relative messages should be used
     */
    bool isRelative(void) const;

    /**
     * @brief Sets the flag that Control is using acceleration
     * 
     * @param shouldBeAccelerated use true when acceleration should be used
     */
    bool isAccelerated(void) const;

    /**
     * @brief Calculates the relative delta value
     * 
     * The acceleration settings affects the delta value.
     * 
     * @param value the value to calculate the delta from
     * @return int16_t the delta value
     */
    int16_t calculateRelativeDelta(int16_t value) const;

    static int16_t sign(int16_t value);
    static int16_t invertedSign(int16_t value);

private:
    struct {
        uint8_t handle : 4;
        uint8_t index : 4;
        uint8_t overlayId : 8;
        bool relative : 1;
        bool accelerated : 1;
    };

    int16_t defaultValue;
    int16_t min;
    int16_t max;
    uint8_t formatter;
    uint8_t function;
    Control *control;
    Overlay *overlay;
    char label[MaxLabelLength + 1];
    int16_t value;

public:
    Message message;
};
