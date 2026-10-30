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
 * @file ControlComponent.h
 *
 * @brief Implements a base class for all on-screen Controls.
 */

#pragma once

#include "Control.h"
#include "Component.h"
#include "ParameterMap.h"
#include "MainDelegate.h"

class ControlComponent : virtual public Component
{
public:
    /**
     * @brief Construct a new Control Component object
     *
     * @param controlToAssign a reference to a Control object in the Model
     * @param newDelegate a reference to the MainDelegate object
     */
    ControlComponent(const Control &controlToAssign, MainDelegate &newDelegate);
    virtual ~ControlComponent() = default;

    /**
     * @brief  Paints the ControlComponent
     * 
     * @param g a graphics driver to use
     */
    virtual void paint(Graphics &g) override;

    /**
     * @brief Updates common attributes of the ControlComponent
     * 
     * This function should be called after attibutes of assigned
     * Control object have been changed.
     */
    virtual void syncComponentProperties(void);

    /**
     * @brief A callback handler for MIDI value changes initiated by MIDI
     * 
     * @param value a reference to the ControlValue object
     * @param midiValue MIDI value received
     * @param handle handle of the ControlValue
     */
    virtual void onMidiValueChange(const ControlValue &value,
                                   int16_t midiValue,
                                   uint8_t handle = 0) = 0;

    /**
     * @brief Triggers an update of the Control from the ParameterMap 
     * 
     */
    virtual void updateValueFromParameterMap(void);

    /**
     * @brief  A callback for Long LCD touch event
     * 
     * @param touchEvent information about the Touch event
     */
    virtual void onTouchLongHold(const TouchEvent &touchEvent) override;

    /**
     * @brief  A callback for Double click LCD touch event
     * 
     * @param touchEvent information about the Touch event
     */
    virtual void onTouchDoubleClick(const TouchEvent &touchEvent) override;

    /**
     * @brief  A callback for LCD touch down event
     * 
     * @param touchEvent information about the Touch event
     */
    virtual void onPotTouchDown(const PotEvent &potEvent) override;

    /**
     * @brief  A callback for LCD touch up event
     * 
     * @param touchEvent information about the Touch event
     */
    virtual void onPotTouchUp(const PotEvent &potEvent) override;

    /**
     * @brief  Emits (distrubutes) the value change to all required destinations
     * 
     * @param newDisplayValue a new value to be displayed
     * @param cv a reference to the ControlValue object
     */
    virtual void emitValueChange(int16_t newDisplayValue,
                                 const ControlValue &cv);

    /**
     * @brief Sets when an alternative background colour should be used
     * 
     * @param shouldUseAltBackground use true when an alternative background
     *  colour should be used
     */
    void setUseAltBackground(bool shouldUseAltBackground);

    /**
     * @brief Gets info if an alternative background colour should be used
     * 
     * @return true when an alternative background colour should be used
     */
    bool getUseAltBackground(void) const;

    /**
     * @brief Sets the active state of the ControlComponent
     * 
     * @param shouldBeActive use true when the ControlComponent should be active
     */
    void setActive(bool shouldBeActive);

    /**
     * @brief Gets the active state of the ControlComponent
     * 
     * @return true when the ControlComponent is active
     */
    bool getActive(void) const;

    /**
     * @brief A factory method for creating ControlComponent objects
     * 
     * @param control a Control object to be assigned to the ControlComponent
     * @param newDelegate a delegate to be assigned to the ControlComponent
     * @return a new ControlComponent object
     */
    static ControlComponent *createControlComponent(const Control &control,
                                                    MainDelegate &newDelegate);

    /**
     * @brief Create a Detail Control Component object
     * 
     * @param control a Control object to be assigned to the ControlComponent
     * @param newDelegate a delegate to be assigned to the ControlComponent
     * @return a new ControlComponent object
     */
    static ControlComponent *
        createDetailControlComponent(const Control &control,
                                     MainDelegate &newDelegate);

private:
    uint16_t calculateRelativeMidiValue(int16_t delta, const ControlValue &cv);
    uint16_t calculateAbsoluteMidiValue(int16_t newDisplayValue,
                                        const ControlValue &cv);

protected:
    const Control &control;
    MainDelegate &delegate;

    struct {
        bool useAltBackground : 1;
        bool active : 1;
    };
};
