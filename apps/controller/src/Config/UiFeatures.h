#pragma once

#include "Enums.h"

struct UiFeatures {
    UiFeatures()
        : touchSwitchControlSets(true),
          resetActiveControlSet(false),
          activeControlSetType(ActiveControlSetType::dim),
          keepPresetState(true),
          loadPresetStateOnStartup(false)
    {
    }

    bool touchSwitchControlSets;
    bool resetActiveControlSet;
    ActiveControlSetType activeControlSetType;
    bool keepPresetState;
    bool loadPresetStateOnStartup;
};
