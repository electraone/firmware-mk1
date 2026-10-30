#pragma once

#include "Window.h"
#include "SnapshotsView.h"
#include "SnapshotBankSelectionWindow.h"
#include "Preset.h"
#include "SnapshotsWindowDelegate.h"
#include "UiApi.h"

class SnapshotsWindow : public Window, public SnapshotsWindowDelegate
{
private:
    SnapshotsWindow(UiApi &newUiApi,
                    const char *newProjectId,
                    uint8_t newBankNumber)
        : uiApi(newUiApi),
          snapshotsView(nullptr),
          snapshotBankSelectionWindow(nullptr),
          currentSnapshotBank(newBankNumber),
          modeIndex(0)
    {
        copyString(projectId, newProjectId, Preset::MaxProjectIdLength);

        snapshotsView = new SnapshotsView(
            newUiApi, newProjectId, newBankNumber, modes[modeIndex]);

        if (snapshotsView) {
            setOwnedContent(snapshotsView);
        }
        setName("SnapshotsWindow");
        setVisible(true);
    }

public:
    virtual ~SnapshotsWindow() = default;

    // API
    void openSnapshotBanks(void) override
    {
        snapshotBankSelectionWindow =
            SnapshotBankSelectionWindow::createSnapshotBankSelectionWindow(
                *this, currentSnapshotBank);
    }

    void closeSnapshotBanks(void) override
    {
        Window::close(snapshotBankSelectionWindow);
        snapshotBankSelectionWindow = nullptr;
    }

    void switchSnapshotBank(uint8_t newBankNumber) override
    {
        uiApi.switchSnapshotBank(newBankNumber);
    }

    void switchMode(void) override
    {
        modeIndex = (modeIndex < 3) ? modeIndex + 1 : 0;
        snapshotsView->setMode(modes[modeIndex]);
    }

    void snapshotRemoved(uint8_t bankNumber, uint8_t slot) override
    {
        snapshotsView->snapshotRemoved(bankNumber, slot);
    }

    void snapshotSaved(uint8_t bankNumber,
                       uint8_t slot,
                       const char *name,
                       uint32_t colour) override
    {
        snapshotsView->snapshotSaved(bankNumber, slot, name, colour);
    }

    void snapshotsSwapped(uint8_t sourceBankNumber,
                          uint8_t sourceSlot,
                          uint8_t destBankNumber,
                          uint8_t destSlot) override
    {
        snapshotsView->snapshotsSwapped(
            sourceBankNumber, sourceSlot, destBankNumber, destSlot);
    }

    void snapshotBankSwitched(uint8_t newBankNumber) override
    {
        currentSnapshotBank = newBankNumber;

        if (snapshotsView) {
            delete snapshotsView;
        }
        snapshotsView = new SnapshotsView(
            uiApi, projectId, newBankNumber, modes[modeIndex]);
        replaceOwnedContent(snapshotsView);

        if (snapshotBankSelectionWindow) {
            snapshotBankSelectionWindow->repaint();
        }
    }

    // Events
    void onButtonDown(uint8_t buttonId) override
    {
        if (buttonId == 3) {
            switchMode();
        } else if (buttonId == 4) {
            uiApi.snapshots_close();
        } else if (buttonId == 5) {
            openSnapshotBanks();
        }
    }

    // Factory function
    static SnapshotsWindow *createSnapshotsWindow(UiApi &newUiApi,
                                                  const char *newProjectId,
                                                  uint8_t newBankNumber)
    {
        return new SnapshotsWindow(newUiApi, newProjectId, newBankNumber);
    }

private:
    UiApi &uiApi;
    char projectId[Preset::MaxProjectIdLength + 1];
    SnapshotsView *snapshotsView;

    // Sub-windows
    SnapshotBankSelectionWindow *snapshotBankSelectionWindow;

    uint8_t currentSnapshotBank;
    uint8_t modeIndex;
    static constexpr SnapshotsView::Mode modes[4] = {
        SnapshotsView::Mode::load,
        SnapshotsView::Mode::loadAndStay,
        SnapshotsView::Mode::remove,
        SnapshotsView::Mode::save
    };
};
