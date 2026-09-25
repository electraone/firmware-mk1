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
#include "ListDetailLayout.h"
#include "LookAndFeel.h"

ListDetailControl::ListDetailControl(const Control &control,
                                     MainDelegate &newDelegate)
    : ListControl(control, newDelegate),
      valueChosen(false),
      paged(ListDetailLayout::isPaged(control)),
      page(0),
      pressedStrip(0),
      listItemWidth(getWidth()),
      listItemHeight(ListDetailLayout::columnItemHeight)
{
    assignPot(control.inputs[0].getPotId(), control.values[0].getNumSteps());
    showPageOf(index);
}

void ListDetailControl::paint(Graphics &g)
{
    g.fillAll(Colours565::black);

    if (paged) {
        paintGrid(g);
        return;
    }

    paintColumn(g);
}

void ListDetailControl::resized(void)
{
    listItemWidth = getWidth();
}

void ListDetailControl::paintColumn(Graphics &g)
{
    auto overlay = getList();

    if (!overlay) {
        return;
    }

    uint16_t numItems = overlay->getNumItems();

    for (uint16_t i = 0;
         (i < numItems) && (i < ListDetailLayout::itemsInColumn);
         i++) {
        paintItem(g,
                  i,
                  0,
                  i * listItemHeight,
                  listItemWidth,
                  listItemHeight - ListDetailLayout::gridItemGap);
    }
}

void ListDetailControl::paintGrid(Graphics &g)
{
    auto overlay = getList();

    if (!overlay) {
        return;
    }

    uint16_t numItems = overlay->getNumItems();
    uint16_t firstItem = page * ListDetailLayout::itemsInPage;
    uint16_t itemWidth = columnWidth() - ListDetailLayout::gridItemGap;
    uint16_t itemHeight = rowHeight() - ListDetailLayout::gridItemGap;

    for (uint16_t i = 0; i < ListDetailLayout::itemsInPage; i++) {
        uint16_t itemIndex = firstItem + i;

        if (itemIndex >= numItems) {
            break;
        }

        uint16_t column = i / ListDetailLayout::itemsInColumn;
        uint16_t row = i % ListDetailLayout::itemsInColumn;

        paintItem(g,
                  itemIndex,
                  gridLeft() + (column * columnWidth())
                      + (ListDetailLayout::gridItemGap / 2),
                  (row * rowHeight()) + (ListDetailLayout::gridItemGap / 2),
                  itemWidth,
                  itemHeight);
    }

    paintSideBar(
        g, ListDetailLayout::sideBarInset, false, page > 0, pressedStrip < 0);
    paintSideBar(g,
                 getWidth() - ListDetailLayout::sideBarInset
                     - ListDetailLayout::sideBarWidth,
                 true,
                 (page + 1) < numPages(),
                 pressedStrip > 0);
}

void ListDetailControl::paintSideBar(Graphics &g,
                                     uint16_t x,
                                     bool pointsRight,
                                     bool enabled,
                                     bool pressed)
{
    /* The arrow is centred in the strip. A strip with no page behind it is
     * not drawn at all - black, like the ground the items stand on - so the
     * only strip that shows is one that can be tapped. */
    const uint16_t top = ListDetailLayout::sideBarTop();
    const uint16_t height = ListDetailLayout::sideBarHeight(getHeight());

    LookAndFeel::paintDetailSideBar(g,
                                    x,
                                    top,
                                    ListDetailLayout::sideBarWidth,
                                    height,
                                    top + (height / 2)
                                        - ListDetailLayout::sideBarGlyphLift,
                                    pointsRight,
                                    enabled,
                                    pressed);
}

void ListDetailControl::paintItem(Graphics &g,
                                  uint16_t itemIndex,
                                  uint16_t x,
                                  uint16_t y,
                                  uint16_t width,
                                  uint16_t height)
{
    auto overlay = getList();
    auto listItem = overlay->getByIndex(itemIndex);

    /* Every other item a shade apart tells the rows of a column from each
     * other. */
    LookAndFeel::paintDetailListItem(
        g, x, y, width, height, control.getColour565(), itemIndex % 2);

    /* The label in the face the preset gave the list - the same one its value
     * wears on the page - centred on the row, and cut to the box when a big
     * face would not fit it. */
    char stringValue[ControlValue::MaxLabelLength + 2];
    const uint16_t available =
        (width > (2 * itemTextMargin)) ? (width - (2 * itemTextMargin)) : 0;
    Bitmap bitmap;

    /* Label and bitmap come out of the display's memory together. Asking for
     * them one at a time fetched the same record twice, for every item. */
    const char *label = listItem.getLabelAndBitmap(bitmap);

    fitValueText(g, stringValue, sizeof(stringValue), label, available);

    const uint16_t labelY = valueLabelYCentredOn(g, y + (height / 2));

    printValue(
        g, x, labelY, stringValue, width, TextAlign::center, Colours565::white);

    if (listItem.isBitmapEmpty() == false) {
        Hardware::memory.bitmapPool.paint(bitmap,
                                          x + 28,
                                          y + (height - BITMAP_HEIGHT) / 2,
                                          Colours565::white);
    }

    if (index == itemIndex) {
        LookAndFeel::paintDetailListItemSelection(g, x, y, width, height);
    }
}

