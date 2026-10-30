#pragma once

#include "Preset.h"

class PresetSlot
{
public:
    PresetSlot() : alreadyLoaded(false), used(false)
    {
        presetName[0] = '\0';
        projectId[0] = '\0';
    }

    PresetSlot(const char *newPresetName, const char *newProjectId)
        : alreadyLoaded(true), used(true)
    {
        copyString(presetName, newPresetName, Preset::MaxNameLength);
        copyString(projectId, newProjectId, Preset::MaxProjectIdLength);
    }

    void setPresetName(const char *newPresetName)
    {
        copyString(presetName, newPresetName, Preset::MaxNameLength);
    }

    const char *getPresetName(void) const
    {
        return (presetName);
    }

    void setProjectId(const char *newProjectId)
    {
        copyString(projectId, newProjectId, Preset::MaxProjectIdLength);
    }

    const char *getProjectId(void) const
    {
        return (projectId);
    }

    void setAlreadyLoaded(bool shouldBeSet)
    {
        alreadyLoaded = shouldBeSet;
    }

    bool hasBeenAlreadyLoaded(void) const
    {
        return (alreadyLoaded);
    }

    void setUsed(bool shouldBeSet)
    {
        used = shouldBeSet;
    }

    bool isUsed(void) const
    {
        return (used);
    }

    void set(const char *newPresetName, const char *newProjectId)
    {
        setPresetName(newPresetName);
        setProjectId(newProjectId);
        setUsed(true);
    }

    void clear(void)
    {
        setPresetName("");
        setProjectId("");
        setUsed(false);
    }

private:
    char presetName[Preset::MaxNameLength + 1];
    char projectId[Preset::MaxProjectIdLength + 1];
    struct {
        bool alreadyLoaded : 1;
        bool used : 1;
    };
};
