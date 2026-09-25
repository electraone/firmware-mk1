#include "SnapshotsView.h"
#include "Database.h"

SnapshotsView::SnapshotsView(UiApi &newUiApi,
                             const char *newProjectId,
                             uint8_t newBankNumber,
                             Mode newMode)
    : uiApi(newUiApi),
      currentProjectId(newProjectId),
      currentBankNumber(newBankNumber),
      mode(newMode),
      loadButton(nullptr),
      loadAndStayButton(nullptr),
      sendCurrentButton(nullptr),
      removeButton(nullptr),
      saveButton(nullptr),
      snapsButton{ nullptr }
{
    setName("Snapshots");
    addActionButtons();
    addSnapshotButtons();
    setMode(newMode);
    setBounds(0, 0, 1024, 546);
}

SnapshotsView::~SnapshotsView()
{
    System::tasks.disableRepaintGraphics();
    System::tasks.clearRepaintGraphics();
    System::tasks.enableRepaintGraphics();
}

void SnapshotsView::paint(Graphics &g)
{
    g.fillAll(Colours565::black);
}

void SnapshotsView::resized(void)
{
    loadButton->setBounds(12, 25, 154, 50);
    loadAndStayButton->setBounds(181, 25, 154, 50);
    sendCurrentButton->setBounds(442, 25, 154, 50);
    removeButton->setBounds(688, 25, 154, 50);
    saveButton->setBounds(857, 25, 154, 50);

    for (uint16_t x = 0; x < 6; x++) {
        for (uint16_t y = 0; y < 6; y++) {
            snapsButton[y * 6 + x]->setBounds(
                x * 169 + 14, y * 74 + 100, 150, 55);
        }
    }
}

void SnapshotsView::snapshotRemoved(uint8_t bankNumber, uint8_t slot)
{
    if (bankNumber == currentBankNumber) {
        uint16_t id = slot;
        snapsButton[id]->setUsed(false);
        repaint();
    }
}

void SnapshotsView::snapshotSaved(uint8_t bankNumber,
                                  uint8_t slot,
                                  const char *name,
                                  uint32_t colour)
{
    if (bankNumber == currentBankNumber) {
        uint16_t id = slot;
        snapsButton[id]->setLabel(name);
        snapsButton[id]->setColour(
            Colours565::darker(Colours888::toRGB565(colour), 0.5f));
        snapsButton[id]->setUsed(true);
        repaint();
    }
}

void SnapshotsView::snapshotsSwapped(uint8_t sourceBankNumber,
                                     uint8_t sourceSlot,
                                     uint8_t destBankNumber,
                                     uint8_t destSlot)
{
    updateSnapsButtons();
    repaint();
}

ActionButton *SnapshotsView::addButton(uint16_t id,
                                       const char *label,
                                       uint32_t colour,
                                       uint32_t colourActive)
{
    ActionButton *button = new ActionButton();

    if (button) {
        button->setId(id);
        button->setName(label);
        button->setLabel(label);
        button->setColours(colour, colourActive);
        addAndMakeVisible(button);
    }
    return (button);
}

void SnapshotsView::addActionButtons(void)
{
    loadButton = addButton(101, "LOAD", 0x07021a, 0x00053c);
    if (loadButton) {
        loadButton->onClick = [this]() {
            System::logger.write(LOG_ERROR,
                                 "SnapshotsView: switch to mode load");
            setMode(load);
            repaint();
            return (true);
        };
    }

    loadAndStayButton = addButton(102, "LOAD & STAY", 0x07021a, 0x00053c);
    if (loadAndStayButton) {
        loadAndStayButton->onClick = [this]() {
            System::logger.write(LOG_ERROR,
                                 "SnapshotsView: switch to mode loadAndStay");
            setMode(loadAndStay);
            repaint();
            return (true);
        };
    }

    sendCurrentButton = addButton(103, "SEND CURRENT", 0x162800, 0x162800);
    if (sendCurrentButton) {
        sendCurrentButton->onClick = [this]() {
            System::logger.write(LOG_ERROR,
                                 "SnapshotsView: send saved snapshot messages");
            uiApi.sendAllSnapshotValues();
            return (false);
        };
    }

    removeButton = addButton(104, "REMOVE", 0x270800, 0x300a00);
    if (removeButton) {
        removeButton->onClick = [this]() {
            System::logger.write(LOG_ERROR,
                                 "SnapshotsView: switch to mode remove");
            setMode(remove);
            repaint();
            return (true);
        };
    }

    saveButton = addButton(105, "SAVE CURRENT", 0x270800, 0x300a00);

    if (saveButton) {
        saveButton->onClick = [this]() {
            System::logger.write(LOG_ERROR,
                                 "SnapshotsView: switch to mode save");
            setMode(save);
            repaint();
            return (true);
        };
    }
}

