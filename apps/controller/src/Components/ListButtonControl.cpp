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

#include "ListButtonControl.h"
#include "ControlMetrics.h"

Overlay ListButtonControl::empty;

ListButtonControl::ListButtonControl(const Control &control,
                                     MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate),
      List(control.values[0].getOverlay())
{
    updateValueFromParameterMap();
    enableEncoderMode(true);
}

void ListButtonControl::syncComponentProperties(void)
{
    const auto &controlValue = control.getValue(0);
    assignListData(controlValue.getOverlay());
    ControlComponent::syncComponentProperties();
}

void ListButtonControl::onTouchDown(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
    setActive(true);
}

void ListButtonControl::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
    setActive(false);
    if (auto list = getList()) {
        const auto &cv = control.getValue(0);

        if (cv.isRelative()) {
            ControlComponent::emitValueChange(1, cv);
        } else {
            uint16_t newIndex = index + 1;
            if (newIndex > list->getMaxIndex()) {
                newIndex = 0;
            }
            emitValueChange(newIndex, cv);
        }
    }
}

void ListButtonControl::onPotTouchDown(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchDown(potEvent);
}

void ListButtonControl::onPotChange(const PotEvent &potEvent)
{
    if (auto list = getList()) {
        const auto &cv = control.getValue(0);

        if (int16_t delta = potEvent.getAcceleratedChange()) {
            if (cv.isRelative()) {
                ControlComponent::emitValueChange(ControlValue::sign(delta),
                                                  cv);
            } else {
                int16_t newIndex = index + delta;

                if (newIndex > list->getMaxIndex()) {
                    newIndex = 0;
                } else if (newIndex < 0) {
                    newIndex = list->getMaxIndex();
                }
                emitValueChange(newIndex, cv);
            }
        }
    }
}

void ListButtonControl::onPotTouchUp(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchUp(potEvent);
}

void ListButtonControl::onMidiValueChange(const ControlValue &value,
                                          int16_t midiValue,
                                          [[maybe_unused]] uint8_t handle)
{
    int16_t index = value.translateMidiValue(midiValue);

    if (index >= 0) {
        setIndex(index);
    }
}

void ListButtonControl::paint(Graphics &g)
{
    auto bounds = getBounds();
    bounds.setX(0);
    bounds.setY(3);
    bounds.setWidth(getWidth());
    bounds.setHeight((getHeight() > 6) ? (getHeight() - 6) : getHeight());

    /* Level with the bottom of the names of the faders beside it, since this
     * control prints none of its own. */
    if (controlMetrics.boxesEndOnNameBaseline) {
        bounds.setHeight(std::min(nameBaseline(g), getHeight()));
    }

    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);
    paintButtonList(g,
                    bounds,
                    colourOfIndex(getList(), index),
                    getList(),
                    index,
                    getActive());
    ControlComponent::paint(g);
}

void ListButtonControl::emitValueChange(int16_t newIndex,
                                        [[maybe_unused]] const ControlValue &cv)
{
    if (auto list = getList()) {
        int16_t midiValue = list->getValueByIndex(newIndex);

        parameterMap.setValue(control.values[0].message.getDeviceId(),
                              control.values[0].message.getType(),
                              control.values[0].message.getParameterNumber(),
                              midiValue,
                              Origin::internal);
    }
}

void ListButtonControl::paintButtonList(Graphics &g,
                                        const Rectangle &bounds,
                                        uint32_t colour,
                                        const ListData *items,
                                        uint16_t activeIndex,
                                        bool active)
{
    if (!items) {
        return;
    }

    const uint16_t inset = controlMetrics.listButtonBoxInset;
    const uint16_t boxHeight = (bounds.getHeight() > inset)
                                   ? (uint16_t)(bounds.getHeight() - inset)
                                   : bounds.getHeight();

    LookAndFeel::paintValueBox(g, bounds, colour, boxHeight, active);

    ListDataItem item = items->getByIndex(activeIndex);

    if (item.isBitmapEmpty()) {
        char stringValue[ControlValue::MaxLabelLength + 1];

        if (control.getValue(0).isLabelSet()) {
            copyString(stringValue,
                       control.getValue(0).getLabel(),
                       ControlValue::MaxLabelLength);
        } else {
            snprintf(stringValue, sizeof(stringValue), "%s", item.getLabel());
        }

        /* In the face the preset chose, and cut to the box with a marker when
         * that face is too wide for it. */
        char fitted[ControlValue::MaxLabelLength + 2];

        fitValueText(g, fitted, sizeof(fitted), stringValue, bounds.getWidth());
        printValue(g,
                   0,
                   bounds.getHeight() * 0.25f,
                   fitted,
                   bounds.getWidth(),
                   TextAlign::center,
                   Colours565::white);
    } else {
        uint16_t paddingBitmap = ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;

        item.paintBitmap(
            paddingBitmap, bounds.getHeight() * 0.25f, Colours565::white);
    }

    /* The dots under the box. Without them a value-only list says what it is
     * on, and nothing at all about where that is in the list. */
    Rectangle dotBounds = bounds;

    dotBounds.setHeight(6);
    dotBounds.setY(
        (boxHeight > controlMetrics.listButtonDotsInset)
            ? (uint16_t)(boxHeight - controlMetrics.listButtonDotsInset)
            : 0);

    if (items->getNumItems() < 16) {
        LookAndFeel::paintDots(g, dotBounds, colour, items, activeIndex);
    } else {
        LookAndFeel::paintBar(g, dotBounds, colour, items, activeIndex);
    }
}
