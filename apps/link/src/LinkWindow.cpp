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

#include "LinkWindow.h"
#include "LinkProgram.h"
#include "LookAndFeel.h"
#include "System.h"
#include "helpers.h"

/*
 * LinkKnob
 */

LinkKnob::LinkKnob(LinkProgram &newProgram, uint8_t newPotId)
    : program(newProgram), potId(newPotId)
{
    setName("linkKnob");
    setBounds(0, 0, 0, 0);
    setVisible(false);
    assignPot(potId);
}

void LinkKnob::paint([[maybe_unused]] Graphics &g)
{
}

void LinkKnob::configure(void)
{
    enableEncoderMode(program.isDiscreteOnPot(potId));
    resetStepCount();
}

uint16_t LinkKnob::numValuesForPot(
    [[maybe_unused]] uint8_t turnedPotId) const
{
    return (program.valuesOnPot(potId));
}

void LinkKnob::onPotChange(const PotEvent &potEvent)
{
    program.turn(potId, potEvent.getAcceleratedChange());
}

void LinkKnob::onPotTouchDown([[maybe_unused]] const PotEvent &potEvent)
{
    program.potTouch(potId, true);
}

void LinkKnob::onPotTouchUp([[maybe_unused]] const PotEvent &potEvent)
{
    program.potTouch(potId, false);
}

/*
 * LinkContent
 */

LinkContent::LinkContent(LinkProgram &newProgram)
    : program(newProgram), status{}, detail{}, showingView(false), knobs{}
{
    setName("linkContent");
    copyString(status, "waiting for a primary", MaxStatusLength);

    for (uint8_t i = 0; i < NR_OF_HW_POTS; i++) {
        knobs[i] = new LinkKnob(program, i);
        addChildComponent(knobs[i]);
    }
}

void LinkContent::setStatus(const char *newStatus)
{
    copyString(status, newStatus, MaxStatusLength);
    repaintIfShown();
}

void LinkContent::setDetail(const char *newDetail)
{
    copyString(detail, newDetail, MaxStatusLength);
    repaintIfShown();
}

/*
 * Repaint only when these words are the thing on the screen. While a view is
 * up they are not drawn at all, and repainting anyway would clear the whole
 * screen under the page for every message counted.
 */
void LinkContent::repaintIfShown(void)
{
    if (showingView) {
        return;
    }

    repaint();
}

void LinkContent::setShowingView(bool shouldBeShowing)
{
    if (showingView == shouldBeShowing) {
        return;
    }

    showingView = shouldBeShowing;
    repaint();
}

void LinkContent::configureKnobs(void)
{
    for (uint8_t i = 0; i < NR_OF_HW_POTS; i++) {
        if (knobs[i] != nullptr) {
            knobs[i]->configure();
        }
    }
}

void LinkContent::paint(Graphics &g)
{
    g.fillAll(LookAndFeel::backgroundColour);

    /* A page of controls is the whole screen. Nothing is written over it -
     * what the program has to say, it says while it has nothing to show. */
    if (showingView) {
        return;
    }

    const uint16_t width = getWidth();
    const uint16_t middle = getHeight() / 2;

    g.system().printText(0,
                         middle - 30,
                         "ELECTRA LINK",
                         TextStyle::mediumTransparent,
                         width,
                         TextAlign::center);

    g.system().printText(0,
                         middle,
                         status,
                         TextStyle::smallTransparent,
                         width,
                         TextAlign::center);

    g.system().printText(0,
                         middle + 24,
                         detail,
                         TextStyle::smallTransparent,
                         width,
                         TextAlign::center);

    /* The unit's own serial, because two of these on a bench are told apart
     * by nothing else. */
    g.system().printText(0,
                         middle + 48,
                         System::runtimeInfo.getElectraInfoSerial(),
                         TextStyle::smallTransparent,
                         width,
                         TextAlign::center);
}

/*
 * LinkWindow
 */

LinkWindow::LinkWindow(LinkProgram &newProgram)
    : program(newProgram), content(new LinkContent(newProgram))
{
    setName("link");

    /* The content fills the window. A window's bounds are already inset for
     * the status bar, so this is 0,0 in the window's own space. */
    content->setBounds(0, 0, getWidth(), getHeight());
    content->setVisible(true);
    setOwnedContent(content);
}

void LinkWindow::onButtonDown(uint8_t buttonId)
{
    program.buttonDown(buttonId);
}

void LinkWindow::onButtonUp(uint8_t buttonId)
{
    program.buttonUp(buttonId);
}

void LinkWindow::setStatus(const char *newStatus)
{
    content->setStatus(newStatus);
}

void LinkWindow::setDetail(const char *newDetail)
{
    content->setDetail(newDetail);
}

LinkContent &LinkWindow::getContent(void)
{
    return (*content);
}
