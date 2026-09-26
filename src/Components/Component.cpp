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

/**
 * @file Component.cpp
 *
 * @brief A base class for all components.
 */

#include "Component.h"
#include "System.h"
#include "Window.h"
#include "helpers.h"

Component::Component()
    : name(""), id(0), parentComponent(nullptr), visible(false), dimmed(false)
{
}

Component::Component(Component *newParent)
    : name(""), id(0), parentComponent(newParent), visible(false), dimmed(false)
{
}

Component::Component(const char *newName)
    : name(newName),
      id(0),
      parentComponent(nullptr),
      visible(false),
      dimmed(false)
{
}

Component::~Component()
{
    /* A component that is going away must not be left in the repaint queue:
     * the queue holds bare pointers, and the repaint task walks it a whole
     * frame later. Until now only SnapshotsView knew this, and it dealt with
     * it by emptying the queue from its own destructor - which worked for
     * that one window and for nothing else. */
    System::repaintManager.invalidateEntries(this);

    setParentComponent(nullptr);
    releasePot();
    deleteAllChildren();
}

void Component::setId(uint16_t newId)
{
    id = newId;
}

uint16_t Component::getId(void) const
{
    return (id);
}

void Component::setName(const char *newName)
{
    name = newName;
}

const char *Component::getName(void) const
{
    return (name);
}

Component *Component::getParentComponent(void) const
{
    return (parentComponent);
}

// \todo this needs better implementation. It is not the right assumption
// that the top parent component is a window. ActiveWindow should be used
// instead.
Window *Component::getWindow(void) const
{
    Component *component = getParentComponent();
    Component *topComponent = component;

    if (!component) {
        return (nullptr);
    }

    do {
        topComponent = component;
    } while ((component = component->getParentComponent()) != nullptr);

    return (static_cast<Window *>(topComponent));
}

void Component::setBounds(const Rectangle &rect)
{
    Rectangle::setBounds(rect);
    moved();
    resized();
}

void Component::setBounds(uint16_t newX,
                          uint16_t newY,
                          uint16_t newWidth,
                          uint16_t newHeight)
{
    Rectangle::setBounds(newX, newY, newWidth, newHeight);
    moved();
    resized();
}

void Component::setPosition(uint16_t newX, uint16_t newY)
{
    Rectangle::setPosition(newX, newY);
    moved();
}

void Component::setSize(uint16_t newWidth, uint16_t newHeight)
{
    Rectangle::setSize(newWidth, newHeight);
    resized();
}

void Component::setX(uint16_t newX)
{
    Rectangle::setX(newX);
    moved();
}

void Component::setY(uint16_t newY)
{
    Rectangle::setY(newY);
    moved();
}

void Component::setWidth(uint16_t newWidth)
{
    Rectangle::setWidth(newWidth);
    resized();
}

void Component::setHeight(uint16_t newHeight)
{
    Rectangle::setHeight(newHeight);
    resized();
}

void Component::setCentre(uint16_t newCentreX, uint16_t newCentreY)
{
    Rectangle::setCentre(newCentreX, newCentreY);
    moved();
}

void Component::setVisible(bool shouldBeVisible)
{
    bool chg = visible != shouldBeVisible;
    visible = shouldBeVisible;

    if (chg) {
        visibilityChanged();
    }
}

bool Component::isVisible(void) const
{
    return (visible);
}

bool Component::shouldBeDisplayed(void) const
{
    for (const Component *c = this; c != nullptr; c = c->getParentComponent()) {
        if (!c->isVisible()) {
            return (false);
        }
    }

    return (true);
}

void Component::setDimmed(bool shouldBeDimmed)
{
    dimmed = shouldBeDimmed;
}

bool Component::isDimmed(void) const
{
    return (dimmed);
}

/** @todo: Get rid of using global System::repaintManager. */
void Component::repaint(void)
{
    if (shouldBeDisplayed()) {
        System::repaintManager.scheduleRepaint(this);
    }
}

void Component::clear(void)
{
    System::repaintManager.scheduleClear(getScreenBounds());
}

