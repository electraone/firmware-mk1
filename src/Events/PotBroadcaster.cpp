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
 * @file PotBroadcaster.cpp
 *
 * @brief A broadcaster of hardware potentiometer events.
 */

#include "PotBroadcaster.h"
#include "PotListener.h"
#include "Component.h"
#include "System.h"

PotBroadcaster::PotBroadcaster() : iteratorInvalidated(false)
{
}

void PotBroadcaster::addListener(PotListener *newListener)
{
    listeners[newListener] = 1;
}

void PotBroadcaster::removeListener(PotListener *listenerToRemove)
{
    iteratorInvalidated = true;
    listeners.erase(listenerToRemove);
}

void PotBroadcaster::potChange(uint8_t potId, int16_t relativeChange)
{
    PotEvent potEvent(potId, relativeChange);
    triggerCallbacks(potEvent, &PotListener::onPotChange);
}

void PotBroadcaster::potTouchDown(uint8_t potId)
{
    PotEvent potEvent(potId);
    triggerCallbacks(potEvent, &PotListener::onPotTouchDown);
}

void PotBroadcaster::potTouchUp(uint8_t potId)
{
    PotEvent potEvent(potId);
    triggerCallbacks(potEvent, &PotListener::onPotTouchUp);
}

void PotBroadcaster::stopPropagation(void)
{
    iteratorInvalidated = true;
}

void PotBroadcaster::triggerCallbacks(PotEvent &potEvent, plFunction function)
{
    Window *originatingWindow = System::windowManager.getActiveWindow();

    iteratorInvalidated = false;
    auto i = listeners.begin();

    while (i != listeners.end()) {
        auto current = i++;
        auto listener = current->first;

        if (listener->listensToPot(potEvent.getPotId())) {
            Component *eventComponent = static_cast<Component *>(listener);
            potEvent.setEventCompoment(eventComponent);

            /* Every listener is scaled, including one that never declared a
             * range: what arrives here is raw phase counts, which mean nothing
             * to anything downstream. A listener with no range of its own is
             * read as an encoder - one step per detent of travel. */
            potEvent.setAcceleratedChange(
                listener->computeRate(potEvent.getRelativeChange(),
                                      potEvent.getPotId()));

            if ((eventComponent->getWindow() == originatingWindow)
                || eventComponent->isWindow()) {
                for (Component *c = eventComponent; c;
                     c = c->getParentComponent()) {
                    if (PotListener *pl = dynamic_cast<PotListener *>(c)) {
                        (pl->*function)(potEvent);

                        if (function == &PotListener::onPotTouchUp) {
                            (pl->resetStepCount)();
                        }
                    }

                    if (originatingWindow
                        != System::windowManager.getActiveWindow()) {
                        break;
                    }
                    if (iteratorInvalidated) {
                        break;
                    }
                }
            }
        }
        if (iteratorInvalidated) {
            break;
        }
    }
}
