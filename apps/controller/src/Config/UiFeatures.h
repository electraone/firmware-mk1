#pragma once

#include "Enums.h"

struct UiFeatures {
    UiFeatures()
        : touchSwitchControlSets(true),
          activeControlSetMode(ActiveControlSetMode::global),
          activeControlSetType(ActiveControlSetType::dim),
          keepPresetState(true),
          loadPresetStateOnStartup(false)
    {
    }

    bool touchSwitchControlSets;
    ActiveControlSetMode activeControlSetMode;
    ActiveControlSetType activeControlSetType;
    bool keepPresetState;
    bool loadPresetStateOnStartup;
};
