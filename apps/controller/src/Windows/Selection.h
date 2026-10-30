#pragma once

#include "Component.h"
#include "Button.h"

class Selection : public Component
{
public:
    Selection(uint8_t newActive, uint32_t newColour, uint32_t newActiveColour);
    virtual ~Selection() = default;

    virtual void paint(Graphics &g) override;
    virtual void resized(void) override;

protected:
    uint8_t active;
    uint32_t colour;
    uint32_t activeColour;
    Button *button[12];
};
