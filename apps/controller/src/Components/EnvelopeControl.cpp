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

#include "EnvelopeControl.h"
#include "LookAndFeel.h"
#include <algorithm>

EnvelopeControl::EnvelopeControl(const Control &control,
                                 MainDelegate &newDelegate,
                                 const EnvelopeShape &newShape)
    : ControlComponent(control, newDelegate), shape(newShape), baselineY(0)
{
    values.resize(shape.getNumSegments());
    points.resize(shape.getNumPoints());

    setValueRanges();
    setActiveSegment(control.inputs[0].getValueId());
    updateValueFromParameterMap();
}

/*
 * A preset is free to declare fewer values than the shape has segments; the
 * ones it did not declare keep the default range rather than being read out of
 * the end of the vector.
 */
void EnvelopeControl::setValueRanges(void)
{
    const uint8_t declared =
        std::min((size_t)shape.getNumSegments(), control.values.size());

    for (uint8_t segment = 0; segment < declared; segment++) {
        setMin(segment, control.values[segment].getMin());
        setMax(segment, control.values[segment].getMax());
    }
}

void EnvelopeControl::syncComponentProperties(void)
{
    setValueRanges();
    ControlComponent::syncComponentProperties();
}

void EnvelopeControl::onTouchDown([[maybe_unused]] const TouchEvent &touchEvent)
{
    showActiveSegment(true);
}

void EnvelopeControl::onTouchMove([[maybe_unused]] const TouchEvent &touchEvent)
{
    /* Dragging a contour does not change it here, and never has: all three
     * envelope controls carried the same commented-out block for it. */
}

void EnvelopeControl::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
    showActiveSegment(false);
}

void EnvelopeControl::onPotTouchDown(const PotEvent &potEvent)
{
    showActiveSegment(true);
    ControlComponent::onPotTouchDown(potEvent);
}

void EnvelopeControl::onPotChange(const PotEvent &potEvent)
{
    if (int16_t delta = potEvent.getAcceleratedChange()) {
        const uint8_t activeHandle = getActiveSegment();
        const auto &cv = control.getValue(activeHandle);
        const int16_t newDisplayValue =
            cv.isRelative() ? cv.calculateRelativeDelta(delta)
                            : cv.turnedValue(getValue(activeHandle), delta);

        emitValueChange(newDisplayValue, cv);
    }
}

void EnvelopeControl::onPotTouchUp(const PotEvent &potEvent)
{
    showActiveSegment(false);
    ControlComponent::onPotTouchUp(potEvent);
}

void EnvelopeControl::onMidiValueChange(const ControlValue &value,
                                        int16_t midiValue,
                                        uint8_t handle)
{
    const int16_t displayValue = value.translateMidiValue(midiValue);

    /* The shape has the last word on what a segment value means: a DX7 rate
     * counts down where the contour wants it counting up. MultiValue ignores a
     * handle it does not own, so the per-class bounds tests the three controls
     * each wrote for themselves are gone. */
    setValue(handle,
             shape.translateSegmentValue(handle, displayValue, value.getMax()));
}

uint8_t EnvelopeControl::getNumHandles(void) const
{
    return (shape.getNumSegments());
}

/*
 * Down to where a pad's box ends, so the baseline sits on the bottom edge of
 * the control beside it. It used to be half the control's height, which is a
 * row short of that and has nothing to do with anything else on the page.
 */
uint16_t EnvelopeControl::getContourHeight(void) const
{
    return ((getHeight() > controlMetrics.boxBottomInset)
                ? (uint16_t)(getHeight() - controlMetrics.boxBottomInset)
                : getHeight());
}

void EnvelopeControl::paintDecorations(Graphics &g)
{
    g.user().printText(0,
                       getHeight() - controlMetrics.nameInset,
                       getName(),
                       getWidth(),
                       TextAlign::center,
                       Colours565::white,
                       TextSize::medium);
    ControlComponent::paint(g);
}

void EnvelopeControl::paint(Graphics &g)
{
    Rectangle contourBounds = getLocalBounds();

    contourBounds.setHeight(getContourHeight());

    shape.computePoints(contourBounds, values, points, baselineY);

    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);
    LookAndFeel::paintEnvelope(g,
                               contourBounds,
                               control.getColour565(),
                               baselineY,
                               points,
                               activeSegment,
                               activeSegmentIsShown);
    paintDecorations(g);
}
