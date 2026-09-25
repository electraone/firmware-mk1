#pragma once

#include "Colours.h"

struct PresetBank {
    PresetBank() : id(0), colour(Colours565::white)
    {
        copyString(name, "", MaxNameLength);
    }

    PresetBank(uint8_t newId, const char *newName, uint32_t newColour)
        : id(newId), colour(newColour)
    {
        copyString(name, newName, MaxNameLength);
    }

    static const int MaxNameLength = 20;

    uint8_t id;
    char name[MaxNameLength + 1];
    uint32_t colour;
};
