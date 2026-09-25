#include "StatusBar.h"
#include "helpers.h"

StatusBar::StatusBar()
    : midi1Out(false),
      midi1In(false),
      midi2Out(false),
      midi2In(false),
      usbHostIo(false),
      usbDevIo(false),
      requestMidi1Out(false),
      requestMidi1In(false),
      requestMidi2Out(false),
      requestMidi2In(false),
      requestUsbHostIo(false),
      requestUsbDevIo(false),
      repaintMidi1Out(true),
      repaintMidi1In(true),
      repaintMidi2Out(true),
      repaintMidi2In(true),
      repaintUsbHostIo(true),
      repaintUsbDevIo(true),
      needsRepaint(false),
      clockCounter(0)
{
    setName("statusBar");
    setBounds(0, 5, 1024, 15);
    setVisible(true);
}

StatusBar::~StatusBar()
{
}

void StatusBar::paint(Graphics &g)
{
    const char *label =
        (USBDevices[0].product[0] != '\0') ? USBDevices[0].product : "USB HOST";

    if (repaintUsbDevIo == true) {
        statusBarItem(g, 8, 5, "USB DEVICE", usbDevIo);
        repaintUsbDevIo = false;
    }

    if (repaintUsbHostIo == true) {
        statusBarItem(g, 176, 5, label, usbHostIo);
        repaintUsbHostIo = false;
    }

    if (repaintMidi1Out == true) {
        statusBarItem(g, 344, 5, "MIDI 1 OUT", midi1Out);
        repaintMidi1Out = false;
    }

    if (repaintMidi1In == true) {
        statusBarItem(g, 512, 5, "MIDI 1 IN", midi1In);
        repaintMidi1In = false;
    }

    if (repaintMidi2Out == true) {
        statusBarItem(g, 680, 5, "MIDI 2 OUT", midi2Out);
        repaintMidi2Out = false;
    }

    if (repaintMidi2In == true) {
        statusBarItem(g, 848, 5, "MIDI 2 IN", midi2In);
        repaintMidi2In = false;
    }
}

void StatusBar::forceRepaint(void)
{
    repaintMidi1Out = true;
    repaintMidi1In = true;
    repaintMidi2Out = true;
    repaintMidi2In = true;
    repaintUsbHostIo = true;
    repaintUsbDevIo = true;
    needsRepaint = false;

    repaint();
}

void StatusBar::indicate(MidiInterface::Type interface,
                         uint8_t port,
                         Direction direction,
                         MidiMessage::Type msgType)
{
    if ((msgType != MidiMessage::Type::Clock)
        && (msgType != MidiMessage::Type::ActiveSensing)) {
        if (interface == MidiInterface::Type::MidiIo) {
            if (direction == Direction::in) {
                if (port == 0) {
                    requestMidi1In = true;
                } else if (port == 1) {
                    requestMidi2In = true;
                }
            } else if (direction == Direction::out) {
                if (port == 0) {
                    requestMidi1Out = true;
                } else if (port == 1) {
                    requestMidi2Out = true;
                }
            }
        } else if (interface == MidiInterface::Type::MidiUsbDev) {
            requestUsbDevIo = true;
        } else if (interface == MidiInterface::Type::MidiUsbHost) {
            requestUsbHostIo = true;
        }

        needsRepaint = true;
    } else if (msgType == MidiMessage::Type::Clock) {
        if (clockCounter >= 23) {
            clockCounter = 0;
            requestUsbDevIo = true;
            needsRepaint = true;
        } else {
            clockCounter++;
        }
    }
}

void StatusBar::indicateUsbHostChange(void)
{
    requestUsbHostIo = true;
    needsRepaint = true;
}

void StatusBar::statusBarItem(Graphics &g,
                              uint16_t x,
                              uint16_t y,
                              const char *label,
                              bool active)
{
    g.setColour(Colours565::black);
    g.fillRect(x, y, 168, 13);
    g.system().printText(
        x, y, label, TextStyle::smallTransparent, 168, TextAlign::center);

    if (active == false) {
        g.dim(x, y, 168, 13, Colours565::black);
    }
}

bool StatusBar::repaintIsNeeded(void)
{
    bool rc = false;

    if (needsRepaint) {
        needsRepaint = false;

        if ((requestMidi1Out == true) && (midi1Out == false)) {
            midi1Out = true;
            repaintMidi1Out = true;
            requestMidi1Out = false;
            rc = true;
        }

        if ((requestMidi1In == true) && (midi1In == false)) {
            midi1In = true;
            repaintMidi1In = true;
            requestMidi1In = false;
            rc = true;
        }

        if ((requestMidi2Out == true) && (midi2Out == false)) {
            midi2Out = true;
            repaintMidi2Out = true;
            requestMidi2Out = false;
            rc = true;
        }

        if ((requestMidi2In == true) && (midi2In == false)) {
            midi2In = true;
            repaintMidi2In = true;
            requestMidi2In = false;
            rc = true;
        }

        if ((requestUsbDevIo == true) && (usbDevIo == false)) {
            usbDevIo = true;
            repaintUsbDevIo = true;
            requestUsbDevIo = false;
            rc = true;
        }

        if ((requestUsbHostIo == true) && (usbHostIo == false)) {
            usbHostIo = true;
            repaintUsbHostIo = true;
            requestUsbHostIo = false;
            rc = true;
        }

        return (rc);
    }

    if (midi1Out == true) {
        midi1Out = false;
        requestMidi1Out = false;
        repaintMidi1Out = true;
        rc = true;
    }

    if (midi1In == true) {
        midi1In = false;
        requestMidi1In = false;
        repaintMidi1In = true;
        rc = true;
    }

    if (midi2Out == true) {
        midi2Out = false;
        requestMidi2Out = false;
        repaintMidi2Out = true;
        rc = true;
    }

    if (midi2In == true) {
        midi2In = false;
        requestMidi2In = false;
        repaintMidi2In = true;
        rc = true;
    }

    if (usbDevIo == true) {
        usbDevIo = false;
        requestUsbDevIo = false;
        repaintUsbDevIo = true;
        rc = true;
    }

    if (usbHostIo == true) {
        usbHostIo = false;
        requestUsbHostIo = false;
        repaintUsbHostIo = true;
        rc = true;
    }

    return (rc);
}
