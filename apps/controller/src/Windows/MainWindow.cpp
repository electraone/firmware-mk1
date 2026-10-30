#include "MainWindow.h"
#include "ControlComponent.h"
#include "GroupControl.h"
#include "Envelope.h"
#include "luaExtension.h"
#include "System.h"
#include "SubscribedEvents.h"

MainWindow::MainWindow(Model &newModel, Midi &newMidi, Config &newConfig)
    : model(newModel),
      preset(newModel.currentPreset),
      snapshots(newModel.snapshots),
      presets(newModel.presets),
      midi(newMidi),
      setup(newConfig),
      uiApi(*this),
      pageView(nullptr),
      presetSelectionWindow(nullptr),
      pageSelectionWindow(nullptr),
      detailWindow(nullptr),
      snapshotsWindow(nullptr),
      usbHostWindow(nullptr),
      currentPageId(0),
      currentControlSetId(0),
      currentSnapshotBank(0),
      inSleepMode(false),
      controlPort(USB_MIDI_PORT_CTRL),
      subscribedEvents(0)
{
    setName("mainWindow");
    setBounds(0, 25, 1024, 575);
}

void MainWindow::resized(void)
{
}

void MainWindow::onButtonDown(uint8_t buttonId)
{
    if (getNumActivePotTouch() > 0) {
        if (buttonId == 0) {
            showDetailOfActivePotTouch();
        } else if (buttonId == 1) {
            switchToNextHandleOfActivePotTouch();
        } else if (buttonId == 2) {
            switchToPreviousHandleOfActivePotTouch();
        }
    } else {
        if (0 <= buttonId && buttonId <= 2) {
            uiApi.switchControlSet(buttonId);
        } else if (buttonId == 3) {
            requestAllPatches();
        } else if (buttonId == 4) {
            openSnapshots();
        } else if (buttonId == 5) {
            if (inSleepMode == true) {
                reboot();
            } else {
                openPageSelection();
            }
        }
    }
}

void MainWindow::onButtonUp(uint8_t buttonId)
{
}

void MainWindow::switchOff(void)
{
    if (setup.uiFeatures.keepPresetState
        || setup.uiFeatures.loadPresetStateOnStartup) {
        parameterMap.keep();
    }
    for (int i = 0; i < 16000; i += 128) {
        Hardware::screen.setBacklightbrightness(i);
        delay(10);
    }
    Hardware::screen.setBacklightbrightness(65535);
    System::tasks.enableSleepMode();
    usb_stop();
    usbHost.stop();
    inSleepMode = true;
}

void MainWindow::reboot(void)
{
    Hardware::reset();
}

bool MainWindow::switchPage(uint8_t pageId)
{
    if (preset.getPage(pageId).isToBeDisplayed()) {
        uint8_t controlSet =
            setup.uiFeatures.resetActiveControlSet
                ? preset.pages.at(pageId).getDefaultControlSetId()
                : currentControlSetId;
        return (switchPage(pageId, controlSet));
    }
    return (false);
}

bool MainWindow::switchPage(uint8_t pageId, uint8_t controlSetId)
{
    uint8_t prevCurrentPageId = currentPageId;
    currentPageId = pageId;
    currentControlSetId = controlSetId;

    System::logger.write(LOG_ERROR,
                         "switchPage: Page switched: page=%d, controlSetId=%d",
                         currentPageId,
                         currentControlSetId);

    displayPage();

    if (prevCurrentPageId != currentPageId) {
        if (subscribedEvents & SubscribedEvents::pages) {
            if (L) {
                pages_onChange(currentPageId, prevCurrentPageId);
                events_onPageChange(currentPageId, prevCurrentPageId);
            }
        }
    }
    return (true);
}

bool MainWindow::switchPageNext(void)
{
    for (uint8_t i = (currentPageId + 1); i < 12; i++) {
        if (preset.getPage(i).isToBeDisplayed()) {
            switchPage(i);
            return (true);
        }
    }
    return (false);
}

