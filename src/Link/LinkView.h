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
 * @file LinkView.h
 *
 * @brief Everything the satellite knows: what to draw, and what it holds.
 *
 * This is the whole of the satellite's state. There is no preset here, no
 * parameter map, no overlays and no formatters - a value that should be shown
 * as words arrives as words, because the primary is the only end that can turn
 * one into the other.
 *
 * It lives in src/ beside the records, and for the same reason they do: both
 * ends of the link hold a view - the primary that describes it and whatever
 * runs the satellite program - and the only way to be sure they agree about it
 * is to compile the same file. Nothing here knows how anything is drawn.
 *
 * An element's id is opaque. It is not a control id and nothing may be read
 * into it: it exists so that an input event or an update can name an element,
 * and only the primary knows what it refers to.
 */

#pragma once

#include "src/Link/LinkRecords.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace Link
{
    struct Rect {
        int16_t x = 0;
        int16_t y = 0;
        int16_t width = 0;
        int16_t height = 0;
    };

    struct ViewValue {
        uint8_t handle = 0;

        /** Which overlay this value reads its words from, or zero. */
        uint8_t overlayId = 0;

        int32_t min = 0;
        int32_t max = 127;
        int32_t current = 0;

        /** False when the primary does not know either. */
        bool known = false;

        /** A switch: it is pressed and released rather than set. What that
         *  makes it do is the primary's to decide. */
        bool isSwitch = false;

        /** What to show instead of the number, when there is one. */
        std::string label;
    };

    struct ViewElement {
        Link::ElementId id = 0;

        /** Which preset the control lives in. On a master performance every
         *  element may name a different one. */
        uint8_t presetId = 0;

        Link::ElementType type = Link::ElementType::None;

        /* What the control is on the controller. Zero when the primary did
         * not say. Only a satellite that draws the controller's own control
         * components reads these; see ElementRecord. */
        uint8_t nativeType = 0;
        uint8_t nativeVariant = 0;
        uint8_t nativeMode = 0;
        bool visible = true;

        /** In the control set the primary's own pots are driving. */
        bool active = true;

        Rect bounds;
        uint32_t colour = 0xFFFFFF;
        uint8_t potId = 0;

        /**
         * Every pot that drives the control, one bit each - potId is the
         * first of them. A custom control may take several knobs; anything
         * else has one. Zero when an older primary did not say, in which
         * case potId is the whole answer.
         */
        uint16_t potMask = 0;

        std::string name;
        std::vector<ViewValue> values;

        /** @brief Whether this pot drives the element, by mask or by the
         *  single pot an older primary sent. */
        bool isDrivenByPot(uint8_t queriedPotId) const
        {
            if (potMask != 0) {
                return ((queriedPotId < 16)
                        && ((potMask & (1u << queriedPotId)) != 0));
            }

            return (potId == queriedPotId);
        }
    };

    /** @brief One entry of an overlay: what a value reads as. */
    struct OverlayItem {
        int16_t value = 0;
        uint32_t colour = 0;
        bool colourIsSet = false;
        std::string label;

        /** The entry's picture as packed 1-bpp source bytes, or empty. It
         *  arrives in its own message after the overlay's items. */
        std::vector<uint8_t> bitmap;
    };

    /** @brief The words a list draws, and a fader shows instead of a number. */
    struct ViewOverlay {
        uint8_t id = 0;
        std::vector<OverlayItem> items;
    };

    struct ViewPage {
        uint8_t id = 0;
        std::string name;
    };

    /** @brief A preset bank, named and coloured as the owner configured it. */
    struct PresetBankInfo {
        uint32_t colour = 0;
        std::string name;
    };

    /** @brief One of the primary's live presets, as offered to the satellite. */
    struct LivePreset {
        uint8_t presetId = 0;

        /** What the preset calls itself wherever it is loaded; how a
         *  satellite's saved assignments find it again. */
        std::string projectId;
        uint8_t bankNumber = 0;
        uint8_t slot = 0;
        uint8_t flags = 0;
        std::string name;

        /** @brief Loaded and running, rather than merely stored. */
        bool isLive(void) const
        {
            return ((flags & Link::PresetFlag::Live) != 0);
        }

        bool isPinned(void) const
        {
            return ((flags & Link::PresetFlag::Pinned) != 0);
        }

        bool isOnScreen(void) const
        {
            return ((flags & Link::PresetFlag::OnScreen) != 0);
        }

        bool isShown(void) const
        {
            return ((flags & Link::PresetFlag::Shown) != 0);
        }
    };

    /**
     * @brief What the satellite is showing.
     *
     * Elements arrive one message at a time after the header, so a view is
     * held incomplete for a moment. Nothing is drawn from it until it is
     * whole, which is what @c isComplete answers - otherwise a page would be
     * seen assembling itself.
     */
    /**
     * @brief What a note is called: "C4 (60)", "A4 (69)".
     *
     * Middle C is C4, so the octave runs from C-1 at note 0 - which is what
     * most instruments print on their pads. Here rather than on the surface
     * because the drawing needs it too, and copying five lines to save an
     * include is how two spellings of the same note start.
     */
    inline std::string noteName(uint8_t number, bool withNumber = true)
    {
        static const char *const names[12] = { "C",  "C#", "D",  "D#",
                                               "E",  "F",  "F#", "G",
                                               "G#", "A",  "A#", "B" };

        std::string name = std::string(names[number % 12])
                           + std::to_string(((int)number / 12) - 1);

        if (withNumber) {
            name += " (" + std::to_string(number) + ")";
        }

        return (name);
    }

    /**
     * @brief One control the satellite asked to have gathered.
     *
     * Fed by the controller whatever is being shown, so a knob assigned to a
     * preset running in the background still knows where its control stands -
     * which is what takeover needs, and what the master performance displays.
     */
    struct SubscriptionValue {
        uint16_t controlId = 0;
        std::string name;
        ViewValue value;
        bool known = false;
    };

    class View
    {
    public:
        void beginView(uint16_t newViewId,
                       uint8_t newPresetId,
                       uint8_t newPageNumber,
                       uint8_t newActiveSet,
                       const std::string &newTitle,
                       uint16_t newElementCount)
        {
            viewId = newViewId;
            presetId = newPresetId;
            pageNumber = newPageNumber;
            activeSet = newActiveSet;
            title = newTitle;
            expectedElements = newElementCount;
            pages.clear();
            overlays.clear();
            elements.clear();
            customFrames.clear();
        }

        void addPage(const ViewPage &page)
        {
            pages.push_back(page);
        }

        void addElement(const ViewElement &element)
        {
            elements.push_back(element);
        }

        bool isComplete(void) const
        {
            return (elements.size() >= expectedElements);
        }

        bool isEmpty(void) const
        {
            return (elements.empty() && pages.empty());
        }

        uint16_t getViewId(void) const
        {
            return (viewId);
        }

        uint8_t getPresetId(void) const
        {
            return (presetId);
        }

        uint8_t getPageNumber(void) const
        {
            return (pageNumber);
        }

        uint8_t getActiveSet(void) const
        {
            return (activeSet);
        }

        const std::string &getTitle(void) const
        {
            return (title);
        }

        const std::vector<ViewPage> &getPages(void) const
        {
            return (pages);
        }

        const std::vector<ViewOverlay> &getOverlays(void) const
        {
            return (overlays);
        }

        /**
         * @brief Add items to an overlay, starting it afresh at sequence 0.
         *
         * A view is re-sent whenever the page changes, so appending blindly
         * would grow an overlay every time it was shown.
         */
        void addOverlayItems(uint8_t overlayId,
                             uint8_t sequence,
                             const std::vector<OverlayItem> &items)
        {
            ViewOverlay *overlay = nullptr;

            for (auto &candidate : overlays) {
                if (candidate.id == overlayId) {
                    overlay = &candidate;
                    break;
                }
            }

            if (overlay == nullptr) {
                overlays.push_back(ViewOverlay());
                overlay = &overlays.back();
                overlay->id = overlayId;
            }

            if (sequence == 0) {
                overlay->items.clear();
            }

            for (const OverlayItem &item : items) {
                overlay->items.push_back(item);
            }
        }

        /**
         * @brief Put one whole overlay into the view, replacing any partial.
         *
         * How a view being assembled is seeded from the session's overlay
         * store: a primary that knows the satellite still holds an overlay
         * does not send it again, so the view has to start with what the
         * session already carries. An overlay that does arrive afterwards
         * replaces this via addOverlayItems' sequence-0 restart.
         */
        void putOverlay(const ViewOverlay &overlay)
        {
            for (auto &candidate : overlays) {
                if (candidate.id == overlay.id) {
                    candidate = overlay;
                    return;
                }
            }

            overlays.push_back(overlay);
        }

        /** @brief Attach a bitmap to one overlay entry, found by its value. */
        void attachOverlayBitmap(uint8_t overlayId,
                                 int16_t value,
                                 const uint8_t *bytes,
                                 size_t length)
        {
            for (ViewOverlay &overlay : overlays) {
                if (overlay.id != overlayId) {
                    continue;
                }

                for (OverlayItem &item : overlay.items) {
                    if (item.value == value) {
                        item.bitmap.assign(bytes, bytes + length);
                        return;
                    }
                }

                return;
            }
        }

        const std::vector<ViewElement> &getElements(void) const
        {
            return (elements);
        }

        /**
         * @brief Keep one custom control's captured paint, replacing what
         *  was held.
         *
         * The frame is the drawing itself - the ops a paint callback made on
         * the primary - and it is view state exactly as a value is: what the
         * element currently looks like. Kept here rather than beside the
         * component so a frame that arrives while the view is still being
         * assembled is waiting in it when the drawing catches up.
         */
        void setCustomFrame(Link::ElementId elementId,
                            const uint8_t *bytes,
                            size_t length)
        {
            customFrames[elementId].assign(bytes, bytes + length);
        }

        const std::map<Link::ElementId, std::vector<uint8_t>> &
            getCustomFrames(void) const
        {
            return (customFrames);
        }

        ViewElement *findElement(Link::ElementId elementId)
        {
            for (auto &element : elements) {
                if (element.id == elementId) {
                    return (&element);
                }
            }

            return (nullptr);
        }

        const ViewElement *findElement(Link::ElementId elementId) const
        {
            return (const_cast<View *>(this)->findElement(elementId));
        }

        /** @brief The page after or before the one showing, wrapping round. */
        uint8_t getAdjacentPage(int delta) const
        {
            if (pages.empty()) {
                return (pageNumber);
            }

            int index = 0;

            for (size_t i = 0; i < pages.size(); i++) {
                if (pages[i].id == pageNumber) {
                    index = (int)i;
                    break;
                }
            }

            index += delta;

            if (index < 0) {
                index = (int)pages.size() - 1;
            } else if (index >= (int)pages.size()) {
                index = 0;
            }

            return (pages[(size_t)index].id);
        }

        void clear(void)
        {
            viewId = 0;
            presetId = 0;
            pageNumber = 0;
            activeSet = 0;
            title.clear();
            expectedElements = 0;
            pages.clear();
            overlays.clear();
            elements.clear();
            customFrames.clear();
        }

    private:
        uint16_t viewId = 0;
        uint8_t presetId = 0;
        uint8_t pageNumber = 0;
        uint8_t activeSet = 0;
        std::string title;
        uint16_t expectedElements = 0;
        std::vector<ViewPage> pages;
        std::vector<ViewOverlay> overlays;
        std::vector<ViewElement> elements;

        /** Captured custom paints, by the element they belong to. */
        std::map<Link::ElementId, std::vector<uint8_t>> customFrames;
    };
} // namespace Link
