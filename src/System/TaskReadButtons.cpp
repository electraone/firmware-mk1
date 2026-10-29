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

#include "TaskReadButtons.h"
#include "Hardware.h"
#include "ButtonListener.h"

void dispatchButtonEvent(HardwareButton *button, HardwareButton::Event event)
{
    switch (event) {
        case HardwareButton::Event::press:
            ButtonListener::buttonBroadcaster.buttonDown(button->getId());
            break;

        case HardwareButton::Event::hold:
            ButtonListener::buttonBroadcaster.buttonLongHold(button->getId());
            break;

        case HardwareButton::Event::release:
            ButtonListener::buttonBroadcaster.buttonUp(button->getId());
            break;

        default:
            // Ignored on purpose
            break;
    }
}

/*
 * Read buttons
 *
 * Only while the controller sleeps. Otherwise the pot scan reads the buttons,
 * because they share the multiplexer with the knobs and the scan keeps it
 * parked on one knob for a whole tick - see readPots().
 */
void readButtons(void)
{
    HardwareButton *button = Hardware::buttons.getNext();

    dispatchButtonEvent(button, button->process());
}
