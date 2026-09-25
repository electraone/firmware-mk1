#include "UiApi.h"
#include "System.h"
#include "MidiOutput.h"

UiApi::UiApi(MainDelegate &newDelegate) : delegate(newDelegate)
{
}

void UiApi::switchOff(void)
{
    delegate.switchOff();
}

void UiApi::reboot(void)
{
    delegate.reboot();
}

void UiApi::switchPreset(uint8_t bankNumber, uint8_t slot)
{
    delegate.scheduleSwitchPreset(bankNumber, slot);
    delegate.closePresetSelection();
    MidiOutput::sendPresetSwitched(MidiInterface::Type::MidiUsbDev,
                                   delegate.getControlPort(),
                                   bankNumber,
                                   slot);
}

void UiApi::switchPresetBank(uint8_t bankNumber)
{
    delegate.switchPresetBank(bankNumber);
    MidiOutput::sendPresetBankSwitched(
        MidiInterface::Type::MidiUsbDev, delegate.getControlPort(), bankNumber);
}

uint8_t UiApi::getCurrentPresetBank(void)
{
    return (delegate.getCurrentPresetBank());
}

void UiApi::switchPage(uint8_t pageId)
{
    delegate.switchPage(pageId);
    MidiOutput::sendPageSwitched(
        MidiInterface::Type::MidiUsbDev, delegate.getControlPort(), pageId - 1);
}

void UiApi::switchControlSet(uint8_t controlSetId)
{
    delegate.switchControlSet(controlSetId);
    MidiOutput::sendControlSetSwitched(MidiInterface::Type::MidiUsbDev,
                                       delegate.getControlPort(),
                                       controlSetId);
}

void UiApi::switchSnapshotBank(uint8_t bankNumber)
{
    delegate.setCurrentSnapshotBank(bankNumber);
    MidiOutput::sendSnapshotBankChanged(
        MidiInterface::Type::MidiUsbDev, delegate.getControlPort(), bankNumber);
}

void UiApi::sendAllSnapshotValues(void)
{
    delegate.sendAllControls();
}

void UiApi::loadSnapshot(const char *projectId,
                         uint8_t bankNumber,
                         uint8_t slot)
{
    delegate.loadSnapshot(projectId, bankNumber, slot);
    delegate.closeSnapshots();
}

void UiApi::loadAndStaySnapshot(const char *projectId,
                                uint8_t bankNumber,
                                uint8_t slot)
{
    delegate.loadSnapshot(projectId, bankNumber, slot);
}

void UiApi::saveSnapshot(const char *projectId,
                         uint8_t bankNumber,
                         uint8_t slot,
                         const char *name,
                         uint32_t colour)
{
    delegate.saveSnapshot(projectId, bankNumber, slot, name, colour);
    MidiOutput::sendSnapshotChanged(MidiInterface::Type::MidiUsbDev,
                                    delegate.getControlPort());
}

void UiApi::removeSnapshot(const char *projectId,
                           uint8_t bankNumber,
                           uint8_t slot)
{
    delegate.removeSnapshot(projectId, bankNumber, slot);
    MidiOutput::sendSnapshotChanged(MidiInterface::Type::MidiUsbDev,
                                    delegate.getControlPort());
}

void UiApi::sendPotTouchEvent(uint8_t potId, uint16_t controlId, bool touched)
{
    MidiOutput::sendPotTouchEvent(MidiInterface::Type::MidiUsbDev,
                                  delegate.getControlPort(),
                                  potId,
                                  controlId,
                                  touched);
}

void UiApi::presetSelection_openUsbHostPorts(void)
{
    delegate.closePresetSelection();
    delegate.openUsbHostPorts();
}

void UiApi::presetSelection_close(void)
{
    delegate.closePresetSelection();
}

void UiApi::presetSelection_openPageSelection(void)
{
    delegate.closePresetSelection();
    delegate.openPageSelection();
}

void UiApi::pageSelection_openUsbHostPorts(void)
{
    delegate.closePageSelection();
    delegate.openUsbHostPorts();
}

void UiApi::pageSelection_openPresetSelection(void)
{
    delegate.closePageSelection();
    delegate.openPresetSelection();
}

void UiApi::pageSelection_close(void)
{
    delegate.closePageSelection();
}

void UiApi::snapshots_close(void)
{
    delegate.closeSnapshots();
}

void UiApi::usbHost_close(void)
{
    delegate.closeUsbHostPorts();
}

void UiApi::usbHost_openPresetSelection(void)
{
    delegate.closeUsbHostPorts();
    delegate.openPresetSelection();
}

void UiApi::usbHost_openPageSelection(void)
{
    delegate.closeUsbHostPorts();
    delegate.openPageSelection();
}