bool MainWindow::switchPagePrev(void)
{
    for (int8_t i = (currentPageId - 1); i > 0; i--) {
        if (preset.getPage(i).isToBeDisplayed()) {
            switchPage(i);
            return (true);
        }
    }
    return (false);
}

bool MainWindow::switchControlSet(uint8_t controlSetId)
{
    if (0 <= controlSetId && controlSetId <= 2) {
        pageView->setControlSet(controlSetId);
        currentControlSetId = controlSetId;
        return (true);
    }
    return (false);
}

void MainWindow::openDetail(uint16_t controlId)
{
    const Control &control = preset.getControl(controlId);
    if (control.hasDetail()) {
        detailWindow = DetailWindow::createDetailWindow(control, *this);
        detailWindow->setBounds(getDetailBounds(control));
    }
}

void MainWindow::lockDetail(void)
{
    if (detailWindow) {
        detailWindow->setLocked();
    }
}

void MainWindow::closeDetail(void)
{
    Window::close(detailWindow);
    detailWindow = nullptr;
}

void MainWindow::openPresetSelection(void)
{
    presetSelectionWindow = PresetSelectionWindow::createPresetSelectionWindow(
        presets, setup.presetBanks, uiApi);
}

void MainWindow::closePresetSelection(void)
{
    Window::close(presetSelectionWindow);
    presetSelectionWindow = nullptr;
}

void MainWindow::openPageSelection(void)
{
    pageSelectionWindow = PageSelectionWindow::createPageSelectionWindow(
        preset.pages, currentPageId, uiApi);
}

void MainWindow::closePageSelection(void)
{
    Window::close(pageSelectionWindow);
    pageSelectionWindow = nullptr;
}

void MainWindow::openUsbHostPorts(void)
{
    usbHostWindow = UsbHostWindow::createUsbHostWindow(uiApi);
}

void MainWindow::closeUsbHostPorts(void)
{
    Window::close(usbHostWindow);
    usbHostWindow = nullptr;
}

void MainWindow::openSnapshots(void)
{
    snapshotsWindow = SnapshotsWindow::createSnapshotsWindow(
        uiApi, preset.getProjectId(), currentSnapshotBank);
}

void MainWindow::closeSnapshots(void)
{
    Window::close(snapshotsWindow);
    snapshotsWindow = nullptr;
    repaintPage();
}

void MainWindow::repaintPage(void)
{
    repaint();
}

void MainWindow::refreshControl(const Control &control)
{
    Component *c = control.getComponent();

    if (ControlComponent *cc = dynamic_cast<ControlComponent *>(c)) {
        cc->syncComponentProperties();
        cc->repaint();
    }
}

bool MainWindow::loadPreset(LocalFile &file)
{
    System::tasks.enableSpinner();
    switchPreset(presets.getCurrentBankNumber(), presets.getCurrentSlot());
    System::tasks.disableSpinner();

    return (preset.isValid());
}

bool MainWindow::loadLua(LocalFile &file)
{
    if (isLuaValid(System::context.getCurrentLuaFile())) {
        if (preset.isValid()) {
            presets.runUploadedLuaScript();
            closeAllWindows();
            switchPage(1, preset.getPage(1).getDefaultControlSetId());
            return (true);
        }
    }
    return (false);
}

bool MainWindow::loadConfig(LocalFile &file)
{
    return (true);
}

void MainWindow::setControlVisible(uint16_t controlId, bool shouldBeVisible)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        if (control.isVisible() != shouldBeVisible) {
            control.setVisible(shouldBeVisible);
            if (Component *component = control.getComponent()) {
                component->setVisible(shouldBeVisible);

                if (currentControlSetId == control.getControlSetId()) {
                    if (shouldBeVisible) {
                        component->assignPot(control.inputs[0].getPotId(),
                                             control.values[0].getNumSteps());
                    } else {
                        component->releasePot();
                    }
                }
            }
        }
    }
}

void MainWindow::setControlName(uint16_t controlId, const char *newName)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        control.setName(newName);
        refreshControl(control);
    }
}

void MainWindow::setControlColour(uint16_t controlId, uint32_t newColour)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        control.setColour(newColour);
        refreshControl(control);
    }
}

