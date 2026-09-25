#pragma once

#include <stdint.h>

struct Router {
    Router()
        : usbDevToMidiIo(true),
          usbDevToUsbHost(true),
          usbDevToMidiControl(true),
          midiIoToUsbDev(true),
          midiIoToUsbHost(true),
          usbHostToMidiIo(true),
          usbHostToUsbDev(true),
          midiIo1Thru(false),
          midiIo2Thru(false),
          midiControlPort(2),
          midiControlChannel(0),
          midiControlDrop(true)
    {
    }

    bool usbDevToMidiIo;
    bool usbDevToUsbHost;
    bool usbDevToMidiControl;
    bool midiIoToUsbDev;
    bool midiIoToUsbHost;
    bool usbHostToMidiIo;
    bool usbHostToUsbDev;
    bool midiIo1Thru;
    bool midiIo2Thru;
    uint8_t midiControlPort;
    uint8_t midiControlChannel;
    bool midiControlDrop;
};
