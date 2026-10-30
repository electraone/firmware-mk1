#pragma once

#include "Component.h"

class Lock : public Component
{
private:
    static constexpr uint16_t pinDiameter = 80;

public:
    Lock() : locked(false), highlighted(false)
    {
    }

    Lock(bool shouldBeLocked) : locked(shouldBeLocked), highlighted(false)
    {
    }

    Lock(Rectangle newBounds) : locked(false)
    {
        setBounds(newBounds);
    }

    virtual ~Lock() = default;

    void setLocked(bool shouldBeLocked)
    {
        locked = shouldBeLocked;
        repaint();
    }

    void onTouchDown(const TouchEvent &touchEvent) override
    {
        setHighlighted(true);
    }

    void onTouchUp(const TouchEvent &touchEvent) override
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

    void paint(Graphics &g) override
    {
        auto bounds = getBounds();

        paintLockOutline(g, bounds, highlighted);

        if (locked) {
            paintCross(g, bounds);
        } else {
            paintLock(g, bounds);
        }
    }

    std::function<void(void)> onClick;

private:
    static void paintCross(Graphics &g, const Rectangle &bounds)
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

    static void paintLock(Graphics &g, const Rectangle &bounds)
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
        g.fillRect(pinSymbolX + quaterOfRadius - 2,
                   pinSymbolY + quaterOfRadius - 4,
                   5,
                   8);
    }

    static void paintLockOutline(Graphics &g,
                                 const Rectangle &bounds,
                                 bool isHighlighted)
    {
        uint16_t radius = bounds.getWidth() / 2;

        g.setColour(isHighlighted ? 0x7000 : 0x2000);
        g.fillElipse(radius, radius, radius, radius);
    }

    void setHighlighted(bool shouldBeHighligthed)
    {
        highlighted = shouldBeHighligthed;
        repaint();
    }

    bool locked;
    bool highlighted;
};
