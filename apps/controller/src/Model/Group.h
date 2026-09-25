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
#include "System.h"

class Group
{
private:
    static constexpr uint8_t maxLabelLength = 40;

public:
    enum class Variant { Default = 0, Highlighted = 1 };

    Group()
        : id(0),
          pageId(0),
          colour(Colours565::white),
          visible(true),
          component(nullptr)
    {
        setLabel(nullptr);
        variant = (uint8_t)Variant::Default;
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
          bounds(newBounds),
          visible(true),
          component(nullptr)
    {
        setLabel(newLabel);
        variant = (uint8_t)newVariant;
    }

    virtual ~Group() = default;

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

    void setLabel(const char *newLabel)
    {
        if (newLabel) {
            copyString(label, newLabel, maxLabelLength);
        } else {
            *label = '\0';
        }
    }

    const char *getLabel(void) const
    {
        return (label);
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
        System::logger.write(LOG_ERROR, "setColour: 0x%06x", newColour);
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
        component = newComponent;
    }

    void resetComponent(void)
    {
        component = nullptr;
    }

    Component *getComponent(void) const
    {
        return (component);
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
        uint8_t variant : 2;
    };

    char label[maxLabelLength + 1];
    Rectangle bounds;

    Component *component;
};

typedef std::map<uint16_t, Group> Groups;
