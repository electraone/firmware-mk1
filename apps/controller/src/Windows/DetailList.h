#pragma once

#include "Detail.h"

class DetailList : public Detail
{
public:
    DetailList(const Control &control, MainDelegate &newDelegate)
        : Detail(control, newDelegate)
    {
    }

    virtual ~DetailList() = default;

    void paint(Graphics &g)
    {
        uint16_t xPosition = 0;
        uint16_t xPositionDim = 0;

        if (delegate.isDetailOnTheLeft()) {
            xPosition = 0;
            xPositionDim = getWidth() - 100;
        } else {
            xPosition = 100;
            xPositionDim = 0;
        }

        g.setColour(Colours565::black);
        g.fillRect(xPosition, 0, getWidth() - 100, getHeight());
        g.setColour(Colours565::darkgrey);
        g.drawRect(xPosition, 0, getWidth() - 100, getHeight());
        g.dim(xPositionDim, 0, 100, getHeight(), Colours565::black);
    }

    void resized(void)
    {
        if (delegate.isDetailOnTheLeft()) {
            component->setBounds(8, 8, getWidth() - 116, getHeight() - 10);
            lock->setBounds(getWidth() - 90, getHeight() - 100, 80, 80);
            dimmedSection->setBounds(
                getWidth() - 100, 0, 100, getHeight() - 100);
        } else {
            component->setBounds(108, 8, getWidth() - 116, getHeight() - 10);
            lock->setBounds(0, getHeight() - 100, 80, 80);
            dimmedSection->setBounds(0, 0, 100, getHeight() - 100);
        }
    }

private:
};
