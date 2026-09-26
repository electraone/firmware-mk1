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

#include "RepaintManager.h"
#include "Component.h"
#include "Window.h"

RepaintManager::RepaintManager(Graphics &newG, WindowManager &newWm)
    : g(newG), wm(newWm)
{
}

void RepaintManager::scheduleRepaint(Component *component)
{
    invalidateEntries(component);
    repaintQueue.push_back(RepaintRequest(component));
}

void RepaintManager::scheduleClear(const Rectangle &area)
{
    repaintQueue.push_back(RepaintRequest(area));
}

#include "System.h"

void RepaintManager::processRepaintQueue(void)
{
    if (!repaintQueue.empty()) {
        g.switchWriteLayer();
        unfoldNestedComponents();

        uint8_t zLevel = 0;
        for (auto window : wm.getWindows()) {
            if (zLevel == 1) {
                scheduleRepaint(window->getOwnedContent());
                unfoldComponent(window->getOwnedContent());
            }
            repaintQueueEntries(window);
            zLevel++;
        }
        repaintQueue.clear();
        g.showPreparedLayer(0, 0, 1024, 600);
    }
}

void RepaintManager::clearRepaintQueue(void)
{
    repaintQueue.clear();
}

void RepaintManager::invalidateEntries(Component *component)
{
    for (RepaintRequest &request : repaintQueue) {
        if (request.component == component) {
            request.valid = false;
        }
    }
}

/*
 * Indexed rather than iterated, and only as far as the queue reached when this
 * started.
 *
 * Unfolding a component calls repaint() on it and on every child, and that
 * appends to this same queue. An iterator - which is what a range for gives -
 * points into the vector's storage, and a push_back that grows the vector moves
 * that storage somewhere else: every following step then reads freed memory,
 * and the dynamic_cast below takes whatever it finds there for a vtable. It
 * faults on a knob if you are lucky and paints nonsense if you are not.
 *
 * Reading the entry through the index each time survives the move. The bound is
 * taken once for the same reason the loop always had it: the entries unfolding
 * appends are the expansion, and expanding those again would never end.
 */
void RepaintManager::unfoldNestedComponents()
{
    const size_t queued = repaintQueue.size();

    for (size_t i = 0; i < queued; i++) {
        if (!repaintQueue[i].valid || !repaintQueue[i].component) {
            continue;
        }

        Component *component = repaintQueue[i].component;

        if (Window *window = dynamic_cast<Window *>(component)) {
            unfoldComponent(window);
            component = window->getOwnedContent();
        }

        unfoldComponent(component);
    }
}

void RepaintManager::unfoldComponent(Component *component)
{
    if (component->isVisible()) {
        component->repaint();

        for (auto &nestedComponent : component->getChildren()) {
            unfoldComponent(nestedComponent);
        }
    }
}

/* Indexed for the same reason as unfoldNestedComponents(): painting a component
 * can ask for another repaint, and that appends here. */
void RepaintManager::repaintQueueEntries(Window *window)
{
    const size_t queued = repaintQueue.size();

    for (size_t i = 0; i < queued; i++) {
        if (repaintQueue[i].valid == false) {
            continue;
        }

        if (repaintQueue[i].component) {
            Component *component = repaintQueue[i].component;
            Window *componentWindow = component->getWindow();

            if ((componentWindow == nullptr) || (componentWindow == window)) {
                repaintQueue[i].valid = false;
                paintComponent(component);
            }
        } else {
            const Rectangle area = repaintQueue[i].areaToClear;

            repaintQueue[i].valid = false;
            clearArea(area);
        }
    }
}

void RepaintManager::paintComponent(Component *component)
{
    g.setActiveWindowPosition(component->getScreenX(), component->getScreenY());
    g.setActiveWindowSize(component->getWidth(), component->getHeight());
    component->paint(g);

    if (component->isDimmed()) {
        g.dim(0,
              0,
              component->getWidth(),
              component->getHeight(),
              Colours565::black);
    }
}

void RepaintManager::clearArea(const Rectangle &area)
{
    g.setColour(0x0000);
    g.fillRect(area);
}