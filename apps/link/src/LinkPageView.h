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
 * @file LinkPageView.h
 *
 * @brief The view, drawn with the controller's own control components.
 *
 * The link program could draw shapes of its own from the element types the
 * link carries, and it would look approximately like an Electra One - today,
 * and less so after every change to the real thing. So this builds real
 * controls and lets the real components paint them, which makes the second
 * screen the same picture by construction rather than a resemblance
 * maintained by hand. It is what the mk2's satellite does, for the same
 * reason.
 *
 * That needs somewhere for a Control to live, and a Control lives in a
 * Preset whose values live in the parameter map. Both are here, empty:
 * nothing loads a file, runs Lua or reads the card. They are containers for
 * what the primary sent, and there is still exactly one copy of everything -
 * the primary's.
 *
 * Values are addressed by a virt message per value. A virt transmits
 * nothing, so the map is purely this program's store: the primary sends a
 * number, it lands at that key, and the component reads it back the way it
 * does on a controller.
 */

#pragma once

#include "ControlComponent.h"
#include "BottomBar.h"
#include "Preset.h"
#include "MainDelegate.h"
#include "LinkGestureSink.h"
#include "src/Link/LinkView.h"
#include <map>
#include <vector>

class LinkCustomControl;

class LinkPageView
{
public:
    LinkPageView(MainDelegate &newDelegate, LinkGestureSink &newGestures);

    /**
     * @brief Draw this view, replacing whatever was there.
     *
     * A view whose structure matches what is already built - same controls,
     * same geometry, same overlays, only the values possibly different - is
     * refreshed in place instead of being torn down: viewId says nothing
     * about content, so without the comparison a resend of the same page
     * cost a full rebuild and a whole-screen repaint.
     *
     * @return true when the page was rebuilt, false when it was refreshed
     */
    bool rebuild(const Link::View &view, Component &parent);

    /** @brief One custom control's captured paint arrived, or changed. */
    void applyCustomFrame(Link::ElementId elementId,
                          const uint8_t *bytes,
                          size_t length);

    /**
     * @brief One value moved on the primary.
     *
     * Applied to the one control rather than by rebuilding the page. The
     * value goes into the map and the component is told directly, the way the
     * controller's repaint pass tells it - without the pass, and its 25 ms.
     */
    void updateValue(Link::ElementId elementId,
                     uint8_t handle,
                     int16_t current,
                     const char *label);

    /** @brief Take down every control. */
    void clear(void);

    /** @brief How many controls are on the screen. */
    size_t getCount(void) const;

    /** @brief The bar's notice text, where the controller shows notices. */
    void setInfoText(const char *text);

    /** @brief Draw the bar again, over whatever is on the screen. */
    void repaintBottomBar(void);

    /**
     * @brief Mark a control as the one under a finger, or stop.
     *
     * What the controller does when a knob is touched: the control under it
     * shows a line. The value moves on the primary, but the finger is here.
     */
    void highlightElement(Link::ElementId elementId, bool on);

private:
    /** @brief Build one control and its component, or nothing. */
    void addElement(const Link::ViewElement &element, Component &parent);

    /**
     * @brief The MIDI value that stands for a display value in this store.
     *
     * The component translates a MIDI value back through its message, so the
     * message is shaped for the translation to be the identity: a range from
     * zero, offset by the minimum. A list keeps the item's value, which is
     * what its overlay is indexed by, and a pad keeps one or nought.
     */
    static uint16_t midiFor(const Control &control,
                            const ControlValue &value,
                            int16_t current);

    /** @brief Hash everything about a view except its value state. */
    static uint32_t structureHash(const Link::View &view);

    /** @brief Apply a structurally identical view's values in place. */
    void refreshValues(const Link::View &view);

    /** @brief The overlay of that id, or nullptr when there is none. */
    Overlay *findOverlay(uint8_t overlayId);

    MainDelegate &delegate;
    LinkGestureSink &gestures;

    /*
     * The shell: a Preset that was never loaded. Its pools hold the values and
     * inputs of the controls, exactly as they would hold a parsed preset's.
     */
    Preset preset;

    /**
     * @brief What the controls are children of.
     *
     * Their own container rather than the window's content, so that taking a
     * page down is deleteAllChildren() on this and nothing else.
     */
    Component *frame;

    /** The preset and page names, in the controller's own bar. Kept across
     *  rebuilds so a page change does not make it flicker. */
    BottomBar *bottomBar;

    size_t count;

    /* The captured paints of the custom controls, by element. A map so the
     * vectors keep their addresses: each custom component points at its own
     * entry. */
    std::map<uint16_t, std::vector<uint8_t>> customFrames;
    std::map<uint16_t, LinkCustomControl *> customComponents;

    /** The next free virt parameter number. Never reused within a run. */
    uint16_t nextParameterNumber;

    /** The structure hash of what is built, or zero when nothing is. */
    uint32_t builtStructureHash;
};