void MainWindow::setControlPot(uint16_t controlId,
                               uint8_t newControlSetId,
                               uint8_t newPotId)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        System::logger.write(LOG_ERROR,
                             "set pot: controlSet=%d, pot=%d",
                             newControlSetId,
                             newPotId);
        control.setControlSetId(newControlSetId - 1);
        control.inputs[0].setPotId(newPotId - 1);

        if (pageView) {
            pageView->reassignComponent(control);
        }
    }
}

void MainWindow::setControlBounds(uint16_t controlId, const Rectangle &bounds)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        control.setBounds(bounds);
        if (Component *component = control.getComponent()) {
            component->setBounds(bounds);
            repaintPage();
        }
    }
}

void MainWindow::setControlSlot(uint16_t controlId, uint8_t newSlot)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        uint8_t newPotId = (newSlot - 1) % Preset::MaxNumPots;
        uint8_t newControlSetId = (newSlot - 1) / Preset::MaxNumPots;
        Rectangle bounds = controlSlotToBounds(newSlot);
        // adjust position from parent group slot
        bounds.setX(bounds.getX() + 6);
        bounds.setY(bounds.getY() + 22);
        bounds.setWidth(bounds.getWidth() - 12);

        control.setBounds(bounds);
        control.setVisible(true);
        control.inputs[0].setPotId(newPotId);
        control.setControlSetId(newControlSetId);

        if (Component *component = control.getComponent()) {
            component->setBounds(bounds);
            component->setVisible(true);

            if (pageView) {
                pageView->reassignComponent(control);
            }
        }
    }
}

void MainWindow::setControlSlot(uint16_t controlId,
                                uint8_t newPageId,
                                uint8_t newSlot)
{
    Control &control = preset.getControl(controlId);
    auto originalPageId = control.getPageId();

    if (control.isValid()) {
        preset.moveControlToSlot(controlId, newPageId, newSlot);

        if (newPageId != originalPageId) {
            if (newPageId == currentPageId) {
                pageView->addControl(control);
            } else {
                pageView->removeControl(control);
            }
        } else {
            pageView->moveControl(control);
        }
    }
}

void MainWindow::setControlValueLabel(uint16_t controlId,
                                      const char *valueId,
                                      const char *text)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        auto handleId = Control::translateValueToId(control.getType(), valueId);
        control.getValue(handleId).setLabel(text);
        refreshControl(control);
    }
}

void MainWindow::setControlValueLabel(uint16_t controlId,
                                      uint8_t handleId,
                                      const char *text)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        handleId = Control::constraintValueId(control.getType(), handleId);
        control.getValue(handleId).setLabel(text);
        refreshControl(control);
    }
}

void MainWindow::setControlValueOverlay(uint16_t controlId,
                                        uint8_t handleId,
                                        uint8_t newOverlayId)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        handleId = Control::constraintValueId(control.getType(), handleId);
        auto &controlValue = control.getValue(handleId);
        controlValue.setOverlay(preset.getOverlay(newOverlayId));
        refreshControl(control);
    }
}

void MainWindow::setControlValueMin(uint16_t controlId,
                                    uint8_t handleId,
                                    int16_t newMin)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        handleId = Control::constraintValueId(control.getType(), handleId);
        auto &controlValue = control.getValue(handleId);
        controlValue.setMin(newMin);
        refreshControl(control);
    }
}

void MainWindow::setControlValueMax(uint16_t controlId,
                                    uint8_t handleId,
                                    int16_t newMax)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        handleId = Control::constraintValueId(control.getType(), handleId);
        auto &controlValue = control.getValue(handleId);
        controlValue.setMax(newMax);
        refreshControl(control);
    }
}

void MainWindow::setControlValueRange(uint16_t controlId,
                                      uint8_t handleId,
                                      int16_t newMin,
                                      int16_t newMax,
                                      int16_t newDefault,
                                      bool updateMesage)
{
    Control &control = preset.getControl(controlId);

    if (control.isValid()) {
        auto &controlValue = control.getValue(handleId);
        controlValue.setMin(newMin);
        controlValue.setMax(newMax);
        controlValue.setDefault(newDefault);
        if (updateMesage) {
            controlValue.message.setMidiMin(newMin);
            controlValue.message.setMidiMax(newMax);
        }
        refreshControl(control);
    }
}

