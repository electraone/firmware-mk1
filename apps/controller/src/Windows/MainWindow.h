#pragma once

#include "ParameterMapWindow.h"
#include "ButtonListener.h"
#include "PresetSelectionWindow.h"
#include "PageSelectionWindow.h"
#include "DetailWindow.h"
#include "SnapshotsWindow.h"
#include "UsbHostWindow.h"
#include "PageView.h"
#include "MainDelegate.h"
#include "Model.h"
#include "Midi/Midi.h"
#include "Config/Config.h"
#include "System.h"

class MainWindow : public ParameterMapWindow, public MainDelegate
{
public:
    MainWindow(Model &newModel, Midi &newMidi, Config &newConfig);
    virtual ~MainWindow() = default;

    void resized(void) override;

    void onButtonDown(uint8_t buttonId) override;
    void onButtonUp(uint8_t buttonId) override;

    void switchOff(void) override;
    void reboot(void) override;

    // User actions
    bool switchPage(uint8_t pageId) override;
    bool switchPage(uint8_t pageId, uint8_t controlSetId) override;
    bool switchPageNext(void) override;
    bool switchPagePrev(void) override;
    bool switchControlSet(uint8_t controlSetId) override;
    void openDetail(uint16_t controlId) override;
    void lockDetail(void) override;
    void closeDetail(void) override;
    void openPresetSelection(void) override;
    void closePresetSelection(void) override;
    void openPageSelection(void) override;
    void closePageSelection(void) override;
    void openUsbHostPorts(void) override;
    void closeUsbHostPorts(void) override;
    void openSnapshots(void) override;
    void closeSnapshots(void) override;
    void repaintPage(void) override;
    void displayPage(void) override;

    // Data loading
    bool loadPreset(LocalFile &file) override;
    bool loadLua(LocalFile &file) override;
    bool loadConfig(LocalFile &file) override;
    bool importSnapshot(LocalFile &file) override;

    // API
    /**
     * set visibility of the control
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param shouldBeVisible visible from set to true
     */
    void setControlVisible(uint16_t controlId, bool shouldBeVisible) override;

    /**
     * set control name
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param newName a new name to be assigned to the control
     */
    void setControlName(uint16_t controlId, const char *newName) override;

    /**
     * set colour of the control
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param newColour colour in RGB format
     */
    void setControlColour(uint16_t controlId, uint32_t newColour) override;

    /**
     * assign a pot to the control's default value
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param newControlSetId an identifier of the control set
     * @param newPotId an identifier of the pot
     */
    void setControlPot(uint16_t controlId,
                       uint8_t newControlSetId,
                       uint8_t newPotId) override;

    /**
     * set boundary box (position and size) of the control
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param bounds boundary box to be used
     */
    void setControlBounds(uint16_t controlId, const Rectangle &bounds) override;

    /**
     * position the control in given page slot
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param newSlot an identifier of the slot on the page
     */
    void setControlSlot(uint16_t controlId, uint8_t newSlot) override;

    virtual void setControlSlot(uint16_t controlId,
                                uint8_t newPageId,
                                uint8_t newSlot) override;
    /**
     * replace control value with fixed text string
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param valueId text identifier of the value to be overridden
     * @param text text to be displayed
     */
    void setControlValueLabel(uint16_t controlId,
                              const char *valueId,
                              const char *text) override;

    /**
     * replace control value with fixed text string
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param handleId a numeric identifier of the value to be overridden
     * @param text text to be displayed
     */
    void setControlValueLabel(uint16_t controlId,
                              uint8_t handleId,
                              const char *text) override;

    /**
     * assign the overlay (list data) to a control value
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param handleId a numeric identifier of the value
     * @param newOverlayId an identifier of the overlay list
     */
    void setControlValueOverlay(uint16_t controlId,
                                uint8_t handleId,
                                uint8_t newOverlayId) override;

