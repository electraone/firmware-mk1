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
    bounds.setHeight(getHeight() - 6);
    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);
    paintButtonList(
        g, bounds, control.getColour565(), getList(), index, getActive());
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
    if (active) {
        g.setColour(Colours565::darker(colour, 0.2f));
        g.fillRoundRect(0, 0, bounds.getWidth(), bounds.getHeight(), 5);
    }

    g.setColour(colour);
    g.drawRoundRect(0, 0, bounds.getWidth(), bounds.getHeight(), 5);

    if (items->getByIndex(activeIndex).isBitmapEmpty()) {
        char stringValue[ControlValue::MaxLabelLength + 1];
        if (control.getValue(0).isLabelSet()) {
            copyString(stringValue,
                       control.getValue(0).getLabel(),
                       ControlValue::MaxLabelLength);
        } else {
            snprintf(stringValue,
                     ControlValue::MaxLabelLength,
                     "%s",
                     items->getByIndex(activeIndex).getLabel());
        }
        // Print the label
        g.printText(0,
                    bounds.getHeight() * 0.3f,
                    stringValue,
                    TextStyle::mediumTransparent,
                    bounds.getWidth(),
                    TextAlign::center);
    } else {
        // display bitmap image
        uint16_t paddingBitmap = ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;
        items->getByIndex(activeIndex)
            .paintBitmap(
                paddingBitmap, bounds.getHeight() * 0.3f, Colours565::white);
    }
}
