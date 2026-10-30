#pragma once

#include "Detail.h"

class DetailFader : public Detail
{
public:
    DetailFader(const Control &control, MainDelegate &newDelegate)
        : Detail(control, newDelegate)
    {
    }

    virtual ~DetailFader() = default;

    void paint(Graphics &g)
    {
        g.setColour(Colours565::black);
        g.fillRect(0, 0, getWidth(), getHeight() - 120);
        g.setColour(Colours565::darkgrey);
        g.drawRect(0, 0, getWidth(), getHeight() - 120);
        g.dim(0, getHeight() - 120, getWidth(), 120, Colours565::black);
    }

    void resized(void)
    {
        component->setBounds(58, 45, 908, 120);
        lock->setBounds(58, getHeight() - 100, 80, 80);
        dimmedSection->setBounds(138, getHeight() - 120, getWidth() - 138, 120);
    }

private:
};