void MainWindow::setGroupLabel(uint16_t groupId, const char *newLabel)
{
    Group &group = preset.getGroup(groupId);
    if (group.isValid()) {
        group.setLabel(newLabel);

        if (strlen(newLabel) == 0) {
            group.setVisible(false);
            if (Component *component = group.getComponent()) {
                component->setVisible(false);
                repaintPage();
            }
        } else {
            if (Component *component = group.getComponent()) {
                if (GroupControl *gc =
                        dynamic_cast<GroupControl *>(component)) {
                    gc->repaint();
                }
            }
        }
    }
}

void MainWindow::setGroupColour(uint16_t groupId, uint32_t newColour)
{
    Group &group = preset.getGroup(groupId);
    if (group.isValid()) {
        group.setColour(newColour);
        if (Component *component = group.getComponent()) {
            component->repaint();
        }
    }
}

void MainWindow::setGroupVisible(uint16_t groupId, bool shouldBeVisible)
{
    Group &group = preset.getGroup(groupId);
    if (group.isValid()) {
        if (group.isVisible() != shouldBeVisible) {
            group.setVisible(shouldBeVisible);
            if (Component *component = group.getComponent()) {
                component->setVisible(shouldBeVisible);
                repaintPage();
            }
        }
    }
}

void MainWindow::setGroupBounds(uint16_t groupId, const Rectangle &bounds)
{
    Group &group = preset.getGroup(groupId);
    if (group.isValid()) {
        group.setBounds(bounds);
        if (Component *component = group.getComponent()) {
            component->setBounds(bounds);
            repaintPage();
        }
    }
}

void MainWindow::setGroupSlot(uint16_t groupId,
                              uint8_t newSlot,
                              uint8_t newWidth,
                              uint8_t newHeight)
{
    Group &group = preset.getGroup(groupId);
    if (group.isValid()) {
        Rectangle bounds = controlSlotToBounds(newSlot);

        // Keep width within the boundaries
        int rowPosition = (newSlot - 1) % 6;

        if ((rowPosition + newWidth) > 7) {
            newWidth -= rowPosition;
        }

        bounds.setWidth((newWidth * 158) + (newWidth - 1) * 9);

        // Keep height within the boundaries
        int colPosition = (newSlot - 1) / 6;

        if ((colPosition + newHeight) > 6) {
            newHeight -= colPosition;
        }

        if (newHeight > 0) {
            bounds.setHeight(newHeight * 90 - 9);
        } else {
            bounds.setHeight(16);
        }

        group.setBounds(bounds);

        if (Component *component = group.getComponent()) {
            component->setBounds(bounds);
            repaintPage();
        }
    }
}

void MainWindow::setGroupHorizontalSpan(uint16_t groupId, uint8_t newWidth)
{
    Group &group = preset.getGroup(groupId);
    if (group.isValid()) {
        Rectangle bounds = group.getBounds();

        int slot = controlBoundsToSlot(bounds);
        int rowPosition = (slot - 1) % 6;

        if ((rowPosition + newWidth) > 7) {
            newWidth -= rowPosition;
        }

        bounds.setWidth((newWidth * 158) + (newWidth - 1) * 9);
        group.setBounds(bounds);

        if (Component *component = group.getComponent()) {
            component->setBounds(bounds);
            repaintPage();
        }
    }
}

void MainWindow::setGroupVerticalSpan(uint16_t groupId, uint8_t newHeight)
{
    Group &group = preset.getGroup(groupId);
    if (group.isValid()) {
        Rectangle bounds = group.getBounds();

        int slot = controlBoundsToSlot(bounds);
        System::logger.write(LOG_ERROR, "slot: %d", slot);

        // Keep height within the boundaries
        int colPosition = (slot - 1) / 6;

        if ((colPosition + newHeight) > 6) {
            newHeight -= colPosition;
        }

        if (newHeight > 0) {
            bounds.setHeight(newHeight * 90 - 9);
        } else {
            bounds.setHeight(16);
        }

        group.setBounds(bounds);

        if (Component *component = group.getComponent()) {
            component->setBounds(bounds);
            repaintPage();
        }
    }
}

