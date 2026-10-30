#include "Config/Config.h"
#include "PersistentStorage.h"
#include "Hardware.h"
#include "System.h"
#include "JsonTools.h"

/** Config contructor
 *  an object that keeps information about the Electra base system settings that
 *  can be adjusted by the user
 */
Config::Config()
{
}

bool Config::load(void)
{
    return (load(System::context.getCurrentConfigFile()));
}

bool Config::load(const char *filename)
{
    File file;

    System::logger.write(LOG_INFO, "Config::load: file: filename=%s", filename);

    file = Hardware::sdcard.createInputStream(filename);

    if (!file) {
        System::logger.write(
            LOG_ERROR, "Config::load: cannot open setup file: %s", filename);
        return (false);
    }

    file.setTimeout(100);

    if (!parse(file)) {
        System::logger.write(LOG_ERROR,
                             "Config::load: cannot parse setup: filename=%s",
                             filename);
        file.close();
        return (false);
    } else {
        file.close();
    }

    return (true);
}

bool Config::parse(File &file)
{
    parseRouter(file);
    parsePresetBanks(file);
    parseUsbHostAssigments(file);
    parseMidiControl(file);
    parseUiFeatures(file);

    return (true);
}

bool Config::parseRouter(File &file)
{
    const size_t capacityRouter = JSON_OBJECT_SIZE(1) + 1000;
    const size_t capacityFilter = JSON_OBJECT_SIZE(1) + 100;
    StaticJsonDocument<capacityRouter> doc;
    StaticJsonDocument<capacityFilter> filter;

    filter["router"] = true;

    if (file.seek(0) == false) {
        System::logger.write(LOG_ERROR,
                             "Config::parseRouter: cannot rewind the file");
        return (false);
    }

    DeserializationError err =
        deserializeJson(doc, file, DeserializationOption::Filter(filter));

    if (err) {
        System::logger.write(
            LOG_ERROR, "Config::parseRouter: parsing failed: %s", err.c_str());
        return (false);
    }

    if (doc["router"]) {
        router.usbDevToUsbHost = doc["router"]["usbDevToUsbHost"].as<bool>();
        router.usbDevToMidiIo = doc["router"]["usbDevToMidiIo"].as<bool>();
        router.usbDevToMidiControl =
            doc["router"]["usbDevToMidiControl"] | true;
        router.usbHostToUsbDev = doc["router"]["usbHostToUsbDev"].as<bool>();
        router.usbHostToMidiIo = doc["router"]["usbHostToMidiIo"].as<bool>();
        router.midiIoToUsbDev = doc["router"]["midiIoToUsbDev"].as<bool>();
        router.midiIoToUsbHost = doc["router"]["midiIoToUsbHost"].as<bool>();
        router.midiIo1Thru = doc["router"]["midiIo1Thru"].as<bool>();
        router.midiIo2Thru = doc["router"]["midiIo2Thru"].as<bool>();

        router.midiControlPort = doc["router"]["midiControlPort"] | 2;
        router.midiControlChannel = doc["router"]["midiControlChannel"] | 0;
        router.midiControlDrop = doc["router"]["midiControlDrop"] | true;

        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: usbDevToUsbHost=%d",
                             router.usbDevToUsbHost);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: usbDevToMidiIo=%d",
                             router.usbDevToMidiIo);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: usbDevToMidiControl=%d",
                             router.usbDevToMidiControl);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: usbHostToUsbDev=%d",
                             router.usbHostToUsbDev);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: usbHostToMidiIo=%d",
                             router.usbHostToMidiIo);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: midiIoToUsbDev=%d",
                             router.midiIoToUsbDev);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: midiIoToUsbHost=%d",
                             router.midiIoToUsbHost);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: midiIo1Thru=%d",
                             router.midiIo1Thru);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: midiIo2Thru=%d",
                             router.midiIo2Thru);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: midiControlPort=%d",
                             router.midiControlPort);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: midiControlChannel=%d",
                             router.midiControlChannel);
        System::logger.write(LOG_TRACE,
                             "Config::parseRouter: midiControlDrop=%d",
                             router.midiControlDrop);
    } else {
        System::logger.write(LOG_WARNING,
                             "Config::parseRouter: no router definition found");
    }

    return (true);
}

