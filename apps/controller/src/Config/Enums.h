#pragma once

enum class AppEventType {
    switchPage,
    switchPageNext,
    switchPagePrev,
    switchPreset,
    switchPresetNext,
    switchPresetPrev,
    switchControlSet,
    switchControlSetNext,
    switchControlSetPrev,
    invalid
};

enum class ActiveControlSetType { none, dim, bars, background };

AppEventType translateAppEventType(const char *event);
const char *translateAppEventTypeToText(AppEventType appEventType);
ActiveControlSetType translateControlSetType(const char *typeText);
const char *translateControlSetTypeToText(ActiveControlSetType type);