void MainWindow::setGroupVariant(uint16_t groupId, uint8_t newVariant)
{
    Group &group = preset.getGroup(groupId);
    if (group.isValid()) {
        group.setVariant((Group::Variant)newVariant);

        if (Component *c = group.getComponent()) {
            if (GroupControl *gc = dynamic_cast<GroupControl *>(c)) {
                gc->setHighlighted(newVariant);
                repaintPage();
            }
        }
    }
}

void MainWindow::setPageName(uint8_t pageId, const char *newName)
{
    Page &page = preset.getPage(pageId);
    if (page.isValid()) {
        page.setName(newName);

        if ((currentPageId == page.getId()) && pageView) {
            pageView->changePageName(newName);
        }
    }
}

void MainWindow::setInfoText(const char *newText)
{
    model.info.setText(newText);
    if (pageView) {
        pageView->setInfoText(newText);
    }
}

void MainWindow::setRamPercentage(uint8_t newPercentage)
{
    pageView->setRamPercentage(newPercentage);
}

void MainWindow::sendSnapshotList(uint8_t port, const char *projectId)
{
    System::logger.write(
        LOG_ERROR, "sendSnapshotList: projectId=%s", projectId);
    snapshots.sendList(port, projectId);
}

void MainWindow::sendSnapshot(uint8_t port,
                              const char *projectId,
                              uint8_t bankNumber,
                              uint8_t slot)
{
    System::logger.write(LOG_ERROR,
                         "sendSnapshot: projectId=%s, bankNumber=%d, slot=%d",
                         projectId,
                         bankNumber,
                         slot);
    snapshots.sendSnapshot(port, projectId, bankNumber, slot);
}

void MainWindow::sendPresetList(uint8_t port)
{
    System::logger.write(LOG_ERROR, "sendPresetList");
    presets.sendList(port);
}

void MainWindow::enableMidiLearn(void)
{
    System::logger.write(LOG_ERROR, "enableMidiLearn");
    System::context.setMidiLearn(true);
}

void MainWindow::disableMidiLearn(void)
{
    System::logger.write(LOG_ERROR, "disableMidiLearn");
    System::context.setMidiLearn(false);
}

void MainWindow::scheduleSwitchPreset(uint8_t bankNumber, uint8_t slot)
{
    presets.setPendingBankNumberAndSlot(bankNumber, slot);
}

void MainWindow::switchPreset(uint8_t bankNumber, uint8_t slot)
{
    System::logger.write(LOG_ERROR,
                         "switchPreset: bankNumber=%d, slot=%d, id=%d",
                         bankNumber,
                         slot,
                         bankNumber * Preset::MaxNumPots + slot);

    // Free the memory used by the active page
    if (pageView) {
        delete pageView;
        pageView = nullptr;
    }

    if (!presets.loadPresetById(bankNumber * Preset::MaxNumPots + slot)) {
        setInfoText("out of memory!");
    } else {
        setInfoText("");
        if (!snapshots.initialise(preset.getProjectId())) {
            System::logger.write(
                LOG_ERROR,
                "MainWindow::switchPreset: cannot initialize snapshot storage");
        }
    }
    closeAllWindows();
    switchPage(1, preset.getPage(1).getDefaultControlSetId());
}

void MainWindow::switchPresetNext(void)
{
    auto presetId = presets.getPresetId();

    if (presetId < 71) {
        presetId++;
        switchPreset(presetId / 12, presetId % 12);
    }
}

void MainWindow::switchPresetPrev(void)
{
    auto presetId = presets.getPresetId();

    if (presetId > 0) {
        presetId--;
        switchPreset(presetId / 12, presetId % 12);
    }
}

void MainWindow::switchPresetBank(uint8_t bankNumber)
{
    System::logger.write(
        LOG_ERROR, "switchPresetBank: bankNumber=%d", bankNumber);
    presets.setCurrentBankNumber(bankNumber);
}

