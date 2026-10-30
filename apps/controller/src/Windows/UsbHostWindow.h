#pragma once

#include "PopupWindow.h"
#include "UsbHostPorts.h"
#include "UiApi.h"

class UsbHostWindow : public PopupWindow
{
private:
    UsbHostWindow(UiApi &newUiApi)
        : uiApi(newUiApi),
          PopupWindow(new UsbHostPorts(newUiApi), colour, activeColour)
    {
        setName("SETTINGS");
        setBounds(0, 216, 1024, 352);
    }

public:
    virtual ~UsbHostWindow() = default;

    void onButtonDown(uint8_t buttonId) override
    {
        if (buttonId == 3) {
            uiApi.usbHost_close();
        } else if (buttonId == 4) {
            uiApi.usbHost_openPresetSelection();
        } else if (buttonId == 5) {
            uiApi.usbHost_openPageSelection();
        }
    }

    // Factory function
    static UsbHostWindow *createUsbHostWindow(UiApi &newUiApi)
    {
        return new UsbHostWindow(newUiApi);
    }

private:
    UiApi &uiApi;

    static constexpr uint32_t colour = 0x2820;
    static constexpr uint32_t activeColour = 0x4860;
};
