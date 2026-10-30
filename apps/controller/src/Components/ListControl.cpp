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

Overlay ListControl::empty;

ListControl::ListControl(const Control &control, MainDelegate &newDelegate)
    : ControlComponent(control, newDelegate),
      List(control.values[0].getOverlay())
{
    updateValueFromParameterMap();
    enableEncoderMode(true);
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
    if (auto list = getList()) {
        int16_t delta = touchEvent.getTouchDownX() - touchEvent.getX();

        if (abs(delta) > thresholdSwipeDistance) {
            int8_t step = ControlValue::invertedSign(delta);
            uint16_t newIndex = constrain(index + step, 0, list->getMaxIndex());

            emitValueChange(newIndex, control.getValue(0));
        }
    }
}

void ListControl::onPotTouchDown(const PotEvent &potEvent)
{
    ControlComponent::onPotTouchDown(potEvent);
}

void ListControl::onPotChange(const PotEvent &potEvent)
{
    if (auto list = getList()) {
        if (int16_t delta = potEvent.getAcceleratedChange()) {
            const auto &cv = control.getValue(0);

            if (cv.isRelative()) {
                ControlComponent::emitValueChange(ControlValue::sign(delta),
                                                  cv);
            } else {
                int16_t newIndex =
                    constrain(index + delta, 0, list->getMaxIndex());
                emitValueChange(newIndex, cv);
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
    listBounds.setHeight(listBounds.getHeight() / 2);
    g.fillAll(getUseAltBackground() ? LookAndFeel::altBackgroundColour
                                    : LookAndFeel::backgroundColour);
    paintList(g, listBounds, control.getColour565(), getList(), index);
    g.printText(0,
                getHeight() - 20,
                getName(),
                TextStyle::mediumWhiteOnBlack,
                getWidth(),
                TextAlign::center,
                2);
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
    if (items) {
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

            g.printText(0,
                        0,
                        stringValue,
                        TextStyle::mediumTransparent,
                        bounds.getWidth(),
                        TextAlign::center);
        } else {
            // display bitmap image
            uint16_t paddingBitmap =
                ((bounds.getWidth() - BITMAP_WIDTH)) / 2 - 1;
            items->getByIndex(activeIndex)
                .paintBitmap(paddingBitmap, 0, colour);
        }
        // Paint the graphics
        if (items->getNumItems() < 16) {
            LookAndFeel::paintDots(g, bounds, colour, items, activeIndex);
        } else {
            LookAndFeel::paintBar(g, bounds, colour, items, activeIndex);
        }
    }
}