void MainWindow::setSnapshotSlot(const char *projectId,
                                 uint8_t bankNumber,
                                 uint8_t slot)
{
    System::logger.write(
        LOG_ERROR,
        "setSnapshotSlot: projectId=%s, bankNumber=%d, slot=%d",
        projectId,
        bankNumber,
        slot);
    snapshots.setDestProjectId(projectId);
    snapshots.setDestBankNumber(bankNumber);
    snapshots.setDestSlot(slot);
}

void MainWindow::setPresetSlot(uint8_t bankNumber, uint8_t slot)
{
    System::logger.write(
        LOG_ERROR, "setPresetSlot: bankNumber=%d, slot=%d", bankNumber, slot);
    presets.setCurrentBankNumber(bankNumber);
    presets.setCurrentSlot(slot);
}

void MainWindow::loadSnapshot(const char *projectId,
                              uint8_t bankNumber,
                              uint8_t slot)
{
    System::logger.write(
        LOG_ERROR,
        "loading snapshot: projectId=%s, bankNumber=%d, slot=%d",
        projectId,
        bankNumber,
        slot);
    snapshots.sendSnapshotMessages(projectId, bankNumber, slot);
    sendAllControls();
}

void MainWindow::saveSnapshot(const char *projectId,
                              uint8_t bankNumber,
                              uint8_t slot,
                              const char *newName,
                              uint32_t newColour)
{
    System::logger.write(
        LOG_ERROR,
        "saving snapshot: projectId=%s, bankNumber=%d, slot=%d, name=%s, colour=%d",
        projectId,
        bankNumber,
        slot,
        newName,
        newColour);
    snapshots.saveSnapshot(projectId, bankNumber, slot, newName, newColour);

    if (snapshotsWindow && strcmp(projectId, preset.getProjectId()) == 0) {
        snapshotsWindow->snapshotSaved(bankNumber, slot, newName, newColour);
    }
}

void MainWindow::updateSnapshot(const char *projectId,
                                uint8_t bankNumber,
                                uint8_t slot,
                                const char *name,
                                uint32_t colour)
{
    System::logger.write(
        LOG_ERROR,
        "updateSnapshot: projectId=%s, bankNumber=%d, slot=%d, name=%s, colour=%06x",
        projectId,
        bankNumber,
        slot,
        name,
        colour);
    snapshots.updateSnapshot(projectId, bankNumber, slot, name, colour);
    if (snapshotsWindow && strcmp(projectId, preset.getProjectId()) == 0) {
        snapshotsWindow->snapshotSaved(bankNumber, slot, name, colour);
    }
}

bool MainWindow::updatePresetSlot(uint8_t bankNumber,
                                  uint8_t slot,
                                  const char *presetPath)
{
    System::logger.write(
        LOG_ERROR,
        "updatePresetSlot: bankNumber=%d, slot=%d, presetPath=%s",
        bankNumber,
        slot,
        presetPath);

    uint8_t presetId = Presets::convertToPresetId(bankNumber, slot);
    bool success = presets.updateSlot(presetId, presetPath);
    switchPreset(bankNumber, slot);
    return (success);
}

void MainWindow::removeSnapshot(const char *projectId,
                                uint8_t bankNumber,
                                uint8_t slot)
{
    System::logger.write(LOG_ERROR,
                         "removeSnapshot: projectId=%s, bankNumber=%d, slot=%d",
                         projectId,
                         bankNumber,
                         slot);
    snapshots.removeSnapshot(projectId, bankNumber, slot);
    if (snapshotsWindow && strcmp(projectId, preset.getProjectId()) == 0) {
        snapshotsWindow->snapshotRemoved(bankNumber, slot);
    }
}

