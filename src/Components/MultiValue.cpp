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
 * @file MultiValue.cpp
 *
 */

#include "MultiValue.h"

MultiValue::MultiValue() : activeSegment(0), activeSegmentIsShown(false)
{
}

/* Handles arrive from the parameter map, from Lua and from the SysEx API, none
 * of which is obliged to send one this component owns. Every accessor below
 * checks before it indexes - the vector is sized by the concrete component and
 * the callers used to guard it themselves, one hand-written comparison per
 * class, which is how a `handle <= getNumHandles()` came to write past the end
 * of a four-element vector.
 */
bool MultiValue::hasHandle(uint8_t handle) const
{
    return (handle < values.size());
}

void MultiValue::setValue(uint8_t handle, float newValue)
{
    if (!hasHandle(handle)) {
        return;
    }
    values[handle].set(newValue);
    repaint();
}

void MultiValue::applyValue(uint8_t handle, float delta)
{
    if (!hasHandle(handle)) {
        return;
    }
    values[handle].apply(delta);
    repaint();
}

float MultiValue::getValue(uint8_t handle)
{
    return (hasHandle(handle) ? values[handle].get() : 0.0f);
}

void MultiValue::setMin(uint8_t handle, float newMin)
{
    if (!hasHandle(handle)) {
        return;
    }
    if (values[handle].get() < newMin) {
        values[handle].set(newMin);
    }
    values[handle].setMin(newMin);
}

void MultiValue::setMax(uint8_t handle, float newMax)
{
    if (!hasHandle(handle)) {
        return;
    }
    if (values[handle].get() > newMax) {
        values[handle].set(newMax);
    }
    values[handle].setMax(newMax);
}

void MultiValue::setActiveSegment(uint8_t newActiveSegment)
{
    activeSegment = hasHandle(newActiveSegment) ? newActiveSegment : 0;
}

void MultiValue::showActiveSegment(bool shouldBeShown)
{
    activeSegmentIsShown = shouldBeShown;
    repaint();
}

void MultiValue::switchToNextActiveHandle(void)
{
    if (activeSegment < (getNumHandles() - 1)) {
        activeSegment++;
        setActiveSegment(activeSegment);
        repaint();
    }
}

void MultiValue::switchToPreviousActiveHandle(void)
{
    if (activeSegment > 0) {
        activeSegment--;
        setActiveSegment(activeSegment);
        repaint();
    }
}

uint8_t MultiValue::getActiveSegment(void) const
{
    return (activeSegment);
}