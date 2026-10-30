#pragma once

#include "PopupWindow.h"
#include "PresetSelection.h"
#include "Model/Presets.h"
#include "UiApi.h"

class PresetSelectionWindow : public PopupWindow
{
private:
    PresetSelectionWindow(const Presets &presets,
                          const PresetBanks &presetBanks,
                          UiApi &newUiApi)
        : uiApi(newUiApi),
          PopupWindow(new PresetSelection(presets,
                                          presetBanks,
                                          newUiApi,
                                          colour,
                                          activeColour),
                      colour,
                      activeColour)
    {
        setName("PRESETS");
        setBounds(0, 243, 1024, 325);
    }

public:
    virtual ~PresetSelectionWindow() = default;

    void onButtonDown(uint8_t buttonId) override
    {
        if (buttonId == 3) {
            uiApi.presetSelection_openUsbHostPorts();
        } else if (buttonId == 4) {
            uiApi.presetSelection_close();
        } else if (buttonId == 5) {
            uiApi.presetSelection_openPageSelection();
        }
    }

    // Factory function
    static PresetSelectionWindow *
        createPresetSelectionWindow(const Presets &presets,
                                    const PresetBanks &presetBanks,
                                    UiApi &newUiApi)
    {
        return new PresetSelectionWindow(presets, presetBanks, newUiApi);
    }

private:
    UiApi &uiApi;

    static constexpr uint32_t colour = 0x2c0000;
    static constexpr uint32_t activeColour = 0x4a0404;
};