void MainWindow::swapSnapshots(const char *projectId,
                               uint8_t sourceBankNumber,
                               uint8_t sourceSlot,
                               uint8_t destBankNumber,
                               uint8_t destSlot)
{
    System::logger.write(
        LOG_ERROR,
        "swapSnapshots: projectId=%s, sourceBankNumber=%d, sourceSlot=%d, destBankNumber=%d, destSlot=%d",
        projectId,
        sourceBankNumber,
        sourceSlot,
        destBankNumber,
        destSlot);
    snapshots.swapSnapshot(
        projectId, sourceBankNumber, sourceSlot, destBankNumber, destSlot);
    if (snapshotsWindow && strcmp(projectId, preset.getProjectId()) == 0) {
        snapshotsWindow->snapshotsSwapped(
            sourceBankNumber, sourceSlot, destBankNumber, destSlot);
    }
}

bool MainWindow::importSnapshot(LocalFile &file)
{
    System::logger.write(
        LOG_ERROR, "importSnapshot: file: %s", file.getFilepath());
    auto snapshot = snapshots.importSnapshot(file);
    if (snapshotsWindow
        && strcmp(snapshots.getDestProjectId(), preset.getProjectId()) == 0) {
        snapshotsWindow->snapshotSaved(snapshots.getDestBankNumber(),
                                       snapshots.getDestSlot(),
                                       snapshot.getName(),
                                       snapshot.getColour());
    }
    return (true); // \todo it should not be always true
}

void MainWindow::setCurrentSnapshotBank(uint8_t bankNumber)
{
    System::logger.write(
        LOG_ERROR, "setCurrentSnapshotBank: bankNumber=%d", bankNumber);
    currentSnapshotBank = bankNumber;
    if (snapshotsWindow) {
        snapshotsWindow->snapshotBankSwitched(currentSnapshotBank);
    }
}

void MainWindow::setControlPort(uint8_t newControlPort)
{
    controlPort = newControlPort;
}

uint8_t MainWindow::getControlPort(void) const
{
    return (controlPort);
}

void MainWindow::setSubscribedEvents(uint8_t newSubscribers)
{
    subscribedEvents = newSubscribers;
}

uint8_t MainWindow::getSubscribedEvents(void) const
{
    return (subscribedEvents);
}

void MainWindow::requestAllPatches(void)
{
    midi.requestAllPatches();
}

void MainWindow::assignComponentToControl(uint16_t controlId,
                                          Component *component)
{
#ifdef DEBUG
    if (component) {
        System::logger.write(
            LOG_ERROR,
            "assignComponentToControl: control=%d, component=%s",
            controlId,
            component->getName());
    }
#endif
    preset.getControl(controlId).setComponent(component);
}

void MainWindow::removeComponentFromControl(uint16_t controlId)
{
    assignComponentToControl(controlId, nullptr);
}

void MainWindow::assignComponentToGroup(uint16_t groupId, Component *component)
{
#ifdef DEBUG
    if (component) {
        System::logger.write(LOG_ERROR,
                             "assignComponentToGroup: group=%d, component=%s",
                             groupId,
                             component->getName());
    }
#endif
    preset.getGroup(groupId).setComponent(component);
}

void MainWindow::removeComponentFromGroup(uint16_t groupId)
{
    assignComponentToGroup(groupId, nullptr);
}

void MainWindow::setDefaultValue(uint16_t controlId, uint8_t handle)
{
    Control &control = preset.getControl(controlId);
    if (control.isValid()) {
        ControlValue &value = control.getValue(handle);
        control.setDefaultValue(value, true);
    }
}

void MainWindow::setActiveControlSetType(
    ActiveControlSetType newActiveControlSetType)
{
    setup.uiFeatures.activeControlSetType = newActiveControlSetType;
}

void MainWindow::setActivePotTouch(uint8_t potId, Component *component)
{
    Window::setActivePotTouch(potId, component);
}

void MainWindow::resetActivePotTouch(uint8_t potId)
{
    Window::resetActivePotTouch(potId);
}

void MainWindow::ping(void)
{
    System::logger.write(LOG_ERROR, "delegate ping");
}

uint8_t MainWindow::getCurrentPageId(void) const
{
    return (currentPageId);
}

uint8_t MainWindow::getCurrentControlSetId(void) const
{
    return (currentControlSetId);
}

uint8_t MainWindow::getCurrentPresetBank(void) const
{
    return (presets.getCurrentBankNumber());
}