bool Config::parsePresetBanks(File &file)
{
    const size_t capacityPresetBanks = JSON_OBJECT_SIZE(1) + 2000;
    const size_t capacityFilter = JSON_OBJECT_SIZE(1) + 100;
    StaticJsonDocument<capacityPresetBanks> doc;
    StaticJsonDocument<capacityFilter> filter;

    filter["presetBanks"] = true;

    resetPresetBanks();

    if (file.seek(0) == false) {
        System::logger.write(
            LOG_ERROR, "Config::parsePresetBanks: cannot rewind the file");
        return (false);
    }

    DeserializationError err =
        deserializeJson(doc, file, DeserializationOption::Filter(filter));

    if (err) {
        System::logger.write(LOG_ERROR,
                             "Config::parsePresetBanks: parsing failed: %s",
                             err.c_str());
        return (false);
    }

    JsonArray jPresetBanks = doc["presetBanks"];

    if (doc["presetBanks"]) {
        for (JsonVariant jPresetBank : jPresetBanks) {
            uint8_t id = jPresetBank["id"];
            const char *name = jPresetBank["name"];
            const char *colourRGB888 = jPresetBank["color"];

            uint32_t colour = Colours565::fromString(colourRGB888);
            presetBanks[id - 1] = PresetBank(id, name, colour);

            System::logger.write(
                LOG_TRACE,
                "Config::parsePresetBanks: preset bank: id=%d, name=%s, "
                "colour=%s",
                id,
                name,
                colourRGB888);
        }
    } else {
        System::logger.write(
            LOG_WARNING,
            "Config::parsePresetBanks: no presetBanks definition found");
    }

    return (true);
}

bool Config::parseUsbHostAssigments(File &file)
{
    const size_t capacityAssigments = JSON_OBJECT_SIZE(1) + 1000;
    const size_t capacityFilter = JSON_OBJECT_SIZE(1) + 100;
    StaticJsonDocument<capacityAssigments> doc;
    StaticJsonDocument<capacityFilter> filter;

    filter["usbHostAssigments"] = true;

    // any previous USB host assigments
    usbHostAssigments.clear();

    if (file.seek(0) == false) {
        System::logger.write(
            LOG_ERROR,
            "Config::parseUsbHostAssigments: cannot rewind the file");
        return (false);
    }

    DeserializationError err =
        deserializeJson(doc, file, DeserializationOption::Filter(filter));

    if (err) {
        System::logger.write(
            LOG_ERROR,
            "Config::parseUsbHostAssigments: parsing failed: %s",
            err.c_str());
        return (false);
    }

    JsonArray jAssigments = doc["usbHostAssigments"];

    if (jAssigments) {
        usbHostAssigments = std::vector<UsbHostAssigment>();

        for (JsonVariant jAssigment : jAssigments) {
            const char *pattern = jAssigment["pattern"].as<char *>();
            uint8_t port = jAssigment["port"].as<uint8_t>();

            if (port > 0) {
                port--;
            }

            if (port > 2) {
                port = 0;
            }

            usbHostAssigments.push_back(UsbHostAssigment(pattern, port));
            System::logger.write(
                LOG_TRACE,
                "Config::parseUsbHostAssigments: usb assigment: pattern=%s, "
                "port=%d",
                pattern,
                port);
        }
    } else {
        System::logger.write(
            LOG_WARNING,
            "Config::parseUsbHostAssigments:: no usbHostAssigments definition found");
    }

    return (true);
}

