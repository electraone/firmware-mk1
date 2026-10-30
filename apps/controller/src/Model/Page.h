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
 * @file Page.h
 *
 * @brief Implements a Page within a Preset.
 */

#pragma once

#include "System.h"

class Page
{
public:
    Page();
    Page(uint8_t id,
         const char *newName,
         uint8_t newDefaultControlSetId,
         bool shouldBeHidden);

    virtual ~Page() = default;

    /**
     * @brief Checks if the Page is valid.
     *
     * @return true if the Page is valid
     */
    bool isValid(void) const;

    /**
     * @brief Returns the id of the Page.
     *
     * @return uint8_t the id of the Page
     */
    uint8_t getId(void) const;

    /**
     * @brief Sets the name of the Page.
     *
     * @param newName a new name to be assigned to the Page
     */
    void setName(const char *newName);

    /**
     * @brief Returns the name of the Page.
     *
     * @return const char* the name of the Page
     */
    const char *getName(void) const;

    /**
     * @brief Sets the default control set id of the Page.
     * 
     * Control set is an active row of controls on the display.
     *
     * @param newDefaultControlSetId a new default control set id to be
     *  assigned to the Page
     */
    void setDefaultControlSetId(uint8_t newDefaultControlSetId);

    /**
     * @brief Returns the default control set id of the Page.
     *
     * @return uint8_t the default control set id of the Page
     */
    uint8_t getDefaultControlSetId(void) const;

    /**
     * @brief Indicates whether the Page has any objects.
     * 
     * Objects are controls and groups.
     * 
     * @return true if the Page has any objects 
     */
    bool hasObjects(void) const;

    /**
     * @brief Sets the flag indicating whether the Page has any objects.
     * 
     * @param shouldHaveObjects a new value of the flag
     */
    void setHasObjects(bool shouldHaveObjects);

    /**
     * @brief Indicates whether the Page is hidden.
     * 
     * Hidden pages are not displayed on the controller page selection screen.
     * 
     * @return true if the Page is hidden
     */
    bool isHidden(void) const;

    /**
     * @brief Sets the flag indicating whether the Page is hidden.
     * 
     * @param shouldBeHidden a new value of the flag
     */
    void setHidden(bool shouldBeHidden);

    /**
     * @brief Indicates whether the Page is to be displayed on the controller
     * page selection screen.
     * 
     * @return true if the Page is to be displayed
     */
    bool isToBeDisplayed(void) const;

    /**
     * @brief Prints the Page to the console.
     * 
     * @param logLevel the log level to be used
     */
    void print(uint8_t logLevel = LOG_TRACE) const;

private:
    static constexpr uint8_t MaxNameLength = 20;

    struct {
        uint8_t id : 4;
        uint8_t defaultControlSetId : 2;
        bool objectsPresent : 1;
        bool hidden : 1;
    };

    char name[MaxNameLength + 1];
};

typedef std::map<uint8_t, Page> Pages;
