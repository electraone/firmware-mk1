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

#include "PageView.h"

PageView::PageView(Preset *preset,
                   MainDelegate &newDelegate,
                   UiApi &newUiApi,
                   const UiFeatures &newUiFeatures,
                   uint8_t newPageId,
                   uint8_t activeControlSetId)
    : model(preset),
      delegate(newDelegate),
      uiApi(newUiApi),
      uiFeatures(newUiFeatures),
      pageId(newPageId),
      controlSetId(activeControlSetId),
      bottomBar(nullptr),
      usedPots{ false, false, false, false, false, false,
                false, false, false, false, false, false }
{
    setName("PageView");
    addGroups(model->groups);
    addControls(model->controls);
    addBottomBar(model->getName(), model->getPage(pageId).getName());
    setBounds(0, 0, 1024, 575);
    assignAllPots();
    parameterMap.enable();
}

PageView::~PageView(void)
{
    for (const auto &[id, control] : model->controls) {
        if (control.getPageId() == pageId) {
            delegate.removeComponentFromControl(control.getId());
        }
    }
    for (const auto &[id, group] : model->groups) {
        if (group.getPageId() == pageId) {
            delegate.removeComponentFromGroup(group.getId());
        }
    }
    getWindow()->resetActiveTouch();
    parameterMap.disable();
}

void PageView::setControlSet(uint8_t newControlSetId)
{
    controlSetId = newControlSetId;

    resetUsedPots();

    for (const auto &[id, control] : model->controls) {
        if (control.getPageId() == pageId) {
            auto cc = dynamic_cast<ControlComponent *>(control.getComponent());

            if (cc) {
                configureControl(cc, control);
            }
        }
    }
    for (const auto &[id, group] : model->groups) {
        if (group.getPageId() == pageId) {
            if (auto g = dynamic_cast<GroupControl *>(group.getComponent())) {
                configureGroup(g, group);
            }
        }
    }
    repaint();
}

void PageView::onTouchDown(const TouchEvent &touchEvent)
{
    if (uiFeatures.touchSwitchControlSets) {
        auto y = touchEvent.getScreenY();

        if (y < 240) {
            delegate.switchControlSet(0);
        } else if (y > 400) {
            delegate.switchControlSet(2);
        } else {
            delegate.switchControlSet(1);
        }
    }
}

void PageView::onPotTouchDown(const PotEvent &potEvent)
{
    if (!usedPots[potEvent.getPotId()]) {
        delegate.sendPotTouchEvent(potEvent.getPotId(), 0, true);
    }
}

void PageView::onPotTouchUp(const PotEvent &potEvent)
{
    if (!usedPots[potEvent.getPotId()]) {
        delegate.sendPotTouchEvent(potEvent.getPotId(), 0, false);
    }
}

void PageView::reassignComponent(const Control &control)
{
    Component *c = control.getComponent();

    if (c) {
        auto originalPotId = c->getPotId();
        auto newPotId = control.inputs[0].getPotId();

        c->releasePot();

        if (control.getControlSetId() != controlSetId) {
            if (uiFeatures.activeControlSetType == ActiveControlSetType::dim) {
                c->setDimmed(true);
            }
        } else {
            if (uiFeatures.activeControlSetType == ActiveControlSetType::dim) {
                c->setDimmed(false);
            }
            c->assignPot(newPotId, control.values[0].getNumSteps());
            usedPots[originalPotId] = false;
            usedPots[newPotId] = true;
        }
        repaint();
    }
}

void PageView::changePageName(const char *newName)
{
    bottomBar->setPageName(newName);
    bottomBar->repaint();
}

void PageView::setInfoText(const char *newText)
{
    bottomBar->setInfoText(newText);
    bottomBar->repaint();
}

void PageView::setRamPercentage(uint8_t newPercentage)
{
    bottomBar->setRamPercentage(newPercentage);
    bottomBar->repaint();
}

void PageView::addControl(const Control &control)
{
    ControlComponent *cc =
        ControlComponent::createControlComponent(control, delegate);

    if (cc) {
        configureControl(cc, control);
        addChildComponent(cc);

        delegate.assignComponentToControl(control.getId(), cc);
    }
}

void PageView::moveControl(const Control &control)
{
    if (Component *component = control.getComponent()) {
        component->clear();
        component->setBounds(control.getBounds());
        component->setVisible(true);
        reassignComponent(control);
    }
}