bool Config::parseMidiControl(File &file)
{
    const size_t capacityMidiControls = JSON_OBJECT_SIZE(1) + 3000;
    const size_t capacityFilter = JSON_OBJECT_SIZE(1) + 200;
    StaticJsonDocument<capacityMidiControls> doc;
    StaticJsonDocument<capacityFilter> filter;

    // clear any previous MIDI Controls
    midiControls.clear();

    if (file.seek(0) == false) {
        System::logger.write(
            LOG_ERROR, "Config::parseMidiControl: cannot rewind the file");
        return (false);
    }

    if (findElement(file, "\"midiControl\"", ARRAY) == false) {
        System::logger.write(
            LOG_ERROR, "Config::parseMidiControl: midiControl array not found");
        return (true);
    }

    if (isElementEmpty(file)) {
        System::logger.write(
            LOG_INFO, "Config::parseMidiControl: no midiControl defined");
        return (true);
    }

    do {
        DeserializationError err = deserializeJson(doc, file);

        if (err) {
            System::logger.write(LOG_ERROR,
                                 "Config::parseMidiControl: parsing failed: %s",
                                 err.c_str());
            return (false);
        }

        JsonObject jMidiControl = doc.as<JsonObject>();

        if (jMidiControl) {
            const char *midiMessage = jMidiControl["midiMessage"].as<char *>();
            uint8_t parameterNumber =
                jMidiControl["parameterNumber"].as<uint8_t>();

            if (jMidiControl["command"]) {
                const char *event =
                    jMidiControl["command"]["type"].as<char *>();
                uint8_t eventParameter1 =
                    jMidiControl["command"]["pageId"].as<uint8_t>();
                uint8_t eventParameter2 =
                    jMidiControl["command"]["controlSetId"].as<uint8_t>();

                if (eventParameter1 > 12) {
                    eventParameter1 = 0;
                }

                if (eventParameter2 > 3) {
                    eventParameter2 = 3;
                }

                if (parameterNumber > 127) {
                    parameterNumber = 0;
                }

                AppEventType eventType = translateAppEventType(event);
                MidiMessage::Type midiMessageType =
                    MidiMessage::translateType(midiMessage);

                midiControls.push_back(MidiControl(eventType,
                                                   eventParameter1,
                                                   eventParameter2,
                                                   midiMessageType,
                                                   parameterNumber));
                System::logger.write(
                    LOG_TRACE,
                    "Config::parseMidiControl: midi control assigment: "
                    "event=%s (%d), eventParameter1=%d, eventParameter2=%d, "
                    "midiMessage=%s (%d), parameterNumber=%d",
                    event,
                    eventType,
                    eventParameter1,
                    eventParameter2,
                    midiMessage,
                    midiMessageType,
                    parameterNumber);
            } else {
                const char *event = jMidiControl["event"].as<char *>();
                uint8_t eventParameter =
                    jMidiControl["eventParameter"].as<uint8_t>();

                if (parameterNumber > 127) {
                    parameterNumber = 0;
                }

                AppEventType eventType = translateAppEventType(event);
                MidiMessage::Type midiMessageType =
                    MidiMessage::translateType(midiMessage);

                midiControls.push_back(MidiControl(eventType,
                                                   eventParameter,
                                                   midiMessageType,
                                                   parameterNumber));
                System::logger.write(
                    LOG_TRACE,
                    "Config::parseMidiControl: midi control assigment: "
                    "event=%s (%d), eventParameter=%d, midiMessage=%s (%d), "
                    "parameterNumber=%d",
                    event,
                    eventType,
                    eventParameter,
                    midiMessage,
                    midiMessageType,
                    parameterNumber);
            }
        } else {
            System::logger.write(
                LOG_WARNING,
                "Config::parseMidiControl:: no midiControl definition found");
        }
    } while (file.findUntil(",", "]"));

    return (true);
}

