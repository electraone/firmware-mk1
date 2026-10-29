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
#include "VramStore.h"
#include <cstdint>
#include <cstring>
#include <string>

class Control;

class ControlValue
{
public:
    /* Twenty, as on the mk2. A list's values are words and fifteen cut
     * several of them; the label itself lives in the display's memory, so the
     * only thing this costs in main memory is the one shared buffer. */
    static constexpr int MaxLabelLength = 20;

    /** @brief The rate factor a value moves at unless it declares another. */
    static constexpr float DefaultSensitivity = 1.0f;

    /** @brief The range a preset may ask for. */
    static constexpr float MinSensitivity = 0.001f;
    static constexpr float MaxSensitivity = 100.0f;

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


    void setControl(Control *newControl);
    Control *getControl(void) const;
    /** The next value listening to the same parameter - see LookupEntry. */
    ControlValue *getNextDestination(void) const;
    void setNextDestination(ControlValue *newNext);
    ControlValue *&nextDestinationRef(void)
    {
        return (nextDestination);
    }
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

    /**
     * @brief How many distinct positions this value really has.
     *
     * A value lives in the parameter map as its MIDI value, so a display range
     * wider than the MIDI range - 100 to 8000 sent as a 7-bit CC - has only as
     * many positions as the MIDI range: 128 there, not 7901. A knob has to be
     * scaled to these, or it adds a fraction of a MIDI step per reading and the
     * round trip through the map rounds every one of them away.
     */
    uint16_t getPotSteps(void) const;

    /**
     * @brief Where a turn of @p steps takes a value, in the positions it has.
     *
     * @param displayValue the value as the component last drew it, used only
     *        while the parameter map holds none
     * @param steps the change, in the units getPotSteps() counts
     *
     * @return the new display value, not yet constrained to the range
     */
    int16_t turnedValue(int16_t displayValue, int16_t steps) const;
    bool isFunctionAssigned(void) const;
    bool isFormatterAssigned(void) const;
    bool hasLua(void) const;
    const char *getFunction(void) const;
    const std::string getFormatter(void) const;
    void setLabel(const char *newLabel);
    void resetLabel(void);

    /**
     * @brief The label, fetched from the display's memory.
     *
     * @attention the returned pointer is a shared buffer that the next call
     * overwrites. Every caller copies it straight into its own storage, and
     * anyone adding a caller must do the same.
     */
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
     * @brief How far a knob turns to move this value.
     *
     * A factor on the usual rate, which is a whole range per POT_SWEEP_DEGREES
     * of travel: 0.1 is a tenth of that, so ten sweeps end to end, and 10 is ten
     * times it. Clamped to MinSensitivity .. MaxSensitivity; anything that is
     * not a positive number is the default.
     *
     * @attention kept to a single byte, and the byte is one the bitfields above
     * already waste on alignment - so this costs nothing per value, and there
     * are a great many values. A byte cannot hold a float, so what is stored is
     * a mantissa of five bits and a power of ten of three: every figure a preset
     * is likely to write - 0.001, 0.05, 0.25, 0.3, 2, 100 - lands exactly, and
     * one that does not is rounded to four percent or so, which is far below
     * what a hand can feel. Nothing reads it back out as JSON on this model, so
     * there is no round trip for the rounding to show up in.
     */
    void setSensitivity(float newSensitivity);
    float getSensitivity(void) const;

    /** @brief Whether the value declares a sensitivity of its own. */
    bool hasSensitivity(void) const;

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

        /* Whether the label record holds anything. Kept here rather than
         * asked of the store, because a painter tests this before deciding
         * to fetch and the test must not itself cost a transfer. */
        bool labelSet : 1;
    };

    /* The knob rate factor, encoded - see setSensitivity(). Zero means the
     * value declares none and moves at the usual rate. */
    uint8_t sensitivity;

    int16_t defaultValue;
    int16_t min;
    int16_t max;
    uint8_t formatter;
    uint8_t function;
    Control *control;
    Overlay *overlay;
    /* A parameter map entry lists the values it drives by threading them
     * through this field, instead of keeping a vector of pointers to them.
     * A value listens to exactly one parameter, so one link is enough. */
    ControlValue *nextDestination;
    /** The label lives in the display's memory. The space is taken the
     *  first time a label is set, and rewritten in place after that, because
     *  a Lua formatter rewrites it on every value change. */
    VramStore::Handle labelHandle;

    int16_t value;

    /** Shared scratch the label is handed out through. See getLabel(). */
    static char labelBuffer[MaxLabelLength + 1];

public:
    Message message;
};
