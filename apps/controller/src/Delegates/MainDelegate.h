#pragma once

#include "Component.h"
#include "Enums.h"
#include "LocalFile.h"

class MainDelegate
{
public:
    /*
     * Actions
     */

    virtual void switchOff(void) = 0;
    virtual void reboot(void) = 0;

    // Window state management
    virtual bool switchPage(uint8_t pageId) = 0;
    virtual bool switchPage(uint8_t pageId, uint8_t controlSetId) = 0;
    virtual bool switchPageNext(void) = 0;
    virtual bool switchPagePrev(void) = 0;
    virtual bool switchControlSet(uint8_t controlSetId) = 0;
    virtual void openDetail(uint16_t controlId) = 0;
    virtual void lockDetail(void) = 0;
    virtual void closeDetail(void) = 0;
    virtual void openPresetSelection(void) = 0;
    virtual void closePresetSelection(void) = 0;
    virtual void openPageSelection(void) = 0;
    virtual void closePageSelection(void) = 0;
    virtual void openUsbHostPorts(void) = 0;
    virtual void closeUsbHostPorts(void) = 0;
    virtual void openSnapshots(void) = 0;
    virtual void closeSnapshots(void) = 0;
    virtual void repaintPage(void) = 0;
    virtual void displayPage(void) = 0;

    // Controls
    virtual void setControlVisible(uint16_t controlId,
                                   bool shouldBeVisible) = 0;
    virtual void setControlName(uint16_t controlId, const char *newName) = 0;
    virtual void setControlColour(uint16_t controlId, uint32_t newColour) = 0;
    virtual void setControlPot(uint16_t controlId,
                               uint8_t newControlSetId,
                               uint8_t newPotId) = 0;
    virtual void setControlBounds(uint16_t controlId,
                                  const Rectangle &bounds) = 0;
    virtual void setControlSlot(uint16_t controlId, uint8_t newSlot) = 0;
    virtual void setControlSlot(uint16_t controlId,
                                uint8_t newPageId,
                                uint8_t newSlot) = 0;
    virtual void setControlValueLabel(uint16_t controlId,
                                      const char *valueId,
                                      const char *text) = 0;
    virtual void setControlValueLabel(uint16_t controlId,
                                      uint8_t handleId,
                                      const char *text) = 0;

    // Values
    virtual void setControlValueOverlay(uint16_t controlId,
                                        uint8_t handleId,
                                        uint8_t newOverlayId) = 0;
    virtual void setControlValueMin(uint16_t controlId,
                                    uint8_t handleId,
                                    int16_t newMin) = 0;
    virtual void setControlValueMax(uint16_t controlId,
                                    uint8_t handleId,
                                    int16_t newMax) = 0;
    virtual void setControlValueRange(uint16_t controlId,
                                      uint8_t handleId,
                                      int16_t newMin,
                                      int16_t newMax,
                                      int16_t newDefault,
                                      bool updateMesage) = 0;

    // BottomBar
    virtual void setPageName(uint8_t pageId, const char *newName) = 0;
    virtual void setInfoText(const char *newText) = 0;
    virtual void setRamPercentage(uint8_t newPercentage) = 0;

    // Groups
    virtual void setGroupLabel(uint16_t groupId, const char *newLabel) = 0;
    virtual void setGroupColour(uint16_t groupId, uint32_t newColour) = 0;
    virtual void setGroupVisible(uint16_t groupId, bool shouldBeVisible) = 0;
    virtual void setGroupBounds(uint16_t groupId, const Rectangle &bounds) = 0;
    virtual void setGroupSlot(uint16_t groupId,
                              uint8_t newSlot,
                              uint8_t newWidth,
                              uint8_t newHeight) = 0;
    virtual void setGroupHorizontalSpan(uint16_t groupId, uint8_t newWidth) = 0;
    virtual void setGroupVerticalSpan(uint16_t groupId, uint8_t newHeight) = 0;
    virtual void setGroupVariant(uint16_t groupId, uint8_t newVariant) = 0;

