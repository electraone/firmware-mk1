#pragma once

#include "MainDelegate.h"
#include "SysexBlock.h"

class SysexApi
{
public:
    explicit SysexApi(MainDelegate &newDelegate);
    virtual ~SysexApi() = default;

    bool process(uint8_t port, const SysexBlock &sysexBlock);
    bool
        process(uint8_t port, LocalFile &file, ElectraCommand::Object fileType);

private:
    bool loadPreset(uint8_t port, LocalFile &file);
    bool loadLua(uint8_t port, LocalFile &file);
    bool loadConfig(uint8_t port, LocalFile &file);
    bool importSnapshot(uint8_t port, LocalFile &file);

    void sendSnapshotList(uint8_t port, MemoryBlock &sysexPayload);
    void sendSnapshot(uint8_t port, MemoryBlock &sysexPayload);
    void sendPresetList(uint8_t port);
    void enableMidiLearn(uint8_t port);
    void disableMidiLearn(uint8_t port);
    void switchPreset(uint8_t port, uint8_t bankNumber, uint8_t slot);
    void switchPage(uint8_t port, uint8_t pageNumber);
    void switchControlSet(uint8_t port, uint8_t controlSetId);
    void updateControl(uint8_t port,
                       uint16_t controlId,
                       MemoryBlock &sysexPayload);
    void updateControlValueLabel(uint8_t port,
                                 uint16_t controlId,
                                 uint8_t valueId,
                                 MemoryBlock &sysexPayload);
    void setSnapshotSlot(uint8_t port, MemoryBlock &sysexPayload);
    void setPresetSlot(uint8_t port, uint8_t bankNumber, uint8_t slot);
    void updateSnapshot(uint8_t port, MemoryBlock &sysexPayload);
    void updatePresetSlot(uint8_t port, MemoryBlock &sysexPayload);
    void removeSnapshot(uint8_t port, MemoryBlock &sysexPayload);
    void swapSnapshots(uint8_t port, MemoryBlock &sysexPayload);
    void setCurrentSnapshotBank(uint8_t port, uint8_t bankNumber);
    void setControlPort(uint8_t port, uint8_t newControlPort);
    void subscribeEvents(uint8_t port, uint8_t newEvents);
    uint8_t getControlPort(void);

    MainDelegate &delegate;
};
