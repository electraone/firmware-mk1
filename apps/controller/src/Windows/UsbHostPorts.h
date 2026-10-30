#pragma once

#include "Component.h"
#include "UsbHostDevice.h"
#include "UiApi.h"
#include "USBDevice.h"

extern USBDevice USBDevices[];

class UsbHostPorts : public Component
{
public:
    UsbHostPorts(UiApi &newUiApi);
    virtual ~UsbHostPorts() = default;

    void paint(Graphics &g) override;
    void resized(void) override;

private:
    UiApi &uiApi;

    UsbHostDevice *usbHostDevice[2];
};
