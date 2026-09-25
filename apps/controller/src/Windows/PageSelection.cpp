#include "PageSelection.h"
#include "Window.h"

PageSelection::PageSelection(PageSelectionWindowDelegate &newDelegate,
                             Pages newPages,
                             uint8_t newActivePage,
                             UiApi &newUiApi,
                             uint32_t newColour,
                             uint32_t newActiveColour)
    : Selection(newActivePage - 1, newColour, newActiveColour),
      delegate(newDelegate),
      pages(newPages),
      uiApi(newUiApi)
{
    setName("PageSelection");

    for (uint8_t i = 0; i < 12; i++) {
        uint8_t pageId = i + 1;

        if (button[i]) {
            if (pages[pageId].isToBeDisplayed()) {
                button[i]->setLabel(pages[pageId].getName());
                button[i]->onClick = [this, i]() {
                    setActivePage(i);
                    return (true);
                };
            } else {
                button[i]->releasePot();
            }
        }
    }
}

void PageSelection::setActivePage(uint8_t newActivePage)
{
    uint8_t previousActivePage = active;
    active = newActivePage;
    uiApi.switchPage(active + 1);

    delegate.userBrowsedPages();

    if (!Hardware::buttons[BUTTON_RIGHT_BOTTOM]->isPressed()) {
        uiApi.pageSelection_close();
    } else {
        button[previousActivePage]->setSelected(false);
        button[active]->setSelected(true);
        getWindow()->repaint();
    }
}
