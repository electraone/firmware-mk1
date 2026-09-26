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

#include "XyPadControl.h"
#include "LookAndFeel.h"
#include <algorithm>

XyPadControl::XyPadControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate)
{
    values.resize(numHandles);

    setValueRanges();
    setActiveSegment(control.inputs[0].getValueId());
    updateValueFromParameterMap();
}

void XyPadControl::setValueRanges(void)
{
    const uint8_t declared =
        std::min((size_t)numHandles, control.values.size());

    for (uint8_t handle = 0; handle < declared; handle++) {
        setMin(handle, control.values[handle].getMin());
        setMax(handle, control.values[handle].getMax());
    }
}

void XyPadControl::syncComponentProperties(void)
{
    setValueRanges();
    ControlComponent::syncComponentProperties();
}

void XyPadControl::onTouchDown([[maybe_unused]] const TouchEvent &touchEvent)
{
    showActiveSegment(true);
}

void XyPadControl::onTouchMove([[maybe_unused]] const TouchEvent &touchEvent)
{
}

void XyPadControl::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
    showActiveSegment(false);
}

void XyPadControl::onPotTouchDown(const PotEvent &potEvent)
{
    showActiveSegment(true);
    ControlComponent::onPotTouchDown(potEvent);
}

void XyPadControl::onPotChange(const PotEvent &potEvent)
{
    if (int16_t delta = potEvent.getAcceleratedChange()) {
        const uint8_t activeHandle = getActiveSegment();
        const auto &cv = control.getValue(activeHandle);
        const int16_t newDisplayValue = cv.isRelative()
                                            ? cv.calculateRelativeDelta(delta)
                                            : getValue(activeHandle) + delta;

        emitValueChange(newDisplayValue, cv);
    }
}

void XyPadControl::onPotTouchUp(const PotEvent &potEvent)
{
    showActiveSegment(false);
    ControlComponent::onPotTouchUp(potEvent);
}

void XyPadControl::onMidiValueChange(const ControlValue &value,
                                     int16_t midiValue,
                                     uint8_t handle)
{
    setValue(handle, value.translateMidiValue(midiValue));
}

uint8_t XyPadControl::getNumHandles(void) const
{
    return (numHandles);
}

uint16_t XyPadControl::handleX(uint16_t width) const
{
    return ((uint16_t)map(values[xHandle].get(),
                          values[xHandle].getMin(),
                          values[xHandle].getMax(),
                          0.0f,
                          (float)width));
}

uint16_t XyPadControl::handleY(uint16_t height) const
{
    return ((uint16_t)(height
                       - map(values[yHandle].get(),
                             values[yHandle].getMin(),
                             values[yHandle].getMax(),
                             0.0f,
                             (float)height)));
}

void XyPadControl::paint(Graphics &g)
{
    const uint32_t highlightedColour = control.getColour565();
    const uint32_t colour = Colours565::darker(highlightedColour, 0.6f);
    const uint16_t inset = controlMetrics.xyPadHeightInset;

    uint16_t height =
        (getHeight() > inset) ? (uint16_t)(getHeight() - inset) : getHeight();

    /* Level with the bottom of the names of the faders beside it. The name
     * moves up with the pad's bottom edge, keeping the distance it had from
     * it. */
    uint16_t nameY = (getHeight() > controlMetrics.nameInset)
                         ? (uint16_t)(getHeight() - controlMetrics.nameInset)
                         : 0;

    if (controlMetrics.boxesEndOnNameBaseline) {
        const uint16_t baseline =
            std::min(nameBaseline(g), (uint16_t)getHeight());
        const uint16_t raise = getHeight() - baseline;

        nameY = (nameY > raise) ? (uint16_t)(nameY - raise) : 0;
        height = baseline;
    }

    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);

    Rectangle padBounds = getLocalBounds();
    padBounds.setHeight(height);

    LookAndFeel::paintXyPad(
        g,
        padBounds,
        Colours565::darker(colour, 0.3f),
        (activeSegmentIsShown && (activeSegment == xHandle)) ? highlightedColour
                                                             : colour,
        (activeSegmentIsShown && (activeSegment == yHandle)) ? highlightedColour
                                                             : colour,
        highlightedColour,
        handleX(getWidth()),
        handleY(height));

    g.user().printText(0,
                       nameY,
                       getName(),
                       getWidth(),
                       TextAlign::center,
                       Colours565::white,
                       TextSize::medium);

    ControlComponent::paint(g);
}
