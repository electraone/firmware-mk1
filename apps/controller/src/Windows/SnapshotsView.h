#pragma once

#include "Component.h"
#include "SnapsButton.h"
#include "ActionButton.h"
#include "UiApi.h"
#include "System.h"

class SnapshotsView : public Component
{
public:
    enum Mode { load, loadAndStay, save, remove };

    SnapshotsView(UiApi &newUiApi,
                  const char *newProjectId,
                  uint8_t newBankNumber,
                  Mode newMode);
    virtual ~SnapshotsView();

    void paint(Graphics &g) override;
    void resized(void) override;
    void setMode(Mode newMode);
    void snapshotRemoved(uint8_t bankNumber, uint8_t slot);
    void snapshotSaved(uint8_t bankNumber,
                       uint8_t slot,
                       const char *name,
                       uint32_t colour);
    void snapshotsSwapped(uint8_t sourceBankNumber,
                          uint8_t sourceSlot,
                          uint8_t destBankNumber,
                          uint8_t destSlot);

private:
    static void paintTitleBar(Graphics &g,
                              const char *title,
                              uint16_t width,
                              uint32_t colour);
    static void paintIcon(Graphics &g, uint16_t x, uint16_t y);
    void updateSnapsButtons(void);
    ActionButton *addButton(uint16_t id,
                            const char *label,
                            uint32_t colour,
                            uint32_t colourActive);
    void addActionButtons(void);
    void addSnapshotButtons(void);
    void runAction(uint8_t slot);
    void loadSnapshot(uint8_t slot);
    void loadAndStaySnapshot(uint8_t slot);
    void removeSnapshot(uint8_t slot);
    void saveSnapshot(uint8_t slot);

    UiApi &uiApi;
    const char *currentProjectId;
    uint8_t currentBankNumber;
    Mode mode;
    ActionButton *loadButton;
    ActionButton *loadAndStayButton;
    ActionButton *sendCurrentButton;
    ActionButton *removeButton;
    ActionButton *saveButton;

    SnapsButton *snapsButton[36];

    static constexpr uint8_t topPadding = 50;
    static constexpr uint32_t defaultColours[] = {
        0xFFFFFF, 0xF45C51, 0xF49500, 0x529DEC, 0x03A598, 0xC44795
    };
};
