#include "Enums.h"
#include <cstring>

ActiveControlSetType translateControlSetType(const char *typeText)
{
    if (typeText) {
        if (strcmp(typeText, "none") == 0) {
            return (ActiveControlSetType::none);
        } else if (strcmp(typeText, "dim") == 0) {
            return (ActiveControlSetType::dim);
        } else if (strcmp(typeText, "bars") == 0) {
            return (ActiveControlSetType::bars);
        } else if (strcmp(typeText, "background") == 0) {
            return (ActiveControlSetType::background);
        }
    }
    return (ActiveControlSetType::dim);
}

const char *translateControlSetTypeToText(ActiveControlSetType type)
{
    if (type == ActiveControlSetType::none) {
        return "none";
    } else if (type == ActiveControlSetType::dim) {
        return "dim";
    } else if (type == ActiveControlSetType::bars) {
        return "bars";
    } else if (type == ActiveControlSetType::background) {
        return "background";
    }

    return ("dim");
}

AppEventType translateAppEventType(const char *event)
{
    if (event) {
        if (strcmp(event, "switchPage") == 0) {
            return (AppEventType::switchPage);
        } else if (strcmp(event, "switchPageNext") == 0) {
            return (AppEventType::switchPageNext);
        } else if (strcmp(event, "switchPagePrev") == 0) {
            return (AppEventType::switchPagePrev);
        } else if (strcmp(event, "switchPreset") == 0) {
            return (AppEventType::switchPreset);
        } else if (strcmp(event, "switchPresetNext") == 0) {
            return (AppEventType::switchPresetNext);
        } else if (strcmp(event, "switchPresetPrev") == 0) {
            return (AppEventType::switchPresetPrev);
        } else if (strcmp(event, "switchControlSet") == 0) {
            return (AppEventType::switchControlSet);
        } else if (strcmp(event, "switchControlSetNext") == 0) {
            return (AppEventType::switchControlSetNext);
        } else if (strcmp(event, "switchControlSetPrev") == 0) {
            return (AppEventType::switchControlSetPrev);
        }
    }
    return AppEventType::invalid;
}

const char *translateAppEventTypeToText(AppEventType appEventType)
{
    if (appEventType == AppEventType::switchPage) {
        return "switchPage";
    } else if (appEventType == AppEventType::switchPageNext) {
        return "switchPageNext";
    } else if (appEventType == AppEventType::switchPagePrev) {
        return "switchPagePrev";
    } else if (appEventType == AppEventType::switchPreset) {
        return "switchPreset";
    } else if (appEventType == AppEventType::switchPresetNext) {
        return "switchPresetNext";
    } else if (appEventType == AppEventType::switchPresetPrev) {
        return "switchPresetPrev";
    } else if (appEventType == AppEventType::switchControlSet) {
        return "switchControlSet";
    } else if (appEventType == AppEventType::switchControlSetNext) {
        return "switchControlSetNext";
    } else if (appEventType == AppEventType::switchControlSetPrev) {
        return "switchControlSetPrev";
    }

    return "unknown";
}
