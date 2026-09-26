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

#include "VerticalFader4Control.h"
#include "LookAndFeel.h"
#include <algorithm>

VerticalFader4Control::VerticalFader4Control(const Control &control,
                                             MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate)
{
    values.resize(numHandles);

    setValueRanges();
    setActiveSegment(control.inputs[0].getValueId());
    updateValueFromParameterMap();
}

/*
 * A preset is free to declare fewer values than the four bars; the ones it did
 * not declare keep the default range rather than being read out of the end of
 * the vector.
 */
void VerticalFader4Control::setValueRanges(void)
{
    const uint8_t declared =
        std::min((size_t)numHandles, control.values.size());

    for (uint8_t handle = 0; handle < declared; handle++) {
        setMin(handle, control.values[handle].getMin());
        setMax(handle, control.values[handle].getMax());
    }
}

void VerticalFader4Control::syncComponentProperties(void)
{
    setValueRanges();
    ControlComponent::syncComponentProperties();
}

void VerticalFader4Control::onTouchDown(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    showActiveSegment(true);
}

void VerticalFader4Control::onTouchMove(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
}

void VerticalFader4Control::onTouchUp(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    showActiveSegment(false);
}

void VerticalFader4Control::onPotTouchDown(const PotEvent &potEvent)
{
    showActiveSegment(true);
    ControlComponent::onPotTouchDown(potEvent);
}

void VerticalFader4Control::onPotChange(const PotEvent &potEvent)
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

void VerticalFader4Control::onPotTouchUp(const PotEvent &potEvent)
{
    showActiveSegment(false);
    ControlComponent::onPotTouchUp(potEvent);
}

void VerticalFader4Control::onMidiValueChange(const ControlValue &value,
                                              int16_t midiValue,
                                              uint8_t handle)
{
    setValue(handle, value.translateMidiValue(midiValue));
}

uint8_t VerticalFader4Control::getNumHandles(void) const
{
    return (numHandles);
}

/*
 * The bars end on the bottom edge of the pad beside them, as an envelope's
 * contour does - they are the whole drawing, not a strip under a value.
 */
uint16_t VerticalFader4Control::getBarsHeight(void) const
{
    return ((getHeight() > controlMetrics.boxBottomInset)
                ? (uint16_t)(getHeight() - controlMetrics.boxBottomInset)
                : getHeight());
}

float VerticalFader4Control::getBarWidthFraction(void) const
{
    return (0.75f);
}

float VerticalFader4Control::getBarGapFraction(void) const
{
    return (2.0f / 3.0f);
}

void VerticalFader4Control::paintDecorations(Graphics &g)
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

/*
 * One slot per bar across the box; the bars a fraction of it wide, a fraction
 * of the rest apart, and the row centred. They used to be placed at
 * i * (width / 4) + i * 4 and drawn width / 4 - 10 wide, which put the last
 * one past the box on any width.
 */
void VerticalFader4Control::paint(Graphics &g)
{
    const uint32_t colour = control.getColour565();
    const uint16_t barsHeight = getBarsHeight();

    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);

    constexpr uint16_t minimumGap = 10;
    const uint16_t slot = getWidth() / numHandles;
    const uint16_t barWidth =
        (slot > minimumGap)
            ? (uint16_t)((slot - minimumGap) * getBarWidthFraction())
            : 0;
    const uint16_t gap = (uint16_t)((slot - barWidth) * getBarGapFraction());
    const uint16_t rowWidth =
        (numHandles * barWidth) + ((numHandles - 1) * gap);
    const uint16_t rowX =
        (getWidth() > rowWidth) ? (getWidth() - rowWidth) / 2 : 0;

    for (uint8_t i = 0; i < numHandles; i++) {
        Rectangle bounds = getBounds();

        bounds.setX(rowX + (i * (barWidth + gap)));
        bounds.setWidth(barWidth);
        bounds.setHeight(barsHeight);

        LookAndFeel::paintBarVertical(g,
                                      bounds,
                                      colour,
                                      values[i].getMin(),
                                      values[i].getMax(),
                                      values[i].get(),
                                      activeSegmentIsShown
                                          && (activeSegment == i));
    }

    paintDecorations(g);
}