uint16_t Component::getScreenX(void) const
{
    if (!parentComponent) {
        return (getX());
    } else {
        return (parentComponent->getScreenX() + getX());
    }
}

uint16_t Component::getScreenY(void) const
{
    if (!parentComponent) {
        return (getY());
    } else {
        return (parentComponent->getScreenY() + getY());
    }
}

void Component::resized(void)
{
}

void Component::moved(void)
{
}

bool Component::hitTest(uint16_t x, uint16_t y)
{
    return (
        isInside(x, y, getScreenX(), getScreenY(), getWidth(), getHeight()));
}

void Component::visibilityChanged(void)
{
    if (visible) {
        repaint();
    } else {
        auto parent = getParentComponent();

        if (parent) {
            parent->repaint();
        }
    }
}

void Component::addChildComponent(Component *component)
{
    component->parentComponent = this;
    components.push_back(component);
}

void Component::addAndMakeVisible(Component *component)
{
    addChildComponent(component);
    component->setVisible(true);
}

void Component::removeComponent(Component *component)
{
    auto it = std::find(components.begin(), components.end(), component);

    if (it != components.end()) {
        components.erase(it);
    }
}

void Component::deleteComponent(Component *component)
{
    auto it = std::find(components.begin(), components.end(), component);

    if (it != components.end()) {
        delete *it;
        components.erase(it);
    }
}

void Component::removeAllChildren(void)
{
    while (!components.empty()) {
        components.clear();
    }
}

void Component::deleteAllChildren(void)
{
    while (!components.empty()) {
        const Component *component = components.back();
        components.pop_back();
        delete component;
    }

    components = std::vector<Component *>();
}

const std::vector<Component *> &Component::getChildren(void)
{
    return (components);
}

Component *Component::getChildComponent(uint16_t index)
{
    if (index < components.size()) {
        return (components[index]);
    }
    return (nullptr);
}

Component *Component::findChildById(uint16_t componentId)
{
    for (auto &component : components) {
        if (component->getId() == componentId) {
            return (component);
        }
    }

    return (nullptr);
}

int Component::getNumChildComponents(void) const
{
    return (components.size());
}

Rectangle Component::getLocalBounds(void) const
{
    return (Rectangle(0, 0, getWidth(), getHeight()));
}

Rectangle Component::getScreenBounds(void) const
{
    return (Rectangle(getScreenX(), getScreenY(), getWidth(), getHeight()));
}

void Component::setParentComponent(Component *newParent)
{
    parentComponent = newParent;
}

bool Component::canBeSeen(Component *upper, Component *lower)
{
    return ((lower->getX() < upper->getX()) || (lower->getY() < upper->getY())
            || ((lower->getX() + lower->getWidth())
                > (upper->getX() + upper->getWidth()))
            || ((lower->getY() + lower->getHeight())
                > (upper->getY() + upper->getHeight())));
}

bool Component::isInside(uint16_t pointX,
                         uint16_t pointY,
                         uint16_t areaX,
                         uint16_t areaY,
                         uint16_t areaWidth,
                         uint16_t areaHeight)
{
    return ((pointX > areaX) && (pointX < (areaX + areaWidth))
            && (pointY > areaY) && (pointY < (areaY + areaHeight)));
}

bool Component::isOverlaping(Component *first, Component *second)
{
    uint16_t l1x = first->getX();
    uint16_t l1y = first->getY();
    uint16_t r1x = first->getRight();
    uint16_t r1y = first->getBottom();
    uint16_t l2x = second->getX();
    uint16_t l2y = second->getY();
    uint16_t r2x = second->getRight();
    uint16_t r2y = second->getBottom();

    // If one rectangle is on left side of other
    if (l1x >= r2x || l2x >= r1x) {
        return (false);
    }

    // If one rectangle is above other
    if (l1y <= r2y || l2y <= r1y) {
        return (false);
    }

    return (true);
}

bool Component::isWindow(void)
{
    return (dynamic_cast<Window *>(this));
}
