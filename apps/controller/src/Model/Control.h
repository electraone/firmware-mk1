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
 * @file Control.h
 *
 * @brief Implements a representation of an on-screen value controller
 */

#pragma once

#include <vector>
#include <map>

#include "ControlValue.h"
#include "Input.h"
#include "Rectangle.h"
#include "Colours.h"
#include "TextBase.h"
#include "Component.h"

extern const char *valueIdsAdsr[];
extern const char *valueIdsAdr[];
extern const char *valueIdsDx7Env[];

class Control
{
public:
    enum class Type {
        None = 0,
        Fader = 1,
        Vfader = 2,
        List = 3,
        Pad = 4,
        Adsr = 5,
        Adr = 6,
        Dx7envelope = 7,
        Knob = 8,
        Relative = 9,
        Custom = 10,
        Ar = 11,
        Adssr = 12,
        Ahdsr = 13,
        XyPad = 15,

        /**
         * A box showing text rather than a number.
         *
         * The text is the parameter's override, so a script - or a synth's
         * patch dump, parsed by one - writes it the same way it would write
         * one onto a fader. The control carries a value like any other so
         * that it has a parameter to hang the text on.
         */
        TextBox = 17
    };

    enum class Mode {
        Default = 0,
        Momentary = 1,
        Toggle = 2,
        Unipolar = 3,
        Bipolar = 4
    };

    /* Numbered as the mk2 numbers them, gaps and all, so that the two read
     * the same preset the same way. The enumerators it carries and this does
     * not belong to controls this has not been given yet. */
    enum class Variant {
        Default = 0,
        Thin = 2,
        ValueOnly = 3,
        Dial = 4,
        Checkbox = 5,
        /* A text box's third shape: no outline, the name beneath, and the
         * text standing on the line every other control's value stands on. */
        Baseline = 7
    };

    /**
     * @brief The face a control draws its text in.
     *
     * Three independent choices - four sizes, two weights, two spacings - so
     * a triple rather than one more Variant enumerator, which could only ever
     * carry one of them.
     *
     * It costs four bits, and they were already there: the bitfield below had
     * eleven going spare inside the words it occupies. A Control is held for
     * every control of every preset, so the alternative - three bytes, or a
     * struct - would have been paid a hundred and thirty times over on the
     * preset this was measured against.
     */
    using Font = TextFont;

    Control();
    Control(uint16_t id,
            uint8_t pageId,
            const char *name,
            const Rectangle &bounds,
            Type type,
            Mode mode,
            uint32_t colour,
            uint8_t controlSetId,
            Variant variant,
            bool visible,
            const Font &font = Font());

    virtual ~Control() = default;

    bool isValid(void) const;
    void setId(uint16_t newId);
    uint16_t getId(void) const;
    void setPageId(uint8_t newPageId);
    uint8_t getPageId(void) const;
    void setType(Type newType);
    Type getType(void) const;
    Mode getMode(void) const;
    void setColour(uint32_t newColour);
    uint32_t getColour(void) const;
    uint16_t getColour565(void) const;
    void setName(const char *newName);
    const char *getName(void) const;
    void setControlSetId(uint8_t newControlSetId);
    uint8_t getControlSetId(void) const;
    Variant getVariant(void) const;
    void setVariant(Variant newVariant);
    void setVisible(bool shouldBeVisible);
    bool isVisible(void) const;

    void setFont(const Font &newFont);
    Font getFont(void) const;
    Rectangle getBounds(void) const;
    void setBounds(const Rectangle &bounds);
    void setValues(std::vector<ControlValue> values);
    ControlValue &getValue(uint8_t index);
    const ControlValue &getValue(uint8_t index) const;
    const ControlValue &getValueByValueId(const char *valueId) const;
    bool hasDetail(void) const;

    static Type translateType(const char *typeText);
    static Mode translateControlMode(const char *modeText);
    static Variant translateVariant(const char *variantText);
    static TextSize translateFontSize(const char *text);
    static TextFace translateFontFace(const char *text);
    static TextSpacing translateFontSpacing(const char *text);

    const char *translateValueId(uint8_t id) const;
    uint8_t translateValueId(const char *handle) const;

    void setComponent(Component *newComponent);
    void resetComponent(void);
    Component *getComponent(void) const;
    void addToParameterMap(ControlValue &value);
    void removeFromParameterMap(ControlValue &value);
    void setDefaultValue(ControlValue &value, bool sendMidiMessages);
    void print(uint8_t logLevel = LOG_TRACE) const;

    static uint8_t translateValueToId(Control::Type controlType,
                                      const char *handle);
    static uint8_t constraintValueId(Control::Type controlType,
                                     uint8_t handleId);

    static const int MaxNameLength = 15;

private:
    struct {
        uint16_t id : 10;
        uint8_t pageId : 4;
        /* Five bits, not four: the type enumerators are the mk2's, and they
         * run to seventeen. A text box truncated to one, which is a fader -
         * so a preset carrying one drew a row of faders and nothing said
         * why. */
        uint8_t type : 5;
        uint8_t mode : 3;
        uint8_t controlSetId : 3;
        uint8_t variant : 4;
        uint8_t visible : 1;
        /* The face, in the padding the fields above leave behind. */
        uint8_t fontSize : 2;
        uint8_t fontFace : 1;
        uint8_t fontSpacing : 1;
        uint32_t colour : 24;
    };

    void printValues(uint8_t logLevel = LOG_TRACE) const;
    void printInputs(uint8_t logLevel = LOG_TRACE) const;

    Rectangle bounds;
    char name[MaxNameLength + 1];

    Component *component;

public:
    std::vector<ControlValue> values;
    std::vector<Input> inputs;
};

typedef std::map<uint16_t, Control> Controls;
