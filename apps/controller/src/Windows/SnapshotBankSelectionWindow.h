#pragma once

#include "PopupWindow.h"
#include "SnapshotBankSelection.h"
#include "SnapshotsWindowDelegate.h"

class SnapshotBankSelectionWindow : public PopupWindow
{
private:
    SnapshotBankSelectionWindow(SnapshotsWindowDelegate &newDelegate,
                                uint8_t bankNumber)
        : PopupWindow(new SnapshotBankSelection(newDelegate,
                                                bankNumber,
                                                colour,
                                                activeColour),
                      colour,
                      activeColour),
          delegate(newDelegate)
    {
        setName("SNAPSHOT BANKS");
        setBounds(0, 333, 1024, 235);
    }

public:
    virtual ~SnapshotBankSelectionWindow() = default;

    void onButtonUp(uint8_t buttonId)
    {
        if (buttonId == 3) {
            System::windowManager.listWindows();
            buttonBroadcaster.listListeners();
        } else if (buttonId == 5) {
            delegate.closeSnapshotBanks();
        }
    }

    // Factory function
    static SnapshotBankSelectionWindow *
        createSnapshotBankSelectionWindow(SnapshotsWindowDelegate &newDelegate,
                                          uint8_t bankNumber)
    {
        return new SnapshotBankSelectionWindow(newDelegate, bankNumber);
    }

private:
    SnapshotsWindowDelegate &delegate;

    static constexpr uint32_t colour = 0x00091A;
    static constexpr uint32_t activeColour = 0x001130;
};
