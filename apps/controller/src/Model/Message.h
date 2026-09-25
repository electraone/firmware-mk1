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
 * @file Message.h
 *
 * @brief Implements a representation of Messages processed by the
 * controller. Message is not limited to MIDI messages only.
 */

#pragma once

#include "SignMode.h"
#include "RelativeMode.h"
#include "Macros.h"
#include "System.h"
#include <cstdint>

class ControlValue;

#define MIDI_VALUE_DO_NOT_SEND 16537
#define VARIABLE_DATA 128
#define VARIABLE_PARAMETER 129
#define VARIABLE_END 130
#define CHECKSUM 131
#define LUAFUNCTION 132

class Message
{
public:
    enum Type {
        virt = 0,
        cc7 = 1,
        cc14 = 2,
        nrpn = 3,
        rpn = 4,
        note = 5,
        program = 6,
        sysex = 7,
        start = 8,
        stop = 9,
        tune = 10,
        atpoly = 11,
        atchannel = 12,
        pitchbend = 13,
        spp = 14,
        relcc = 15,
        none = 16
    };

    Message();
    Message(uint8_t newDeviceId,
            Type newType,
            uint16_t newParameterNumber,
            uint16_t newMidiMin,
            uint16_t newMidiMax,
            uint16_t newValue,
            std::vector<uint8_t> *newData,
            bool newLsbFirst,
            bool newResetRpn,
            SignMode newSignMode,
            uint8_t newBitWidth,
            bool newRelative,
            RelativeMode newRelativeMode,
            bool newAccelerated);
    virtual ~Message() = default;

    /**
     * @brief  Sets the device id.
     * 
     * @param newDeviceId a new device id to be assigned.
     */
    void setDeviceId(uint8_t newDeviceId);

    /**
     * @brief  Returns the device id.
     * 
     * @return the device id.
     */
    uint8_t getDeviceId(void) const;

    /**
     * @brief  Sets the message type.
     * 
     * @param newType a new message type to be assigned.
     */
    void setType(Type newType);

    /**
     * @brief  Returns the message type.
     * 
     * @return the message type.
     */
    Type getType(void) const;

    /**
     * @brief  Sets the MIDI min value.
     * 
     * @param newMin a new MIDI min value to be assigned.
     */
    void setMidiMin(uint16_t newMin);

    /**
     * @brief  Returns the MIDI min value.
     * 
     * @return the MIDI min value.
     */
    uint16_t getMidiMin(void) const;

    /**
     * @brief  Sets the MIDI max value.
     * 
     * @param newMax a new MIDI max value to be assigned.
     */
    void setMidiMax(uint16_t newMax);

    /**
     * @brief  Returns the MIDI max value.
     * 
     * @return the MIDI max value.
     */
    uint16_t getMidiMax(void) const;

    /**
     * @brief  Sets the MIDI Off (for switches/pads) value.
     * 
     * @param newMin a new MIDI Off value to be assigned.
     */
    void setMidiOff(uint16_t newMin);

    /**
     * @brief  Returns the MIDI Off (for switches/pads) value.
     * 
     * @return the MIDI Off value.
     */
    uint16_t getMidiOff(void) const;

    /**
     * @brief  Sets the MIDI On (for switches/pads) value.
     * 
     * @param newMax a new MIDI On value to be assigned.
     */
    void setMidiOn(uint16_t newMax);

    /**
     * @brief  Returns the MIDI On (for switches/pads) value.
     * 
     * @return the MIDI On value.
     */
    uint16_t getMidiOn(void) const;

    /**
     * @brief  Sets the parameter number.
     * 
     * @param newParameterNumber a new parameter number to be assigned.
     */
    void setParameterNumber(uint16_t newParameterNumber);

    /**
     * @brief  Returns the parameter number.
     * 
     * @return the parameter number.
     */
    uint16_t getParameterNumber(void) const;

    /**
     * @brief  Sets the value.
     * 
     * @param newValue a new value to be assigned.
     */
    void setValue(uint16_t newValue);

    /**
     * @brief  Marks value as empty / unset.
     */
    void setEmptyValue(void);

    /**
     * @brief  Returns true if the value is empty / unset.
     * 
     * @return true if the value is empty / unset.
     */
    bool hasEmptyValue(void) const;

    /**
     * @brief  Returns true if the value is not empty / unset.
     * 
     * @return true if the value is not empty / unset.
     */
    bool hasValidValue(void) const;

    /**
     * @brief  Returns the value.
     * 
     * @return the value.
     */
    uint16_t getValue(void) const;