void PageView::removeControl(const Control &control)
{
    if (Component *component = control.getComponent()) {
        component->clear();
        delegate.removeComponentFromControl(control.getId());
        deleteComponent(component);
    }
}

void PageView::paint(Graphics &g)
{
    g.fillAll(Colours565::black);

    if (uiFeatures.activeControlSetType == ActiveControlSetType::bars) {
        g.setColour(0x7BCF);
        g.fillRect(4, 12 + controlSetId * 180, 3, 166);
        g.fillRect(1014, 12 + controlSetId * 180, 3, 166);
        g.setColour(Colours565::black);
        g.fillRect(6, 14 + controlSetId * 180, 2, 162);
        g.fillRect(1014, 14 + controlSetId * 180, 1, 162);
    } else if (uiFeatures.activeControlSetType
               == ActiveControlSetType::background) {
        g.setColour(LookAndFeel::altBackgroundColour);
        g.fillRect(10, 12 + controlSetId * 180, 1003, 168);
        g.setColour(0x0000);
        g.drawPixel(10, 12 + controlSetId * 180);
        g.drawPixel(10, 12 + controlSetId * 180 + 167);
        g.drawPixel(10 + 1002, 12 + controlSetId * 180);
        g.drawPixel(10 + 1002, 12 + controlSetId * 180 + 167);
    }
}

void PageView::resized(void)
{
    bottomBar->setBounds(12, 543, 1000, 22);
}

void PageView::addControls(const Controls &controls)
{
    for (const auto &[id, control] : controls) {
        if (control.getPageId() == pageId) {
            addControl(control);
        }
    }
}

void PageView::addGroups(const Groups &groups)
{
    for (const auto &[id, group] : groups) {
        if (group.getPageId() == pageId) {
            GroupControl *g = new GroupControl(group);

            if (g) {
                configureGroup(g, group);
                addChildComponent(g);
                delegate.assignComponentToGroup(group.getId(), g);
            }
        }
    }
}

void PageView::addBottomBar(const char *presetName, const char *pageName)
{
    bottomBar = new BottomBar(presetName, pageName);

    if (bottomBar) {
        addAndMakeVisible(bottomBar);
    }
}

void PageView::configureGroup(GroupControl *g,
                              [[maybe_unused]] const Group &group)
{
    uint16_t topY = controlSetId * 177 + 0;
    uint16_t bottomY = controlSetId * 177 + 177 + 0;
    uint16_t groupBottomY = g->getY() + g->getHeight();

    if (uiFeatures.activeControlSetType == ActiveControlSetType::dim) {
        if (((topY >= g->getY()) && (topY <= groupBottomY))
            || ((topY <= g->getY()) && (bottomY >= groupBottomY))
            || ((topY <= g->getY()) && (bottomY >= g->getY()))) {
            g->setDimmed(false);
        } else {
            g->setDimmed(true);
        }
    } else if (uiFeatures.activeControlSetType
               == ActiveControlSetType::background) {
        if (topY < g->getY() && (g->getY() < bottomY)) {
            g->setUseAltBackground(true);
        } else {
            g->setUseAltBackground(false);
        }
    }
}

void PageView::configureControl(ControlComponent *cc, const Control &control)
{
    if (control.getControlSetId() == controlSetId) {
        if (uiFeatures.activeControlSetType == ActiveControlSetType::dim) {
            cc->setDimmed(false);
        } else if (uiFeatures.activeControlSetType
                   == ActiveControlSetType::background) {
            cc->setUseAltBackground(true);
        }
    } else {
        if (uiFeatures.activeControlSetType == ActiveControlSetType::dim) {
            cc->setDimmed(true);
        } else if (uiFeatures.activeControlSetType
                   == ActiveControlSetType::background) {
            cc->setUseAltBackground(false);
        }
    }

    if (control.isVisible() && control.getControlSetId() == controlSetId) {
        cc->releasePot();
        cc->assignPot(control.inputs[0].getPotId(),
                      control.values[0].getNumSteps());
        usedPots[control.inputs[0].getPotId()] = true;
    } else {
        cc->releasePot();
    }
}

void PageView::resetUsedPots(void)
{
    for (uint8_t i = 0; i < NR_OF_HW_POTS; i++) {
        usedPots[i] = false;
    }
}
