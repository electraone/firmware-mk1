#include "Selection.h"

Selection::Selection(uint8_t newActive,
                     uint32_t newColour,
                     uint32_t newActiveColour)
    : active(newActive),
      colour(newColour),
      activeColour(newActiveColour),
      button{ nullptr }
{
    setName("Selection");

    for (uint8_t i = 0; i < 12; i++) {
        button[i] = new Button();

        if (button[i]) {
            button[i]->setId(i);
            button[i]->setColours(colour, activeColour);
            button[i]->assignPot(i);

            if (active == i) {
                button[i]->setSelected(true);
            }
            addAndMakeVisible(button[i]);
        }
    }
}

void Selection::paint(Graphics &g)
{
    g.fillAll(Colours565::black);
}

void Selection::resized(void)
{
    uint16_t segmentWidth = getWidth() / 6 - 2;
    constexpr uint16_t segmentHeight = 70;
    uint16_t baseY = getHeight() - (2 * segmentHeight) - 25;

    for (uint8_t i = 0; i < 12; i++) {
        uint16_t x = (i % 6) * segmentWidth;
        uint16_t y = (i < 6) ? baseY : baseY + segmentHeight + 10;
        button[i]->setBounds(x + 12, y, segmentWidth - 10, segmentHeight);
    }
}
