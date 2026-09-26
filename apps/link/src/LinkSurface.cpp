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

#include "LinkSurface.h"
#include "Control.h"
#include "PotEvent.h"

LinkSurface::LinkSurface(LinkClient &newLink) : link(newLink)
{
}

bool LinkSurface::isCustom(const Link::ViewElement &element)
{
    return ((Control::Type)element.nativeType == Control::Type::Custom);
}

/*
 * The element in the active control set first, because a page has three rows
 * on the same twelve knobs and the primary said which row is live. Without
 * the preference the top row won every time, whatever was highlighted.
 */
const Link::ViewElement *LinkSurface::elementOnPot(uint8_t potId) const
{
    const Link::ViewElement *fallback = nullptr;

    for (const Link::ViewElement &element : link.getView().getElements()) {
        if (!element.visible || !element.isDrivenByPot(potId)) {
            continue;
        }

        if (element.active) {
            return (&element);
        }

        if (fallback == nullptr) {
            fallback = &element;
        }
    }

    return (fallback);
}

bool LinkSurface::isDiscreteOnPot(uint8_t potId) const
{
    const Link::ViewElement *element = elementOnPot(potId);

    if (element == nullptr) {
        return (false);
    }

    const Control::Type type = (Control::Type)element->nativeType;

    return ((type == Control::Type::List) || (type == Control::Type::Pad));
}

uint16_t LinkSurface::valuesOnPot(uint8_t potId) const
{
    const Link::ViewElement *element = elementOnPot(potId);

    if ((element == nullptr) || element->values.empty()) {
        return (0);
    }

    const Link::ViewValue &value = element->values[0];
    const int32_t span = (int32_t)value.max - (int32_t)value.min;

    if ((span <= 0) || (span > UINT16_MAX)) {
        return (0);
    }

    return ((uint16_t)span);
}

bool LinkSurface::turn(uint8_t potId, int16_t delta)
{
    if (!link.isConnected() || (delta == 0)) {
        return (false);
    }

    const Link::ViewElement *element = elementOnPot(potId);

    if (element == nullptr) {
        return (false);
    }

    if (isCustom(*element)) {
        Link::GestureRecord record = {};

        record.elementId = element->id;
        record.kind = (uint8_t)Link::GestureKind::PotTurn;
        /* PotEvent::Change on the mk2, whose pot events are numbered; the
         * primary replays the number into the script. */
        record.event = 2;
        record.potId = potId;
        record.delta = delta;

        link.sendGesture(record);

        return (true);
    }

    if (element->values.empty()) {
        return (false);
    }

    const Link::ViewValue &value = element->values[0];

    if (!value.known) {
        return (false);
    }

    int32_t wanted = value.current + delta;

    if (wanted < value.min) {
        wanted = value.min;
    } else if (wanted > value.max) {
        wanted = value.max;
    }

    if (wanted == value.current) {
        return (true);
    }

    link.sendValue(element->id, value.handle, wanted);

    return (true);
}

bool LinkSurface::potTouch(uint8_t potId, bool down)
{
    if (!link.isConnected()) {
        return (false);
    }

    const Link::ViewElement *element = elementOnPot(potId);

    if ((element == nullptr) || !isCustom(*element)) {
        return (false);
    }

    Link::GestureRecord record = {};

    record.elementId = element->id;
    record.kind = (uint8_t)Link::GestureKind::PotTouch;
    /* PotEvent::Down is 1 and Up is 3 on the mk2. */
    record.event = (uint8_t)(down ? 1 : 3);
    record.potId = potId;

    link.sendGesture(record);

    return (true);
}
