#include "UsbHostPorts.h"
#include "System.h"

UsbHostPorts::UsbHostPorts(UiApi &newUiApi)
    : uiApi(newUiApi), usbHostDevice{ nullptr }
{
    setName("settings");

    for (uint8_t i = 0; i < 2; i++) {
        usbHostDevice[i] = new UsbHostDevice(USBDevices[i].product,
                                             USBDevices[i].manufacturer,
                                             USBDevices[i].serial,
                                             USBDevices[i].midiBus);

        if (usbHostDevice[i]) {
            usbHostDevice[i]->setId(100 + i);

            if (USBDevices[i].midiDevice == NULL) {
                usbHostDevice[i]->setDisabled(true);
            }

            usbHostDevice[i]->onClick = [this, i](uint8_t port) {
                System::logger.write(
                    LOG_ERROR,
                    "UsbHostPorts: setting USB device %d to port %d",
                    i,
                    port);
                usbHostDevice[i]->setPort(port);
                USBDevices[i].midiBus = port;
            };

            addAndMakeVisible(usbHostDevice[i]);
        }
    }
}

void UsbHostPorts::paint(Graphics &g)
{
    g.fillAll(Colours565::black);
}

void UsbHostPorts::resized(void)
{
    for (uint8_t i = 0; i < 2; i++) {
        usbHostDevice[i]->setBounds(166, 25 + (i * 150), 700, 100);
    }
}