    // Api
    virtual void sendSnapshotList(uint8_t port, const char *projectId) = 0;
    virtual void sendSnapshot(uint8_t port,
                              const char *projectId,
                              uint8_t bankNumber,
                              uint8_t slot) = 0;
    virtual void sendPresetList(uint8_t port) = 0;
    virtual void enableMidiLearn(void) = 0;
    virtual void disableMidiLearn(void) = 0;
    virtual void scheduleSwitchPreset(uint8_t bankNumber, uint8_t slot) = 0;
    virtual void switchPreset(uint8_t bankNumber, uint8_t slot) = 0;
    virtual void switchPresetNext(void) = 0;
    virtual void switchPresetPrev(void) = 0;
    virtual void switchPresetBank(uint8_t bankNumber) = 0;
    virtual void setSnapshotSlot(const char *projectId,
                                 uint8_t bankNumber,
                                 uint8_t slot) = 0;
    virtual void setPresetSlot(uint8_t bankNumber, uint8_t slot) = 0;
    virtual bool updatePresetSlot(uint8_t bankNumber,
                                  uint8_t slot,
                                  const char *presetPath) = 0;
    virtual void loadSnapshot(const char *projectId,
                              uint8_t bankNumber,
                              uint8_t slot) = 0;
    virtual void saveSnapshot(const char *projectId,
                              uint8_t bankNumber,
                              uint8_t slot,
                              const char *newName,
                              uint32_t newColour) = 0;
    virtual void updateSnapshot(const char *projectId,
                                uint8_t bankNumber,
                                uint8_t slot,
                                const char *name,
                                uint32_t colour) = 0;
    virtual void removeSnapshot(const char *projectId,
                                uint8_t bankNumber,
                                uint8_t slot) = 0;
    virtual void swapSnapshots(const char *projectId,
                               uint8_t sourceBankNumber,
                               uint8_t sourceSlot,
                               uint8_t destBankNumber,
                               uint8_t destSlot) = 0;
    virtual bool loadPreset(LocalFile &file) = 0;
    virtual bool loadLua(LocalFile &file) = 0;
    virtual bool loadConfig(LocalFile &file) = 0;
    virtual bool importSnapshot(LocalFile &file) = 0;
    virtual void setCurrentSnapshotBank(uint8_t bankNumber) = 0;
    virtual void setControlPort(uint8_t newControlPort) = 0;
    virtual uint8_t getControlPort(void) const = 0;
    virtual void setSubscribedEvents(uint8_t newSubscribers) = 0;
    virtual uint8_t getSubscribedEvents(void) const = 0;
    virtual void
        sendPotTouchEvent(uint8_t potId, uint16_t controlId, bool touched) = 0;

    virtual void sendAllControls(void) = 0;

    // Patch handling
    virtual void requestAllPatches(void) = 0;

    // Component assigments
    virtual void assignComponentToControl(uint16_t controlId,
                                          Component *component) = 0;
    virtual void removeComponentFromControl(uint16_t controlId) = 0;
    virtual void assignComponentToGroup(uint16_t groupId,
                                        Component *component) = 0;
    virtual void removeComponentFromGroup(uint16_t groupId) = 0;

    // Control management actions
    virtual void setActivePotTouch(uint8_t potId, Component *component) = 0;
    virtual void resetActivePotTouch(uint8_t potId) = 0;
    virtual void setDefaultValue(uint16_t controlId, uint8_t handle) = 0;
    virtual void closeAllWindows(void) = 0;

    // Config
    virtual void
        setActiveControlSetType(ActiveControlSetType activeControlSetType) = 0;

    // Testing
    virtual void ping(void) = 0;

    // Getters
    virtual uint8_t getCurrentPageId(void) const = 0;
    virtual uint8_t getCurrentControlSetId(void) const = 0;
    virtual uint8_t getCurrentPresetBank(void) const = 0;
    virtual uint8_t getCurrentSnapshotBank(void) const = 0;
    virtual const char *getCurrentProjectId(void) const = 0;
    virtual bool isDetailLocked(void) const = 0;
    virtual bool isDetailOnTheLeft(void) const = 0;

    virtual void initialiseEmpty(void) = 0;
};