bool Config::parseUiFeatures(File &file)
{
    const size_t capacityUiFeatures = JSON_OBJECT_SIZE(1) + 1000;
    const size_t capacityFilter = JSON_OBJECT_SIZE(1) + 100;
    StaticJsonDocument<capacityUiFeatures> doc;
    StaticJsonDocument<capacityFilter> filter;

    filter["uiFeatures"] = true;

    if (file.seek(0) == false) {
        System::logger.write(LOG_ERROR,
                             "Config::parseUiFeatures: cannot rewind the file");
        return (false);
    }

    DeserializationError err =
        deserializeJson(doc, file, DeserializationOption::Filter(filter));

    if (err) {
        System::logger.write(LOG_ERROR,
                             "Config::parseUiFeatures: parsing failed: %s",
                             err.c_str());
        return (false);
    }

    if (doc["uiFeatures"]) {
        uiFeatures.touchSwitchControlSets =
            doc["uiFeatures"]["touchSwitchControlSets"].as<bool>();
        System::logger.write(
            LOG_TRACE,
            "Config::parseUiFeatures: touchSwitchControlSets=%d",
            uiFeatures.touchSwitchControlSets);
        uiFeatures.resetActiveControlSet =
            doc["uiFeatures"]["resetActiveControlSet"].as<bool>();
        System::logger.write(
            LOG_TRACE,
            "Config::parseUiFeatures: resetActiveControlSet=%d",
            uiFeatures.resetActiveControlSet);
        uiFeatures.activeControlSetType = translateControlSetType(
            doc["uiFeatures"]["activeControlSetType"].as<char *>());
        System::logger.write(LOG_TRACE,
                             "Config::parseUiFeatures: activeControlSetType=%d",
                             uiFeatures.activeControlSetType);
        uiFeatures.keepPresetState =
            doc["uiFeatures"]["keepPresetState"].as<bool>();
        System::logger.write(LOG_TRACE,
                             "Config::parseUiFeatures: keepPresetState=%d",
                             uiFeatures.keepPresetState);
        uiFeatures.loadPresetStateOnStartup =
            doc["uiFeatures"]["loadPresetStateOnStartup"].as<bool>();
        System::logger.write(
            LOG_TRACE,
            "Config::parseUiFeatures: loadPresetStateOnStartup=%d",
            uiFeatures.loadPresetStateOnStartup);
    } else {
        System::logger.write(
            LOG_TRACE,
            "Config::parseUiFeatures: no UiFeatures definition found");
    }

    return (true);
}

void Config::resetPresetBanks(void)
{
    presetBanks[0] = PresetBank(1, "BANK #1", Colours565::white);
    presetBanks[1] = PresetBank(2, "BANK #2", Colours565::red);
    presetBanks[2] = PresetBank(3, "BANK #3", Colours565::orange);
    presetBanks[3] = PresetBank(4, "BANK #4", Colours565::blue);
    presetBanks[4] = PresetBank(5, "BANK #5", Colours565::green);
    presetBanks[5] = PresetBank(6, "BANK #6", Colours565::purple);
}

void Config::resetUiFeatures(void)
{
    uiFeatures.touchSwitchControlSets = true;
    uiFeatures.resetActiveControlSet = true;
    uiFeatures.activeControlSetType = ActiveControlSetType::dim;
    uiFeatures.keepPresetState = false;
    uiFeatures.loadPresetStateOnStartup = false;
}

void Config::useDefault(void)
{
    resetPresetBanks();
    resetUiFeatures();
}

uint8_t Config::getUsbHostAssigment(const char *productName)
{
    char productNameUpperCase[50];

    copyString(
        productNameUpperCase, productName, sizeof(productNameUpperCase) - 1);
    toUpperCase(productNameUpperCase);

    for (auto &assigment : usbHostAssigments) {
        if (strstr(productNameUpperCase, assigment.pattern)) {
            System::logger.write(
                LOG_INFO,
                "Config::getUsbHostAssigment: assigning to midi bus: "
                "device=%s, midiBus=%d",
                productName,
                assigment.port);
            return (assigment.port);
        }
    }
    return (0);
}

const char *Config::translatePresetBankColour(uint32_t rgb888)
{
    if (rgb888 == 0xF45C51) {
        return ("F45C51");
    } else if (rgb888 == 0xF49500) {
        return ("F49500");
    } else if (rgb888 == 0x529DEC) {
        return ("529DEC");
    } else if (rgb888 == 0x03A598) {
        return ("03A598");
    } else if (rgb888 == 0xC44795) {
        return ("C44795");
    }
    return ("FFFFFF");
}
