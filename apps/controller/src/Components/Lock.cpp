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

#include "Lock.h"

Lock::Lock() : locked(false), highlighted(false)
{
}

Lock::Lock(bool shouldBeLocked) : locked(shouldBeLocked), highlighted(false)
{
}

Lock::Lock(Rectangle newBounds) : locked(false), highlighted(false)
{
    setBounds(newBounds);
}

void Lock::setLocked(bool shouldBeLocked)
{
    locked = shouldBeLocked;
    repaint();
}

void Lock::onTouchDown([[maybe_unused]] const TouchEvent &touchEvent)
{
    setHighlighted(true);
}

void Lock::onTouchUp([[maybe_unused]] const TouchEvent &touchEvent)
{
    if (highlighted) {
        setHighlighted(false);
    }

    if (!locked) {
        setLocked(true);
    }

    if (onClick) {
        onClick();
    }
}

void Lock::paint(Graphics &g)
{
    auto bounds = getBounds();

    paintLockOutline(g, bounds, highlighted);

    if (locked) {
        paintCross(g, bounds);
    } else {
        paintLock(g, bounds);
    }
}

void Lock::paintCross(Graphics &g, const Rectangle &bounds)
{
    uint16_t radius = bounds.getWidth() / 2;
    uint16_t quaterOfRadius = radius / 4;

    g.setColour(Colours565::white);
    g.drawLine(
        radius, radius, radius + quaterOfRadius, radius + quaterOfRadius);
    g.drawLine(
        radius, radius, radius - quaterOfRadius, radius + quaterOfRadius);
    g.drawLine(
        radius, radius, radius + quaterOfRadius, radius - quaterOfRadius);
    g.drawLine(
        radius, radius, radius - quaterOfRadius, radius - quaterOfRadius);
}

void Lock::paintLock(Graphics &g, const Rectangle &bounds)
{
    uint16_t radius = bounds.getWidth() / 2;
    uint16_t quaterOfRadius = radius / 4;
    uint16_t halfOfRadius = radius / 2;
    uint16_t pinSymbolX = radius - (quaterOfRadius);
    uint16_t pinSymbolY = radius - (quaterOfRadius) + (radius / 8) - 2;

    g.setColour(Colours565::lightcoral);
    g.drawElipse(pinSymbolX + (quaterOfRadius),
                 pinSymbolY,
                 (quaterOfRadius),
                 (quaterOfRadius));
    g.drawElipse(pinSymbolX + (quaterOfRadius),
                 pinSymbolY,
                 (quaterOfRadius - 1),
                 (quaterOfRadius - 1));
    g.fillRect(pinSymbolX, pinSymbolY, halfOfRadius + 1, halfOfRadius);
    g.setColour(Colours565::black);
    g.fillRect(
        pinSymbolX + quaterOfRadius - 2, pinSymbolY + quaterOfRadius - 4, 5, 8);
}

void Lock::paintLockOutline(Graphics &g,
                            const Rectangle &bounds,
                            bool isHighlighted)
{
    uint16_t radius = bounds.getWidth() / 2;

    g.setColour(isHighlighted ? 0x7000 : 0x2000);
    g.fillElipse(radius, radius, radius, radius);
}

void Lock::setHighlighted(bool shouldBeHighligthed)
{
    highlighted = shouldBeHighligthed;
    repaint();
}