uint8_t MainWindow::getCurrentSnapshotBank(void) const
{
    return (currentSnapshotBank);
}

const char *MainWindow::getCurrentProjectId(void) const
{
    return (preset.getProjectId());
}

bool MainWindow::isDetailLocked(void) const
{
    if (detailWindow) {
        return detailWindow->isLocked();
    }
    return false;
}

bool MainWindow::isDetailOnTheLeft(void) const
{
    if (detailWindow) {
        return (detailWindow->getX() < 512);
    }
    return false;
}

void MainWindow::displayPage(void)
{
    if (pageView) {
        delete pageView;
    }

    pageView = new PageView(&preset,
                            *this,
                            uiApi,
                            setup.uiFeatures,
                            currentPageId,
                            currentControlSetId);
    replaceOwnedContent(pageView);
    setRamPercentage(getUsedMemory());
    setInfoText(model.info.getText());
}

void MainWindow::showDetailOfActivePotTouch(void)
{
    if (Component *c = getActivePotComponent()) {
        if (ControlComponent *cc = dynamic_cast<ControlComponent *>(c)) {
            cc->setActive(false);
            openDetail(c->getId());
        }
    }
}

void MainWindow::switchToNextHandleOfActivePotTouch(void)
{
    for (uint8_t i = 0; i < Preset::MaxNumPots; i++) {
        if (potTouchComponents[i]) {
            switchToNextHandle(potTouchComponents[i]);
        }
    }
}

void MainWindow::switchToPreviousHandleOfActivePotTouch(void)
{
    for (uint8_t i = 0; i < Preset::MaxNumPots; i++) {
        if (potTouchComponents[i]) {
            switchToPreviousHandle(potTouchComponents[i]);
        }
    }
}

void MainWindow::showActiveHandle(Component *component, bool shouldBeShown)
{
    if (Envelope *en = dynamic_cast<Envelope *>(component)) {
        en->showActiveSegment(shouldBeShown);
    }
}

void MainWindow::switchToNextHandle(Component *component)
{
    if (Envelope *en = dynamic_cast<Envelope *>(component)) {
        en->switchToNextActiveHandle();
    }
}

void MainWindow::switchToPreviousHandle(Component *component)
{
    if (Envelope *en = dynamic_cast<Envelope *>(component)) {
        en->switchToPreviousActiveHandle();
    }
}

Rectangle MainWindow::getDetailBounds(const Control &control)
{
    if (control.getType() == Control::Type::Fader) {
        return Rectangle(8, 217, 1008, 330);
    } else if (control.getType() == Control::Type::List) {
        uint16_t xPosition = (control.getBounds().getX() > 510) ? 8 : 577;
        return Rectangle(xPosition, 25, 436, 550);
    } else if (control.getType() == Control::Type::Dx7envelope) {
        return Rectangle(158, 62, 828, 490);
    }
    return Rectangle(158, 110, 828, 380);
}

/*
void MainWindow::closeWindow(Window *window)
{
    if (window) {
        delete window;
    }
}
*/

void MainWindow::closeAllWindows(void)
{
    closeDetail();
    closePresetSelection();
    closePageSelection();
    closeUsbHostPorts();
    closeSnapshots();
}

void MainWindow::sendAllControls(void)
{
    for (auto &[id, control] : preset.controls) {
        for (auto &value : control.values) {
            value.message.setValue(
                parameterMap.getValue(value.message.getDeviceId(),
                                      value.message.getType(),
                                      value.message.getParameterNumber()));
            midi.sendMessage(value.message);
        }
    }
}

void MainWindow::sendPotTouchEvent(uint8_t potId,
                                   uint16_t controlId,
                                   bool touched)
{
    if (subscribedEvents & SubscribedEvents::pots) {
        uiApi.sendPotTouchEvent(potId, controlId, touched);
        if (L) {
            events_onPotTouch(potId, controlId, touched);
        }
    }
}

void MainWindow::initialiseEmpty(void)
{
    switchPage(1, 0);
    setInfoText("");
    presets.setCurrentBankNumber(0);
}
