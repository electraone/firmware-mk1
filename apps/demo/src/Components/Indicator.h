#pragma once

#include "Component.h"

class Indicator final : public Component
{
public:
    Indicator()
    {
        setName("indicator");
    }

    void paint(Graphics &g) override
    {
        g.dim(0, 0, getWidth(), 100, 0x0007);
        g.setColour(Colours565::white);
        g.fillTriangle(getWidth() / 2,
                       getHeight() - 30,
                       getWidth() / 2 - 10,
                       getHeight() - 20,
                       getWidth() / 2 + 10,
                       getHeight() - 20);
        g.system().printText(0,
                             getHeight() - 13,
                             "Touch Me",
                             TextStyle::smallTransparent,
                             getWidth(),
                             TextAlign::center);
    }

    void onTouchDown(const TouchEvent &touchEvent) override
    {
        System::logger.write(LOG_ERROR, "callback: indicator");
    }
};
