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

#include "ListDetailControl.h"

ListDetailControl::ListDetailControl(const Control &control,
                                     MainDelegate &newDelegate)
    : ListControl(control, newDelegate),
      valueChosen(false),
      offsetY(0),
      prevTouchY(0),
      listItemWidth(getWidth()),
      listItemHeight(80),
      maxItemsInViewport(7),
      itemTop(offsetY / listItemHeight)
{
    determineOffsetY();
    assignPot(control.inputs[0].getPotId(), control.values[0].getNumSteps());
}

void ListDetailControl::paint(Graphics &g)
{
    g.fillAll(Colours565::black);
    paintListItems(g, offsetY);
    g.copy(SDRAM_PAGE_4,
           0,
           (offsetY % listItemHeight),
           g.getActiveBufferAddress(),
           0,
           0,
           listItemWidth,
           getHeight() - 5);
}

void ListDetailControl::resized(void)
{
    listItemWidth = getWidth();
    maxItemsInViewport = (getHeight() / listItemHeight) + 1;
}

void ListDetailControl::onTouchDown(const TouchEvent &touchEvent)
{
    prevTouchY = touchEvent.getY();
}

void ListDetailControl::onTouchMove(const TouchEvent &touchEvent)
{
    auto overlay = getList();
    uint16_t numItems = overlay->getNumItems();

    if (numItems < maxItemsInViewport) {
        return;
    }

    if (prevTouchY != touchEvent.getY()) {
        offsetY += prevTouchY - touchEvent.getY();

        if (offsetY < 0) {
            offsetY = 0;
        } else if (offsetY > ((numItems * listItemHeight)
                              - (listItemHeight * maxItemsInViewport))) {
            offsetY = (numItems * listItemHeight)
                      - (listItemHeight * maxItemsInViewport);
        }

        prevTouchY = touchEvent.getY();
        itemTop = offsetY / listItemHeight;

        repaint();
    }
}

void ListDetailControl::onTouchClick(const TouchEvent &touchEvent)
{
    int16_t newIndex = (offsetY + touchEvent.getY()) / listItemHeight;
    const auto &cv = control.getValue(0);

    if (cv.isRelative()) {
        int16_t delta = newIndex - index;
        emitValueChange(delta, cv);
    } else {
        emitValueChange(newIndex, cv);
    }

    valueChosen = true;
}

void ListDetailControl::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
    if (valueChosen) {
        if (!delegate.isDetailLocked()) {
            delegate.closeDetail();
        }
    }
}

void ListDetailControl::onTouchLongHold(
    [[maybe_unused]] const TouchEvent &touchEvent)
{
}

void ListDetailControl::onPotTouchUp([[maybe_unused]] const PotEvent &potEvent)
{
    if (!delegate.isDetailLocked()) {
        delegate.closeDetail();
    }
}

void ListDetailControl::onMidiValueChange(
    [[maybe_unused]] const ControlValue &value,
    int16_t midiValue,
    [[maybe_unused]] uint8_t handle)
{
    if (auto list = getList()) {
        int16_t index = list->getIndexByValue(midiValue);

        if (index >= 0) {
            setIndex(index);
            determineOffsetY();
        }
    }
}

void ListDetailControl::paintListItems(Graphics &g, int16_t offset)
{
    auto overlay = getList();
    uint16_t firstItem = offset / listItemHeight;
    uint16_t j = 0;
    uint16_t numItems = overlay->getNumItems();
    uint32_t writeAddress = g.getActiveBufferAddress();

    // set for the list rendering memory page
    auto virtualWindowHeight = listItemHeight * (maxItemsInViewport + 1);
    auto origX = g.getActiveWindowX();
    auto origY = g.getActiveWindowY();
    auto origWidth = g.getActiveWindowWidth();
    auto origHeight = g.getActiveWindowHeight();
    g.setCanvasAddress(SDRAM_PAGE_4);
    g.setActiveWindowPosition(0, 0);
    g.setActiveWindowSize(1024, virtualWindowHeight);

    g.setColour(Colours565::black);
    g.fillRect(0, 0, listItemWidth, virtualWindowHeight);

    for (uint16_t i = firstItem; i < numItems; i++) {
        if (j > maxItemsInViewport) {
            break;
        }

        auto listItem = overlay->getByIndex(i);

        g.setColour(i % 2 ? Colours565::darker(control.getColour565(), 0.2f)
                          : Colours565::darker(control.getColour565(), 0.5f));
        g.fillRect(0, j * listItemHeight + 0, listItemWidth, 60);

        char stringValue[21];

        snprintf(stringValue, sizeof(stringValue), "%s", listItem.getLabel());

        g.printText(0,
                    j * listItemHeight + 25,
                    stringValue,
                    TextStyle::mediumTransparent,
                    listItemWidth,
                    TextAlign::center);

        if (listItem.isBitmapEmpty() == false) {
            Bitmap bitmap = listItem.getBitmap();
            Hardware::memory.bitmapPool.paint(bitmap,
                                              28,
                                              j * listItemHeight + 22,
                                              Colours565::white,
                                              SDRAM_PAGE_4);
        }

        if (index == i) {
            g.setColour(Colours565::white);
            g.drawRect(0, j * listItemHeight + 0, listItemWidth, 60);
            g.drawRect(1, j * listItemHeight + 1, listItemWidth - 2, 58);
        }
        j++;
    }

    g.setCanvasAddress(writeAddress);
    g.setActiveWindowPosition(origX, origY);
    g.setActiveWindowSize(origWidth, origHeight);
}

void ListDetailControl::determineOffsetY(void)
{
    auto overlay = getList();
    uint16_t numItems = overlay->getNumItems();

    if (numItems < maxItemsInViewport) {
        offsetY = 0;
    } else if (index <= itemTop || index >= (itemTop + maxItemsInViewport)) {
        if (index < itemTop) {
            itemTop = index;
        } else if (index >= (itemTop + maxItemsInViewport)) {
            itemTop = index - maxItemsInViewport + 1;
        }
        offsetY = itemTop * listItemHeight;
    }
}
