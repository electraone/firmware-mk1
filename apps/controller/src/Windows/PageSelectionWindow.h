#pragma once

#include "PopupWindow.h"
#include "PageSelection.h"
#include "PageSelectionWindowDelegate.h"
#include "Model/Page.h"
#include "UiApi.h"

class PageSelectionWindow : public PopupWindow,
                            public PageSelectionWindowDelegate
{
private:
    PageSelectionWindow(Pages pages, uint8_t activePage, UiApi &newUiApi)
        : PopupWindow(new PageSelection(*this,
                                        pages,
                                        activePage,
                                        newUiApi,
                                        colour,
                                        activeColour),
                      colour,
                      activeColour),
          uiApi(newUiApi),
          pagesBrowsed(false)
    {
        setName("PAGES");
        setBounds(0, 333, 1024, 235);
    }

public:
    virtual ~PageSelectionWindow() = default;

    void onButtonDown(uint8_t buttonId) override
    {
        if (buttonId == 2) {
            uiApi.switchOff();
        } else if (buttonId == 3) {
            uiApi.pageSelection_openUsbHostPorts();
        } else if (buttonId == 4) {
            uiApi.pageSelection_openPresetSelection();
        } else if (buttonId == 5) {
            uiApi.pageSelection_close();
        }
    }

    void onButtonUp(uint8_t buttonId)
    {
        if (buttonId == 5 && pagesBrowsed) {
            uiApi.pageSelection_close();
        }
    }

    void userBrowsedPages(void) override
    {
        pagesBrowsed = true;
    }

    // Factory function
    static PageSelectionWindow *createPageSelectionWindow(Pages pages,
                                                          uint8_t activePage,
                                                          UiApi &newUiApi)
    {
        return new PageSelectionWindow(pages, activePage, newUiApi);
    }

private:
    UiApi &uiApi;

    bool pagesBrowsed;

    static constexpr uint32_t colour = 0x00091A;
    static constexpr uint32_t activeColour = 0x001130;
};
