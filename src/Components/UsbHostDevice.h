/*
* Electra One MIDI Controller Firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

/**
 * @file UpdateHostDevice.h
 *
 * @brief A component to provide info about USB Host device.
 */

#pragma once

#include "Component.h"

enum class uiState { ENABLED, SELECTED, DISABLED };

class UsbHostDevice : public Component
{
public:
    UsbHostDevice(const char *newProduct,
                  const char *newManufacturer,
                  const char *newSerial,
                  uint8_t newPortNumber)
        : product(newProduct),
          manufacturer(newManufacturer),
          serial(newSerial),
          portNumber(newPortNumber),
          isDisabled(false),
          dimmed(false),
          active(false)
    {
    }

    virtual ~UsbHostDevice() = default;

    std::function<void(uint8_t)> onClick;

    void onTouchDown(const TouchEvent &touchEvent) override
    {
        uint16_t x = touchEvent.getX();
        uint16_t y = touchEvent.getY();

        if (!isDisabled) {
            if ((y > 0) && (y < getHeight())) {
                if ((x > 330) && (x < 470)) {
                    onClick(0);
                } else if ((x > 470) && (x < 610)) {
                    onClick(1);
                } else if ((x > 610) && (x < 740)) {
                    onClick(2);
                }
            }
        }
    }

    void usbDevice(Graphics &g,
                   uint16_t x,
                   uint16_t y,
                   uiState state,
                   const char *product,
                   const char *manufacturer,
                   const char *serial)
    {
        g.setBteChromaColor(Colours565::red);

        if (strcmp(product, "Electra Controller") == 0) {
            g.copyFromToolkit(821, 79, x, y, 92, 78);
        } else {
            g.copyFromToolkit(922, 79, x + 14, y, 78, 78);
        }

        if (product[0] != '\0') {
            g.system().printText(x + 114,
                                 y,
                                 product,
                                 TextStyle::mediumTransparent,
                                 200,
                                 TextAlign::left);
            g.system().printText(x + 114,
                                 y + 25,
                                 manufacturer,
                                 TextStyle::smallTransparent,
                                 200,
                                 TextAlign::left);
            g.system().printText(x + 114,
                                 y + 45,
                                 serial,
                                 TextStyle::smallTransparent,
                                 200,
                                 TextAlign::left);
            setDisabled(false);
        } else {
            g.system().printText(x + 114,
                                 y,
                                 "NO DEVICE CONNECTED",
                                 TextStyle::mediumTransparent,
                                 200,
                                 TextAlign::left);
            setDisabled(true);
        }

        if (state == uiState::DISABLED) {
            g.dim(x, y, 300, 78, Colours565::black);
        }
    }

    void usbDevicePorts(Graphics &g,
                        uint16_t x,
                        uint16_t y,
                        uiState state,
                        uint8_t port)
    {
        usbDevicePort(g,
                      x,
                      y,
                      "PORT 1",
                      (port == 0) ? uiState::SELECTED : uiState::ENABLED);
        usbDevicePort(g,
                      x + 120,
                      y,
                      "PORT 2",
                      (port == 1) ? uiState::SELECTED : uiState::ENABLED);
        usbDevicePort(g,
                      x + 240,
                      y,
                      "CTRL",
                      (port == 2) ? uiState::SELECTED : uiState::ENABLED);

        if (state == uiState::DISABLED) {
            g.dim(x, y, 300, 78, Colours565::black);
        }
    }

    void usbDevicePort(Graphics &g,
                       uint16_t x,
                       uint16_t y,
                       const char *label,
                       uiState state)
    {
        // \todo Temporary fix to make sure the Chroma colour is not white
        g.setBteChromaColor(Colours565::red);

        if (state == uiState::SELECTED) {
            g.copyFromToolkit(0, 494, x, y, 100, 40);
            g.system().printText(x,
                                 y + 14,
                                 label,
                                 TextStyle::mediumTransparent,
                                 100,
                                 TextAlign::center,
                                 Colours565::black);
        } else {
            g.copyFromToolkit(0, 545, x, y, 100, 40);
            g.system().printText(x,
                                 y + 14,
                                 label,
                                 TextStyle::mediumTransparent,
                                 100,
                                 TextAlign::center,
                                 Colours565::dimgrey);
        }
    }

    void paint(Graphics &g)
    {
        g.fillAll(Colours565::black);

        usbDevice(g, 0, 0, uiState::ENABLED, product, manufacturer, serial);
        usbDevicePorts(g, 340, 0, uiState::ENABLED, portNumber);

        if (isDisabled == true) {
            g.dimAll();
        }
    }

    void setDisabled(bool shouldBeDisabled)
    {
        isDisabled = shouldBeDisabled;
    }

    void setDimmed(bool shouldBeDimmed)
    {
    }

    void setActive(bool shouldBeActive)
    {
        active = shouldBeActive;
    }

    bool isActive(void) const
    {
        return (active);
    }

    bool isDimmed(void) const
    {
        return (dimmed);
    }

    void setPort(uint8_t newPort)
    {
        portNumber = newPort;
        this->repaint();
    }

private:
    const char *product;
    const char *manufacturer;
    const char *serial;
    uint8_t portNumber;

    bool isDisabled;
    bool dimmed;
    bool active;
};