    /**
     * @brief  Sets the sign mode.
     * 
     * @param newSignMode a new sign mode to be assigned.
     */
    void setSignMode(SignMode newSignMode);

    /**
     * @brief  Returns the sign mode.
     * 
     * @return the sign mode.
     */
    SignMode getSignMode(void) const;

    /**
     * @brief  Sets the Off value for a Pad
     * 
     * @param newOffValue a new off value to be assigned.
     */
    void setOffValue(int16_t newOffValue);

    /**
     * @brief  Returns the Off value for a Pad
     * 
     * @return the Off value for a Pad
     */
    int16_t getOffValue(void) const;

    /**
     * @brief  Sets the On value for a Pad
     * 
     * @param newOnValue a new on value to be assigned.
     */
    void setOnValue(int16_t newOnValue);

    /**
     * @brief  Returns the On value for a Pad
     * 
     * @return the On value for a Pad
     */
    int16_t getOnValue(void) const;

    /**
     * @brief Sets the order of the bytes in the message.
     * 
     * @param newLsbFirst true for LSB first, false for MSB first.
     */
    void setLsbFirst(bool newLsbFirst);

    /**
     * @brief Returns the order of the bytes in the message.
     * 
     * @return true for LSB first, false for MSB first.
     */
    bool getLsbFirst(void) const;

    /**
     * @brief Sets the flag that RPN should be reset before sending the message.
     * 
     * When RPN is reset, the controller sends RPN reset message after each
     * RPN/NRPN message.
     * 
     * @param newResetRpn use true when RPN should be reset
     */
    void setResetRpn(bool newResetRpn);

    /**
     * @brief Returns the flag that RPN should be reset before sending the message.
     * 
     * @return true when RPN should be reset
     */
    bool getResetRpn(void) const;

    /**
     * @brief Sets the bit width of the message.
     * 
     * @param newBitWidth the bit width of the message
     */
    void setBitWidth(uint8_t newBitWidth);

    /**
     * @brief Returns the bit width of the message.
     * 
     * @return the bit width of the message
     */
    uint8_t getBitWidth(void) const;

    /**
     * @brief Forces to send relative CC messages.
     * 
     * @param shouldBeRelative true when the message should be sent as relative
     */
    void setRelative(uint8_t shouldBeRelative);

    /**
     * @brief Returns true when the message should be sent as relative.
     * 
     * @return true when the message should be sent as relative
     */
    uint8_t isRelative(void) const;

    /**
     * @brief Sets the relative mode of the message.
     * 
     * @param newRelativeMode the relative mode of the message
     */
    void setRelativeMode(RelativeMode newRelativeMode);

    /**
     * @brief Returns the relative mode of the message.
     * 
     * @return the relative mode of the message
     */
    RelativeMode getRelativeMode(void) const;

    /**
     * @brief Sets whether or not relative messages should
     *  support acceleration.
     * 
     * @param shouldBeAccelerated true when the message should be sent as accelerated
     */
    void setAccelerated(uint8_t shouldBeAccelerated);

    /**
     * @brief Returns true when the message should be sent as accelerated.
     * 
     * @return true when the message should be sent as accelerated
     */
    uint8_t isAccelerated(void) const;

    /**
     * @brief Sets the ControlValue (parent) of the message.
     * 
     * @param newControlValue pointer to the ControlValue
     */
    void setControlValue(ControlValue *newControlValue);

    /**
     * @brief Returns the ControlValue (parent) of the message.
     * 
     * @return pointer to the ControlValue
     */
    ControlValue *getControlValue(void) const;

    /**
     * @brief  Prints the message to the logger output.
     * 
     * @param logLevel 
     */
    void print(uint8_t logLevel = LOG_TRACE) const;

    /**
     * @brief  Translate a text string to a message type.
     * 
     * @return message type or none if the text is not recognized.
     */
    static Type translateType(const char *typeText);

    /**
     * @brief  Translate a message type to a text string.
     * 
     * @return text string or "none" if the type is not recognized.
     */
    static const char *translateType(Type messageType);

    std::vector<uint8_t> *data;

private:
    uint16_t parameterNumber;
    uint16_t value;
    uint16_t midiMin;
    uint16_t midiMax;
    ControlValue *controlValue;

    struct {
        uint8_t deviceId : 6;
        uint8_t type : 5;
        bool lsbFirst : 1;
        bool resetRpn : 1;
        uint8_t signMode : 3;
        uint8_t bitWidth : 4;
        bool relative : 1;
        uint8_t relativeMode : 2;
        bool accelerated : 1;
    };
};

typedef std::map<uint8_t, std::vector<uint8_t>> SysexMessages;