uint16_t ListDetailControl::rowHeight(void)
{
    return (ListDetailLayout::rowHeight(getHeight()));
}

uint16_t ListDetailControl::columnWidth(void)
{
    return ((getWidth() - (2 * gridLeft())) / ListDetailLayout::gridColumns);
}

/** Where the items start: past the inset, the strip, and the gap after it. */
uint16_t ListDetailControl::gridLeft(void)
{
    return (ListDetailLayout::sideBarInset + ListDetailLayout::sideBarWidth
            + ListDetailLayout::gridSideGap);
}

int16_t ListDetailControl::itemAt(uint16_t x, uint16_t y)
{
    auto overlay = getList();

    if (!overlay || (x < gridLeft())) {
        return (-1);
    }

    uint16_t column = (x - gridLeft()) / columnWidth();
    uint16_t row = y / rowHeight();

    if ((column >= ListDetailLayout::gridColumns)
        || (row >= ListDetailLayout::itemsInColumn)) {
        return (-1);
    }

    uint16_t itemIndex = (page * ListDetailLayout::itemsInPage)
                         + (column * ListDetailLayout::itemsInColumn) + row;

    return ((itemIndex < overlay->getNumItems()) ? (int16_t)itemIndex : -1);
}

uint16_t ListDetailControl::numPages(void)
{
    auto overlay = getList();
    uint16_t numItems = overlay ? overlay->getNumItems() : 0;

    return ((numItems == 0) ? 1
                            : ((numItems + ListDetailLayout::itemsInPage - 1)
                               / ListDetailLayout::itemsInPage));
}

void ListDetailControl::showPageOf(uint16_t itemIndex)
{
    uint16_t newPage = itemIndex / ListDetailLayout::itemsInPage;

    if (newPage != page) {
        page = newPage;
        repaintPage();
    }
}

void ListDetailControl::repaintPage(void)
{
    Component *parent = getParentComponent();

    if (!parent) {
        repaint();
        return;
    }

    /* In the order the detail added them, which is the order they are drawn
     * in: the list first, then the lock on top of it. */
    for (Component *sibling : parent->getChildren()) {
        sibling->repaint();
    }
}

int8_t ListDetailControl::stripAt(uint16_t x)
{
    /* The inset beside each strip counts as the strip: a finger that lands
     * just outside it on the screen's edge still means it. */
    const uint16_t reach =
        ListDetailLayout::sideBarInset + ListDetailLayout::sideBarWidth;

    if (x < reach) {
        return (-1);
    }

    if (x >= (getWidth() - reach)) {
        return (1);
    }

    return (0);
}

void ListDetailControl::setPressedStrip(int8_t strip)
{
    if (strip != pressedStrip) {
        pressedStrip = strip;
        repaintPage();
    }
}

void ListDetailControl::chooseItem(uint16_t itemIndex)
{
    const auto &cv = control.getValue(0);

    if (cv.isRelative()) {
        emitValueChange((int16_t)itemIndex - index, cv);
    } else {
        emitValueChange(itemIndex, cv);
    }

    valueChosen = true;
}

void ListDetailControl::onTouchDown(const TouchEvent &touchEvent)
{
    if (paged) {
        setPressedStrip(stripAt(touchEvent.getX()));
    }
}

void ListDetailControl::onTouchClick(const TouchEvent &touchEvent)
{
    if (paged) {
        /* The strips turn the page rather than choosing anything, so the
         * window stays up under the finger. The lock sits on top of the
         * bottom of the right hand one and takes its own touches before this
         * is ever asked. */
        const int8_t strip = stripAt(touchEvent.getX());

        if (strip < 0) {
            if (page > 0) {
                page--;
                repaintPage();
            }
            return;
        }

        if (strip > 0) {
            if ((page + 1) < numPages()) {
                page++;
                repaintPage();
            }
            return;
        }

        int16_t itemIndex = itemAt(touchEvent.getX(), touchEvent.getY());

        if (itemIndex >= 0) {
            chooseItem(itemIndex);
        }
        return;
    }

    chooseItem(touchEvent.getY() / listItemHeight);
}

void ListDetailControl::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
    setPressedStrip(0);

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
        int16_t newIndex = list->getIndexByValue(midiValue);

        if (newIndex >= 0) {
            setIndex(newIndex);
            showPageOf(newIndex);
        }
    }
}
