#pragma once

#include "Presets.h"
#include "Snapshots.h"
#include "Info.h"
#include "Config/Config.h"

struct Model {
    explicit Model(const char *newAppSandbox, const Config &newConfig)
        : presets(newAppSandbox,
                  newConfig.uiFeatures.keepPresetState,
                  newConfig.uiFeatures.loadPresetStateOnStartup),
          snapshots(newAppSandbox),
          currentPreset(presets.preset)
    {
    }

    virtual ~Model() = default;

    Presets presets;
    Snapshots snapshots;
    Preset &currentPreset;
    Info info;
};
