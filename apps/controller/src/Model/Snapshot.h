#pragma once

#include "Hardware.h"
#include <ArduinoJson.h>

#define MAX_SNAPSHOT_NAME_LENGTH 14

class Snapshot
{
public:
    Snapshot(const char *filename);
    virtual ~Snapshot() = default;

    const char *getName(void) const;
    uint32_t getColour(void) const;
    uint16_t getColour565(void) const;

private:
    bool parse(File &file);

    char name[MAX_SNAPSHOT_NAME_LENGTH + 1];
    uint32_t colour;
};
