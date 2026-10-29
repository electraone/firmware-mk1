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

#include "ListControl.h"
#include "ControlMetrics.h"

Overlay ListControl::empty;

ListControl::ListControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate),
      List(control.values[0].getOverlay())
{
    updateValueFromParameterMap();
    enableEncoderMode(true);
}

/*
 * Where a step from the current entry lands. A list in cycle mode goes round,
 * from the last entry to the first and back; any other stops at its ends.
 */
int16_t ListControl::stepIndex(int16_t delta)
{
    const auto list = getList();
    const int16_t maxIndex = list ? list->getMaxIndex() : 0;
    const int16_t candidate = index + delta;

    if (control.getMode() == Control::Mode::Cycle) {
        const int16_t numItems = maxIndex + 1;
        const int16_t wrapped = candidate % numItems;

        return ((wrapped < 0) ? (int16_t)(wrapped + numItems) : wrapped);
    }

    return (constrain(candidate, 0, maxIndex));
}

void ListControl::syncComponentProperties(void)
{
    const auto &controlValue = control.getValue(0);
    assignListData(controlValue.getOverlay());
    setIndex(getIndex());
    ControlComponent::syncComponentProperties();
}

void ListControl::onTouchUp(const TouchEvent &touchEvent)
{
    if (getList()) {
        int16_t delta = touchEvent.getTouchDownX() - touchEvent.getX();

        if (abs(delta) > thresholdSwipeDistance) {
            int8_t step = ControlValue::invertedSign(delta);

            emitValueChange(stepIndex(step), control.getValue(0));
        }
    }
}

void ListControl::onPotTouchDown(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchDown(potEvent);
}

void ListControl::onPotChange(const PotEvent &potEvent)
{
    if (getList()) {
        if (int16_t delta = potEvent.getAcceleratedChange()) {
            const auto &cv = control.getValue(0);

            if (cv.isRelative()) {
                ControlComponent::emitValueChange(ControlValue::sign(delta),
                                                  cv);
            } else {
                emitValueChange(stepIndex(delta), cv);
            }
        }
    }
}

void ListControl::onPotTouchUp(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchUp(potEvent);
}

void ListControl::onMidiValueChange(const ControlValue &value,
                                    int16_t midiValue,
                                    [[maybe_unused]] uint8_t handle)
{
    int16_t index = value.translateMidiValue(midiValue);

    if (index >= 0) {
        setIndex(index);
    }
}

void ListControl::paint(Graphics &g)
{
    Rectangle listBounds = getBounds();

    /* The entry being shown may name a colour of its own. */
    const uint32_t colour = colourOfIndex(getList(), index);

    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);

    listBounds.setHeight(listBounds.getHeight() / 2);
    paintList(g, listBounds, colour, getList(), index);

    g.user().printText(0,
                       getHeight() - controlMetrics.nameInset,
                       getName(),
                       getWidth(),
                       TextAlign::center,
                       Colours565::white,
                       TextSize::medium);
    ControlComponent::paint(g);
}

void ListControl::emitValueChange(int16_t newIndex, const ControlValue &cv)
{
    if (cv.isRelative()) {
        ControlComponent::emitValueChange(newIndex, cv);
    } else {
        if (auto list = getList()) {
            int16_t midiValue = list->getValueByIndex(newIndex);

            parameterMap.setValue(
                control.values[0].message.getDeviceId(),
                control.values[0].message.getType(),
                control.values[0].message.getParameterNumber(),
                midiValue,
                Origin::internal);
        }
    }
}

void ListControl::paintList(Graphics &g,
                            const Rectangle &bounds,
                            uint32_t colour,
                            const ListData *items,
                            uint16_t activeIndex)
{
    if (!items) {
        return;
    }

    /* A list finds its overlay item by position, not by value. Held in a
     * local because getByIndex() answers with a copy. */
    ListDataItem item = items->getByIndex(activeIndex);

    /*
     * The dots, or the bar that stands in for them, end where every control's
     * graphics end - the same row a fader's track ends on. They used to sit
     * at a row written into the painter, which finished higher than the fader
     * beside them and made a row of controls read as several things.
     *
     * The two shapes are anchored differently: paintDots takes the centre of
     * its circles and paintBar the top of its track. Both are as tall as the
     * strip, so both are placed here from the one bottom edge rather than
     * from a y that means something different to each.
     */
    const uint16_t dotsHeight = controlMetrics.stripHeight;
    const uint16_t graphicsEnd =
        (getHeight() > controlMetrics.graphicsBottomInset)
            ? (uint16_t)(getHeight() - controlMetrics.graphicsBottomInset)
            : 0;
    const uint16_t graphicsTop =
        (graphicsEnd > dotsHeight) ? (uint16_t)(graphicsEnd - dotsHeight) : 0;

    /* The value a gap above them, in the face the preset asked for. */
    const uint16_t labelY = valueLabelY(g, graphicsTop);

    if (item.isBitmapEmpty()) {
        char stringValue[ControlValue::MaxLabelLength + 1];

        if (control.getValue(0).isLabelSet()) {
            copyString(stringValue,
                       control.getValue(0).getLabel(),
                       ControlValue::MaxLabelLength);
        } else {
            snprintf(stringValue, sizeof(stringValue), "%s", item.getLabel());
        }

        /* Cut to the control with a marker when the face is too wide for it -
         * a huge face makes that likely, and a label clipped at the edge says
         * less than one that says it was cut. */
        char fitted[ControlValue::MaxLabelLength + 2];

        fitValueText(g, fitted, sizeof(fitted), stringValue, bounds.getWidth());
        printValue(g,
                   0,
                   labelY,
                   fitted,
                   bounds.getWidth(),
                   TextAlign::center,
                   Colours565::white);
    } else {
        uint16_t paddingBitmap = ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;

        item.paintBitmap(paddingBitmap, labelY, colour);
    }

    Rectangle dotBounds = bounds;
    dotBounds.setHeight(dotsHeight);

    const uint16_t radius = (dotsHeight - 1) / 2;
    const uint16_t spacing = (2 * radius) + 3;

    /* Dots while they fit across the control, the bar once they would not.
     * This used to be a count of sixteen whatever the width, which asks the
     * wrong question: it is the width that runs out. */
    const bool dotsFit = (items->getNumItems() * spacing) <= bounds.getWidth();

    if (dotsFit) {
        /* The centre of the circles, which is a radius below the top. */
        dotBounds.setY(graphicsTop + radius);
        LookAndFeel::paintDots(
            g, dotBounds, colour, items, activeIndex, radius, spacing);
    } else {
        dotBounds.setY(graphicsTop);
        LookAndFeel::paintBar(
            g, dotBounds, colour, items, activeIndex, dotsHeight);
    }
}
