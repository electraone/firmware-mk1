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
 * @file LinkWindow.h
 *
 * @brief The link program's screen.
 *
 * The unit owns no preset and decides nothing: everything it draws is what a
 * primary sent it. Until one has, this says so - which is also how the unit
 * announces that it is running the link program rather than the controller,
 * since the two are told apart by nothing else on the outside.
 */

#pragma once

#include "Component.h"
#include "Window.h"
#include <cstdint>

class LinkProgram;

/**
 * @brief One knob, listened for on the program's behalf.
 *
 * The control components are released from their pots, so something has to
 * hear the knobs. A listener per knob rather than one for all of them,
 * because what a turn is worth is decided per listener - a range and an
 * accumulator each - and twelve knobs driving twelve controls want twelve
 * answers. A knob's range is asked of the program at every turn, so a new
 * view never has to reassign anything.
 */
class LinkKnob : public Component
{
public:
    LinkKnob(LinkProgram &newProgram, uint8_t newPotId);
    ~LinkKnob() override = default;

    void paint(Graphics &g) override;

    void onPotChange(const PotEvent &potEvent) override;
    void onPotTouchDown(const PotEvent &potEvent) override;
    void onPotTouchUp(const PotEvent &potEvent) override;

    /** @brief Stepped like an encoder, or swept: decided per view. */
    void configure(void);

protected:
    uint16_t numValuesForPot(void) const override;

private:
    LinkProgram &program;
    uint8_t potId;
};

/**
 * @brief What the screen holds.
 *
 * A Component rather than drawing straight onto the Window, because a Window
 * does not paint itself: the repaint manager sends a window's repaint to its
 * owned content and drops it when there is none.
 */
class LinkContent : public Component
{
public:
    explicit LinkContent(LinkProgram &newProgram);
    ~LinkContent() override = default;

    void paint(Graphics &g) override;

    /** @brief What the screen says about the link, in one line. Copied: the
     *  caller's text is often a local and the screen is painted later. */
    void setStatus(const char *newStatus);

    /** @brief A line of raw link state, under the status. */
    void setDetail(const char *newDetail);

    /**
     * @brief Whether a view is being drawn here.
     *
     * While one is, the holding message gets out of the way: the controls
     * are the screen, and a unit showing a page has nothing to announce.
     */
    void setShowingView(bool shouldBeShowing);

    /** @brief Tell every knob what it is driving now. */
    void configureKnobs(void);

private:
    /** @brief Repaint, unless a view is what the screen is showing. */
    void repaintIfShown(void);

    static constexpr uint8_t MaxStatusLength = 60;

    LinkProgram &program;
    char status[MaxStatusLength + 1];
    char detail[MaxStatusLength + 1];
    bool showingView;
    LinkKnob *knobs[NR_OF_HW_POTS];
};

class LinkWindow : public Window
{
public:
    explicit LinkWindow(LinkProgram &newProgram);
    ~LinkWindow() override = default;

    void onButtonDown(uint8_t buttonId) override;
    void onButtonUp(uint8_t buttonId) override;

    void setStatus(const char *newStatus);
    void setDetail(const char *newDetail);

    /** @brief What the controls of a view are added to. */
    LinkContent &getContent(void);

private:
    LinkProgram &program;

    /* On the heap rather than by value: a Window deletes its owned content. */
    LinkContent *content;
};