    /**
     * set the display minimum of a control value
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param handleId a numeric identifier of the value
     * @param newMin a new minimum display value
     */
    void setControlValueMin(uint16_t controlId,
                            uint8_t handleId,
                            int16_t newMin) override;

    /**
     * set the display maximum of a control value
     *
     * @param controlId an identifier of the control (#ref in the editor)
     * @param handleId a numeric identifier of the value
     * @param newMax a new minimum display value
     */
    void setControlValueMax(uint16_t controlId,
                            uint8_t handleId,
                            int16_t newMax) override;

    /**
     * @brief Reconfigures active range of a control value
     * 
     * @param controlId an identifier of the control (#ref in the editor)
     * @param handleId a numeric identifier of the value
     * @param newMin a new minimum display value
     * @param newMax a new minimum display value
     * @param newDefault a new default value
     * @param updateMesage if true, the message MIDI gets updated too
     */
    void setControlValueRange(uint16_t controlId,
                              uint8_t handleId,
                              int16_t newMin,
                              int16_t newMax,
                              int16_t newDefault,
                              bool updateMesage) override;

    /**
     * set a group label
     *
     * @param groupId an identifier of the group (#ref in the editor)
     * @param newLabel a new label to be used
     */
    void setGroupLabel(uint16_t groupId, const char *newLabel) override;

    /**
     * set colour of the group
     *
     * @param groupId an identifier of the group (#ref in the editor)
     * @param newColour colour in RGB format
     */
    void setGroupColour(uint16_t groupId, uint32_t newColour) override;

    /**
     * set visibility of the group
     *
     * @param groupId an identifier of the group (#ref in the editor)
     * @param shouldBeVisible visible from set to true
     */
    void setGroupVisible(uint16_t groupId, bool shouldBeVisible) override;

    /**
      * set boundary box (position and size) of the group
      *
      * @param groupId an identifier of the group (#ref in the editor)
      * @param bounds boundary box to be used
      */
    void setGroupBounds(uint16_t groupId, const Rectangle &bounds) override;

    /**
     * position the group in given page slot
     *
     * @param groupId an identifier of the group (#ref in the editor)
     * @param newSlot an identifier of the slot on the page
     */
    void setGroupSlot(uint16_t groupId,
                      uint8_t newSlot,
                      uint8_t newWidth,
                      uint8_t newHeight) override;

    /**
     * set the horizontal width (in slot units)
     *
     * @param groupId an identifier of the group (#ref in the editor)
     * @param newWidth a new width to be used
     */
    void setGroupHorizontalSpan(uint16_t groupId, uint8_t newWidth) override;

    /**
     * set the vertical height (in slot units)
     *
     * @param groupId an identifier of the group (#ref in the editor)
     * @param newHeight a new height to be used
     */
    void setGroupVerticalSpan(uint16_t groupId, uint8_t newHeight) override;

    /**
     * set a group variant
     *
     * @param groupId an identifier of the group (#ref in the editor)
     * @param newVariant 0 for thin, 1 for highlighted
     */
    void setGroupVariant(uint16_t groupId, uint8_t newVariant) override;

    void setPageName(uint8_t pageId, const char *newName) override;
    void setInfoText(const char *newText) override;
    void setRamPercentage(uint8_t newPercentage) override;

