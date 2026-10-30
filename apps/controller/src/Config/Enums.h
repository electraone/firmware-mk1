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

/**
 * @brief Which control set a page opens on, as the mk2 has it.
 *
 * Global keeps the one set across pages, local gives every page its own, which
 * it remembers, and custom uses the default the preset gives each page.
 */
enum class ActiveControlSetMode { global, local, custom };

AppEventType translateAppEventType(const char *event);
const char *translateAppEventTypeToText(AppEventType appEventType);
ActiveControlSetType translateControlSetType(const char *typeText);
ActiveControlSetMode translateControlSetMode(const char *modeText);
const char *translateControlSetTypeToText(ActiveControlSetType type);
