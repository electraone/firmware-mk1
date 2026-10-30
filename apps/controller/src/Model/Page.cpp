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

#include "Page.h"

Page::Page()
    : id(0), defaultControlSetId(0), objectsPresent(false), hidden(false)
{
    *name = '\0';
}

Page::Page(uint8_t id,
           const char *newName,
           uint8_t newDefaultControlSetId,
           bool shouldBeHidden)
    : id(id),
      defaultControlSetId(newDefaultControlSetId),
      objectsPresent(false),
      hidden(shouldBeHidden)
{
    setName(newName);
}

bool Page::isValid(void) const
{
    return (0 < id && id <= 12);
}

uint8_t Page::getId(void) const
{
    return (id);
}

void Page::setName(const char *newName)
{
    if (newName) {
        copyString(name, newName, MaxNameLength);
    } else {
        *name = '\0';
    }
}

const char *Page::getName(void) const
{
    return (name);
}

void Page::setDefaultControlSetId(uint8_t newDefaultControlSetId)
{
    defaultControlSetId = newDefaultControlSetId;
}

uint8_t Page::getDefaultControlSetId(void) const
{
    return (defaultControlSetId);
}

bool Page::hasObjects(void) const
{
    return (objectsPresent);
}

void Page::setHasObjects(bool shouldHaveObjects)
{
    objectsPresent = shouldHaveObjects;
}

bool Page::isHidden(void) const
{
    return (hidden);
}

void Page::setHidden(bool shouldBeHidden)
{
    hidden = shouldBeHidden;
}

bool Page::isToBeDisplayed(void) const
{
    return (!hidden && objectsPresent);
}

void Page::print(uint8_t logLevel) const
{
    System::logger.write(
        logLevel, "--[Page]------------------------------------------------");
    System::logger.write(logLevel, "id: %d", getId());
    System::logger.write(logLevel, "name: %s", getName());
    System::logger.write(logLevel, "hasObjects: %d", hasObjects());
    System::logger.write(
        logLevel, "defaultControlSetId: %d", getDefaultControlSetId());
    System::logger.write(logLevel, "hidden: %d", isHidden());
}