#pragma once

#include "Preset.h"
#include "PresetSlot.h"
#include "Hardware.h"
#include "LocalFile.h"

class Presets
{
public:
    Presets(const char *newAppSandbox,
            const bool &shouldKeepPresetState,
            const bool &shouldLoadPresetStateOnStartup);
    virtual ~Presets() = default;

    void assignPresetNames(void);
    void sendList(uint8_t port);

    bool loadPresetById(uint8_t presetId);
    void runUploadedLuaScript(void);
    bool loadPreset(LocalFile file);
    void removePreset(uint8_t presetId);
    void reset(void);
    void runPresetLuaScript(void);

    void setBankNumberAndSlot(uint8_t presetId);
    void setBankNumberAndSlot(uint8_t newBankNumber, uint8_t newSlot);
    void setPendingBankNumberAndSlot(uint8_t newBankNumber, uint8_t newSlot);
    uint8_t getPresetId(void) const;
    void setCurrentSlot(uint8_t newSlot);
    uint8_t getCurrentSlot(void) const;
    uint8_t getPendingSlot(void) const;
    uint8_t getPendingBankNumber(void) const;
    bool isPresetChangePending(void) const;
    void setCurrentBankNumber(uint8_t newBankNumber);
    uint8_t getCurrentBankNumber(void) const;
    const char *getPresetName(uint8_t slotId) const;
    bool updateSlot(uint8_t presetId, const char *newPresetPath);

    static uint8_t convertToPresetId(uint8_t bankNumber, uint8_t slot);

    Preset preset;

    static constexpr uint8_t NumPresetsInBank = 12;
    static constexpr uint8_t NumBanks = 6;
    static constexpr uint16_t NumSlots = NumBanks * NumPresetsInBank;

private:
    void setDefaultFiles(uint8_t newBankNumber, uint8_t newSlot);

    const char *appSandbox;

    uint8_t currentBankNumber;
    uint8_t currentSlot;
    uint8_t pendingBankNumber;
    uint8_t pendingSlot;
    bool presetChangePending;
    PresetSlot presetSlot[NumSlots];

    bool readyForPresetSwitch;
    const bool &keepPresetState;
    const bool &loadPresetStateOnStartup;
};
