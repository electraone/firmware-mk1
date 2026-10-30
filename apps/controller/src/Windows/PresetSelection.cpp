#include "PresetSelection.h"

PresetSelection::PresetSelection(const Presets &newPresets,
                                 const PresetBanks &newPresetBanks,
                                 UiApi &newUiApi,
                                 uint32_t newColour,
                                 uint32_t newActiveColour)
    : Selection(newPresets.getCurrentSlot(), newColour, newActiveColour),
      presets(newPresets),
      presetBanks(newPresetBanks),
      uiApi(newUiApi),
      activeBank(uiApi.getCurrentPresetBank()),
      bankButton{ nullptr }
{
    setName("PresetSelection");

    for (uint8_t i = 0; i < 6; i++) {
        bankButton[i] = new BankButton();

        if (bankButton[i]) {
            bankButton[i]->setId(i + 100); // offset the id
            bankButton[i]->setLabel(presetBanks[i].name);
            auto colour = presetBanks[i].colour;

            bankButton[i]->setColours(colour, Colours565::darker(colour, 0.3f));

            if (i == activeBank) {
                bankButton[i]->setSelected(true);
            } else {
                bankButton[i]->setSelected(false);
            }

            bankButton[i]->onClick = [this, i]() {
                setActiveBank(i);
            };
            addAndMakeVisible(bankButton[i]);
        }
    }

    for (uint8_t i = 0; i < 12; i++) {
        if (button[i]) {
            uint8_t slotId = (activeBank * Presets::NumPresetsInBank) + i;
            button[i]->setLabel(presets.getPresetName(slotId));
            button[i]->onClick = [this, i]() {
                uiApi.switchPreset(presets.getCurrentBankNumber(), i);
                return (true);
            };
        }
    }
}

void PresetSelection::resized(void)
{
    uint16_t segmentWidth = getWidth() / 6 - 2;
    uint16_t segmentHeight = 70;

    for (uint8_t i = 0; i < 6; i++) {
        uint16_t x = (i % 6) * segmentWidth;
        uint16_t y = ((i < 6) ? 0 : segmentHeight + 20);
        bankButton[i]->setBounds(x + 12, y, segmentWidth - 10, segmentHeight);
    }

    Selection::resized();
}

void PresetSelection::setActiveBank(uint8_t newActiveBank)
{
    bankButton[activeBank]->setSelected(false);
    activeBank = newActiveBank;
    uiApi.switchPresetBank(activeBank);
    for (uint8_t i = 0; i < 12; i++) {
        uint8_t slotId = (activeBank * Presets::NumPresetsInBank) + i;
        button[i]->setLabel(presets.getPresetName(slotId));
    }

    bankButton[activeBank]->setSelected(true);
    repaint();
}
