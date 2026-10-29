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

#include "TaskReadPotTouch.h"
#include "Hardware.h"
#include "PotListener.h"

/*
 * Read capacitive touch
 */
void readPotTouch(void)
{
    if (GET_POT_TOUCH_INT == HIGH) {
        uint16_t sensors = Hardware::potTouch.readSensors();

        for (uint8_t potId = 0; potId < MAX_POT_ID; potId++) {
            Hardware::pots[potId].touched = ((sensors >> (potId)) & 0x01);

            if (Hardware::pots[potId].touched) {
#ifdef DEBUG
                System::logger.write(
                    ERROR, "readPotTouch: pot touch touched: potId=%d", potId);
#endif /* DEBUG */

                if (Hardware::pots[potId].active == false) {
                    Hardware::pots[potId].active = true;
                    PotListener::potBroadcaster.potTouchDown(potId);
                }
            } else {
                if (Hardware::pots[potId].active == true) {
#ifdef DEBUG
                    System::logger.write(
                        ERROR,
                        "readPotTouch: pot touch released: potId=%d",
                        potId);
#endif /* DEBUG */
                    Hardware::pots[potId].active = false;
                    PotListener::potBroadcaster.potTouchUp(potId);
                }
            }
        }
        Hardware::potTouch.clearAllInterrupts();
    }
}
