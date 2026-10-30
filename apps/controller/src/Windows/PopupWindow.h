#pragma once

#include "Window.h"
#include "PresetSelection.h"
#include "Model/Presets.h"
#include "UiApi.h"

class PopupWindow : public Window
{
public:
    PopupWindow(Component *newContent,
                uint32_t newColour,
                uint32_t newActiveColour)
        : content(newContent), colour(newColour), activeColour(newActiveColour)
    {
        if (content) {
            setOwnedContent(content);
        }
        setVisible(true);
    }

    virtual ~PopupWindow() = default;

    virtual void paint(Graphics &g) override
    {
        g.setColour(Colours565::black);
        g.fillRect(0, 40, getWidth(), getHeight() - topPadding);
        paintTitleBar(g, getName(), getWidth(), colour);
        Window::paint(g);
    }

    virtual void resized(void) override
    {
        content->setBounds(0, topPadding, getWidth(), getHeight() - topPadding);
    }

protected:
    static void paintTitleBar(Graphics &g,
                              const char *title,
                              uint16_t width,
                              uint32_t colour)
    {
        g.dim(0, 0, width, 10, Colours565::black);
        g.setColour(Colours888::toRGB565(colour));
        g.fillRect(0, 10, width, 30);
        paintIcon(g, 16, 15);
        g.printText(40,
                    18,
                    title,
                    TextStyle::mediumTransparent,
                    width,
                    TextAlign::left);
    }

    static void paintIcon(Graphics &g, uint16_t x, uint16_t y)
    {
        g.setColour(Colours565::white);
        g.drawRect(x, y, 10, 13);
        g.fillRect(x + 5, y + 3, 10, 13);
    }

private:
    Component *content;
    uint32_t colour;
    uint32_t activeColour;

    static constexpr uint16_t topPadding = 60;
};
