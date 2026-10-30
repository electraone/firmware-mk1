#pragma once

#include "Selection.h"
#include "PageSelectionWindowDelegate.h"
#include "Model/Page.h"
#include "UiApi.h"

class PageSelection : public Selection
{
public:
    PageSelection(PageSelectionWindowDelegate &newDelegate,
                  Pages newPages,
                  uint8_t newActivePage,
                  UiApi &newUiApi,
                  uint32_t newColour,
                  uint32_t newActiveColour);
    virtual ~PageSelection() = default;

private:
    void setActivePage(uint8_t newActivePage);

    Pages pages;
    UiApi &uiApi;
    PageSelectionWindowDelegate &delegate;
};
