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
 * @file Context.cpp
 *
 * @brief A collection of attributes important for the System.
 * 
 * @todo Contains application specific data. Needs to be changed.
 */

#pragma once

#include "helpers.h"
#include "settings.h"
#include <cstdio>

class Context
{
public:
    Context()
        : appName(nullptr),
          presetName(nullptr),
          midiLearnEnabled(false),
          loadFiles(true),
          touchEnabled(true)
    {
    }

    void setAppName(const char *newAppName)
    {
        appName = newAppName;
        snprintf(
            currentConfigFile, MAX_FILENAME_LENGTH, "%s/setup.cfg", appName);
        snprintf(currentTempFile, MAX_FILENAME_LENGTH, "%s/file.tmp", appName);
    }

    void setPresetName(const char *newPresetName)
    {
        presetName = newPresetName;
    }

    const char *getAppName(void)
    {
        return (appName);
    }

    const char *getPresetName(void)
    {
        return (presetName);
    }

    void setMidiLearn(bool shouldBeEnabled)
    {
        midiLearnEnabled = shouldBeEnabled;
    }

    bool getMidiLearn(void)
    {
        return (midiLearnEnabled);
    }

    void setLoadDefaultFiles(bool shouldLoadFiles)
    {
        loadFiles = shouldLoadFiles;
    }

    bool getLoadDefaultFiles(void)
    {
        return (loadFiles);
    }

    void setTouchEnabled(bool shouldTouchBeEnabled)
    {
        touchEnabled = shouldTouchBeEnabled;
    }

    bool getTouchEnabled(void)
    {
        return (touchEnabled);
    }

    // \todo this is not OK
    void setCurrentFile(uint16_t presetId)
    {
        if (appName) {
            formatPresetFilename(
                currentPresetFile, MAX_FILENAME_LENGTH, presetId);
            formatLuaFilename(currentLuaFile, MAX_FILENAME_LENGTH, presetId);
        }
    }

    char *getCurrentPresetFile(void)
    {
        return (currentPresetFile);
    }

    char *getCurrentLuaFile(void)
    {
        return (currentLuaFile);
    }

    char *getCurrentConfigFile(void)
    {
        return (currentConfigFile);
    }

    char *getCurrentTempFile(void)
    {
        return (currentTempFile);
    }

    void formatPresetFilename(char *buffer, size_t maxSize, uint16_t fileNumber)
    {
        snprintf(buffer, maxSize - 1, "/%s/p%03d.epr", appName, fileNumber);
    }

    void formatLuaFilename(char *buffer, size_t maxSize, uint16_t fileNumber)
    {
        snprintf(buffer, maxSize - 1, "%s/p%03d.lua", appName, fileNumber);
    }

    /**
     * @brief Where a staged upload puts a file until its transaction commits.
     *
     * On the card rather than in memory: a preset is tens of kilobytes and
     * there is a quarter of a megabyte of RAM.
     */
    void formatCacheDirectory(char *buffer, size_t maxSize)
    {
        snprintf(buffer, maxSize - 1, "%s/cache", appName);
    }

    void formatCacheFilename(char *buffer, size_t maxSize, uint8_t fileId)
    {
        snprintf(buffer, maxSize - 1, "%s/cache/f%03d", appName, fileId);
    }

    void formatPreloadedFilename(char *buffer,
                                 size_t maxSize,
                                 const char *preloadedPath,
                                 const char *extension)
    {
        snprintf(buffer,
                 maxSize - 1,
                 "%s/presets/%s.%s",
                 appName,
                 preloadedPath,
                 extension);
    }

private:
    char currentPresetFile[MAX_FILENAME_LENGTH];
    char currentLuaFile[MAX_FILENAME_LENGTH];
    char currentConfigFile[MAX_FILENAME_LENGTH];
    char currentTempFile[MAX_FILENAME_LENGTH];
    const char *appName;
    const char *presetName;

    bool midiLearnEnabled;
    bool loadFiles;
    bool touchEnabled;
};
