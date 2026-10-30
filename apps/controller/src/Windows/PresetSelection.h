#pragma once

#include "Selection.h"
#include "Model/Presets.h"
#include "Config/Config.h"
#include "UiApi.h"
#include "BankButton.h"

class PresetSelection : public Selection
{
public:
    PresetSelection(const Presets &newPresets,
                    const PresetBanks &newPresetBanks,
                    UiApi &newUiApi,
                    uint32_t newColour,
                    uint32_t newActiveColour);
    virtual ~PresetSelection() = default;

    void resized(void) override;

private:
    void setActiveBank(uint8_t newActiveBank);

    const Presets &presets;
    const PresetBanks &presetBanks;
    UiApi &uiApi;
    uint8_t activeBank;

    BankButton *bankButton[6];
};