    void sendSnapshotList(uint8_t port, const char *projectId) override;
    void sendSnapshot(uint8_t port,
                      const char *projectId,
                      uint8_t bankNumber,
                      uint8_t slot) override;
    void sendPresetList(uint8_t port) override;
    void enableMidiLearn(void) override;
    void disableMidiLearn(void) override;
    void scheduleSwitchPreset(uint8_t bankNumber, uint8_t slot) override;
    void switchPreset(uint8_t bankNumber, uint8_t slot) override;
    void switchPresetNext(void) override;
    void switchPresetPrev(void) override;
    void switchPresetBank(uint8_t bankNumber) override;
    void setSnapshotSlot(const char *projectId,
                         uint8_t bankNumber,
                         uint8_t slot) override;
    void setPresetSlot(uint8_t bankNumber, uint8_t slot) override;
    bool updatePresetSlot(uint8_t bankNumber,
                          uint8_t slot,
                          const char *presetPath) override;
    void loadSnapshot(const char *projectId,
                      uint8_t bankNumber,
                      uint8_t slot) override;
    void saveSnapshot(const char *projectId,
                      uint8_t bankNumber,
                      uint8_t slot,
                      const char *newName,
                      uint32_t newColour) override;
    void updateSnapshot(const char *projectId,
                        uint8_t bankNumber,
                        uint8_t slot,
                        const char *name,
                        uint32_t colour) override;
    void removeSnapshot(const char *projectId,
                        uint8_t bankNumber,
                        uint8_t slot) override;
    void swapSnapshots(const char *projectId,
                       uint8_t sourceBankNumber,
                       uint8_t sourceSlot,
                       uint8_t destBankNumber,
                       uint8_t destSlot) override;
    void setCurrentSnapshotBank(uint8_t bankNumber) override;
    void setControlPort(uint8_t newControlPort) override;
    uint8_t getControlPort(void) const override;
    void setSubscribedEvents(uint8_t newSubscribers) override;
    uint8_t getSubscribedEvents(void) const override;
    void sendAllControls(void) override;
    void sendPotTouchEvent(uint8_t potId,
                           uint16_t controlId,
                           bool touched) override;

    // Misc
    void requestAllPatches(void) override;

    void assignComponentToControl(uint16_t controlId,
                                  Component *component) override;
    void removeComponentFromControl(uint16_t controlId) override;
    void assignComponentToGroup(uint16_t groupId,
                                Component *component) override;
    void removeComponentFromGroup(uint16_t groupId) override;

    void setDefaultValue(uint16_t controlId, uint8_t handle) override;

    void setActiveControlSetType(
        ActiveControlSetType newActiveControlSetType) override;
    void closeAllWindows(void) override;

    void setActivePotTouch(uint8_t potId, Component *component) override;
    void resetActivePotTouch(uint8_t potId) override;

    void ping(void) override;

    // Getters
    uint8_t getCurrentPageId(void) const override;
    uint8_t getCurrentControlSetId(void) const override;
    uint8_t getCurrentPresetBank(void) const override;
    uint8_t getCurrentSnapshotBank(void) const override;
    const char *getCurrentProjectId(void) const override;
    bool isDetailLocked(void) const override;
    bool isDetailOnTheLeft(void) const override;

    void initialiseEmpty(void) override;

private:
    void showDetailOfActivePotTouch(void);
    void showActiveHandle(Component *component, bool shouldBeShown);
    void switchToNextHandle(Component *component);
    void switchToPreviousHandle(Component *component);
    void switchToNextHandleOfActivePotTouch(void);
    void switchToPreviousHandleOfActivePotTouch(void);
    Rectangle getDetailBounds(const Control &control);
    void refreshControl(const Control &control);

    // MainWindow data
    Model &model;
    Preset &preset;
    Snapshots &snapshots;
    Presets &presets;

    // MIDI Interface
    Midi &midi;

    // Config
    Config &setup;

    // UI Activity API handler
    UiApi uiApi;

    // Window content
    PageView *pageView;

    // Sub-windows
    PageSelectionWindow *pageSelectionWindow;
    PresetSelectionWindow *presetSelectionWindow;
    DetailWindow *detailWindow;
    SnapshotsWindow *snapshotsWindow;
    UsbHostWindow *usbHostWindow;

    // Current state
    uint8_t currentPageId;
    uint8_t currentControlSetId;
    uint8_t currentSnapshotBank;

    // Controller status
    bool inSleepMode;
    uint8_t controlPort;
    uint8_t subscribedEvents;
};
