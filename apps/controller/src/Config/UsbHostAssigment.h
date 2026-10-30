#pragma once

#include "helpers.h"

struct UsbHostAssigment {
    UsbHostAssigment(const char *newPattern, uint8_t newPort) : port(newPort)
    {
        copyString(pattern, newPattern, MaxPatternLength);
        toUpperCase(pattern);
    }

    static const int MaxPatternLength = 20;
    char pattern[MaxPatternLength + 1];
    uint8_t port;
};