void SnapshotsView::setMode(Mode newMode)
{
    mode = newMode;

    loadButton->setSelected(mode == load ? true : false);
    loadAndStayButton->setSelected(mode == loadAndStay ? true : false);
    removeButton->setSelected(mode == remove ? true : false);
    saveButton->setSelected(mode == save ? true : false);
}

void SnapshotsView::addSnapshotButtons(void)
{
    for (uint8_t i = 0; i < 36; i++) {
        snapsButton[i] = new SnapsButton();

        if (snapsButton[i]) {
            auto button = snapsButton[i];
            char programInfo[SnapsButton::maxProgramInfoLength + 1];
            sprintf(programInfo, "%d-%02d", currentBankNumber + 1, i + 1);

            uint16_t id = (currentBankNumber * 36) + i;

            button->setId(id);
            button->setName(programInfo);
            button->setProgramInfo(programInfo);
            button->setBankNumber(currentBankNumber);
            button->setUsed(false);

            button->onClick = [this, i]() {
                runAction(i);
            };
        }
        addAndMakeVisible(snapsButton[i]);
    }

    updateSnapsButtons();
}

void SnapshotsView::updateSnapsButtons(void)
{
    SnapshotRecord snapRec;
    char snapshotFilename[MAX_FILENAME_LENGTH + 1];

    snprintf(snapshotFilename,
             MAX_FILENAME_LENGTH,
             "ctrlv2/snaps/%s/snapshot.db",
             currentProjectId);
    Database dbSnapshot(snapshotFilename);

    if (!dbSnapshot.open()) {
        System::logger.write(
            LOG_ERROR, "SnapsWindow::paint: cannot open the snapshot database");
        return;
    }

    for (uint8_t i = 0; i < 36; i++) {
        if (snapsButton[i]) {
            uint16_t id = (currentBankNumber * 36) + i;
            auto button = snapsButton[i];

            if (dbSnapshot.select(id, DB_RECORD snapRec)) {
                button->setUsed(true);
                button->setLabel(snapRec.name);
                button->setColour(Colours565::darker(
                    Colours888::toRGB565(snapRec.colour), 0.5f));
            } else {
                button->setUsed(false);
            }
        }
    }
    dbSnapshot.close();
}

void SnapshotsView::runAction(uint8_t slot)
{
    switch (mode) {
        case SnapshotsView::load:
            loadSnapshot(slot);
            break;

        case SnapshotsView::loadAndStay:
            loadAndStaySnapshot(slot);
            break;

        case SnapshotsView::remove:
            removeSnapshot(slot);
            break;

        case SnapshotsView::save:
            saveSnapshot(slot);
            break;

        default:
            break;
    }
}

void SnapshotsView::loadSnapshot(uint8_t slot)
{
    uiApi.loadSnapshot(currentProjectId, currentBankNumber, slot);
}

void SnapshotsView::loadAndStaySnapshot(uint8_t slot)
{
    uiApi.loadAndStaySnapshot(currentProjectId, currentBankNumber, slot);
}

void SnapshotsView::removeSnapshot(uint8_t slot)
{
    uiApi.removeSnapshot(currentProjectId, currentBankNumber, slot);
}

void SnapshotsView::saveSnapshot(uint8_t slot)
{
    uint32_t colour = defaultColours[slot % 6];
    char name[20 + 1];
    sprintf(name, "%c%d", 65 + slot % 6, slot / 6);
    uiApi.saveSnapshot(currentProjectId, currentBankNumber, slot, name, colour);
}
