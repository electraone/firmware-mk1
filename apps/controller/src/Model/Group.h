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
 * @file Group.h
 *
 * @brief Implements an on-screen visual for grouping Controls.
 * 
 * @todo Component is present here, but it is not used. It should be removed.
 */

#pragma once

#include "Colours.h"
#include "Component.h"
#include "ComponentTable.h"
#include "Hardware.h"
#include "System.h"
#include "TextBase.h"
#include "VramStore.h"

class Group
{
private:
    static constexpr uint8_t maxLabelLength = 40;

public:
    /* Numbered as the mk2 numbers its control variants, gaps and all, so that
     * the two read the same preset the same way. */
    enum class Variant { Default = 0, Highlighted = 1, Buttonlike = 6 };

    /** The face a group draws its label in. */
    using Font = TextFont;

    /** The face a group's label has unless the preset names one: small, as
     *  v3.5.4 drew every group label, where a control's text is medium. */
    static constexpr Font defaultFont(void)
    {
        return (Font{ TextSize::small,
                      TextFace::regular,
                      TextSpacing::proportional });
    }

    Group()
        : id(0),
          pageId(0),
          colour(Colours565::white),
          visible(true),
          labelHandle(VramStore::invalidHandle)
    {
        variant = (uint8_t)Variant::Default;
        setFont(defaultFont());
    }

    Group(uint16_t newId,
          uint8_t newPageId,
          const Rectangle &newBounds,
          const char *newLabel,
          uint32_t newColour,
          Group::Variant newVariant)
        : id(newId),
          pageId(newPageId),
          colour(newColour),
          labelHandle(VramStore::invalidHandle),
          bounds(newBounds),
          visible(true)
    {
        setLabel(newLabel);
        variant = (uint8_t)newVariant;
        setFont(defaultFont());
    }

    ~Group() = default;

    bool isValid(void) const
    {
        return (id != 0);
    }

    void setId(uint16_t newId)
    {
        id = newId;
    }

    uint16_t getId(void) const
    {
        return (id);
    }

    /* The label lives in the display's memory, as a control's name does.
     * Forty-one bytes of label in every group of every preset was most of
     * what a group cost, and the Access Virus preset's sixty groups carry
     * no label at all. */
    void setLabel(const char *newLabel)
    {
        if ((newLabel == nullptr) || (newLabel[0] == '\0')) {
            if (labelHandle != VramStore::invalidHandle) {
                Hardware::memory.stringPool.updateString(labelHandle, "");
            }
            return;
        }

        if (labelHandle == VramStore::invalidHandle) {
            labelHandle =
                Hardware::memory.stringPool.reserve(maxLabelLength + 1);

            if (labelHandle == VramStore::invalidHandle) {
                return;
            }
        }

        char text[maxLabelLength + 1];

        copyString(text, newLabel, maxLabelLength);
        Hardware::memory.stringPool.updateString(labelHandle, text);
    }

    /**
     * @brief The label, fetched from the display's memory.
     *
     * @attention the returned pointer is a shared buffer that the next call
     * overwrites; a caller that keeps a label copies it.
     */
    const char *getLabel(void) const
    {
        if (labelHandle == VramStore::invalidHandle) {
            labelBuffer[0] = '\0';
            return (labelBuffer);
        }

        return (Hardware::memory.stringPool.loadString(
            labelHandle, labelBuffer, sizeof(labelBuffer)));
    }

    void setPageId(uint8_t newPageId)
    {
        pageId = newPageId;
    }

    uint8_t getPageId(void) const
    {
        return (pageId);
    }

    void setColour(uint32_t newColour)
    {
        colour = newColour;
    }

    uint32_t getColour(void) const
    {
        return (colour);
    }

    uint16_t getColour565(void) const
    {
        return (Colours888::toRGB565(colour));
    }

    void setVariant(Variant newVariant)
    {
        variant = (uint8_t)newVariant;
    }

    Variant getVariant(void) const
    {
        return ((Variant)variant);
    }

    /* Four bits, taken from the ones the bitfield already had going spare.
     * A group is held for every group of every preset, so a struct or three
     * bytes here would be paid for on every one of them. */
    void setFont(const Font &newFont)
    {
        fontSize = (uint8_t)newFont.size;
        fontFace = (uint8_t)newFont.face;
        fontSpacing = (uint8_t)newFont.spacing;
    }

    Font getFont(void) const
    {
        return (Font{
            (TextSize)fontSize, (TextFace)fontFace, (TextSpacing)fontSpacing });
    }

    void setVisible(bool shouldBeVisible)
    {
        visible = shouldBeVisible;
    }

    bool isVisible(void) const
    {
        return ((bool)visible);
    }

    void setBounds(const Rectangle &newBounds)
    {
        bounds = newBounds;
    }

    Rectangle getBounds(void) const
    {
        return (bounds);
    }

    void setComponent(Component *newComponent)
    {
        ComponentTable::set(this, newComponent);
    }

    void resetComponent(void)
    {
        ComponentTable::set(this, nullptr);
    }

    Component *getComponent(void) const
    {
        return (ComponentTable::get(this));
    }

    void print(uint8_t logLevel = LOG_TRACE) const
    {
        System::logger.write(
            logLevel,
            "--[Group]-----------------------------------------------");
        System::logger.write(logLevel, "address: 0x%08x", this);
        System::logger.write(logLevel, "id: %d", getId());
        System::logger.write(logLevel, "label: %s", getLabel());
        System::logger.write(logLevel, "pageId: %d", getPageId());
        System::logger.write(logLevel, "colour: 0x%06x", getColour());
        System::logger.write(logLevel, "visible: %d", isVisible());
        System::logger.write(logLevel, "variant: %d", getVariant());
        //        getBounds().print(logLevel);
    }

    static Variant translateVariant(const char *variantText)
    {
        if (variantText) {
            if (strcmp(variantText, "highlighted") == 0) {
                return (Variant::Highlighted);
            } else if (strcmp(variantText, "buttonlike") == 0) {
                return (Variant::Buttonlike);
            }
        }
        return (Variant::Default);
    }

private:
    struct {
        uint16_t id : 10;
        uint8_t pageId : 4;
        uint8_t visible : 1;
        uint32_t colour : 24;
        uint8_t variant : 3;
        uint8_t fontSize : 2;
        uint8_t fontFace : 1;
        uint8_t fontSpacing : 1;
    };

    VramStore::Handle labelHandle;
    Rectangle bounds;

    /** Shared scratch the label is handed out through. See getLabel(). */
    static inline char labelBuffer[maxLabelLength + 1];
};

static_assert(sizeof(Group) == 20,
              "sizeof(Group) has changed - every preset pays for it once per "
              "group, so check that it was meant");

typedef std::map<uint16_t, Group> Groups;
