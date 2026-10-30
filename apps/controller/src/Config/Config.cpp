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
    resetHardwareControls();
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
    parseHardwareControls(file);

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

            if (parameterNumber > 127) {
                parameterNumber = 0;
            }

            /*
             * Two spellings, because both have been written. The older one
             * names the command in "event" with its number in
             * "eventParameter"; the newer one puts both in a "command" object,
             * where the number is called after what it meant for the one
             * command that used it. Either way what comes out is a command and
             * a number.
             */
            const char *commandName = nullptr;
            int16_t parameter = 0;

            if (jMidiControl["command"]) {
                JsonObject jCommand = jMidiControl["command"];

                commandName = jCommand["type"].as<char *>();

                if (jCommand["parameter"].is<int>()) {
                    parameter = jCommand["parameter"].as<int16_t>();
                } else if (jCommand["pageId"].is<int>()) {
                    parameter = jCommand["pageId"].as<int16_t>();
                } else if (jCommand["controlSetId"].is<int>()) {
                    parameter = jCommand["controlSetId"].as<int16_t>();
                }
            } else {
                commandName = jMidiControl["event"].as<char *>();
                parameter = jMidiControl["eventParameter"].as<int16_t>();
            }

            UiCommand::Type command = UiCommand::translate(commandName);
            MidiMessage::Type midiMessageType =
                MidiMessage::translateType(midiMessage);

            if (command == UiCommand::Type::None) {
                System::logger.write(
                    LOG_WARNING,
                    "Config::parseMidiControl: no such command: %s",
                    commandName ? commandName : "(none)");
            } else {
                midiControls.push_back(MidiControl(
                    command, parameter, midiMessageType, parameterNumber));

                System::logger.write(
                    LOG_TRACE,
                    "Config::parseMidiControl: midi control assigment: "
                    "command=%s, parameter=%d, midiMessage=%s (%d), "
                    "parameterNumber=%d",
                    UiCommand::translateToText(command),
                    parameter,
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

        /* The mode where the configuration names one. A configuration written
         * for an older firmware says it with a switch instead, and one saying
         * neither stays on the default. As the mk2 reads it. */
        if (doc["uiFeatures"]["activeControlSetMode"].is<const char *>()) {
            uiFeatures.activeControlSetMode = translateControlSetMode(
                doc["uiFeatures"]["activeControlSetMode"].as<const char *>());
        } else if (doc["uiFeatures"]["resetActiveControlSet"].is<bool>()) {
            uiFeatures.activeControlSetMode =
                doc["uiFeatures"]["resetActiveControlSet"].as<bool>()
                    ? ActiveControlSetMode::custom
                    : ActiveControlSetMode::global;
        }
        System::logger.write(LOG_TRACE,
                             "Config::parseUiFeatures: activeControlSetMode=%d",
                             uiFeatures.activeControlSetMode);
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

/*
 * What the six buttons on the panel do:
 *
 *     "hardware": {
 *         "buttons": {
 *             "leftTop":     { "primary": "openPresetMenu" },
 *             "rightBottom": { "primary": "switchPage", "parameter": 3,
 *                              "knobTouch": "openDetail" }
 *         }
 *     }
 *
 * "primary" is what the button does on its own and "knobTouch" what it does
 * while a finger is on a knob, which is the mk2's spelling. A button the file
 * does not name keeps what it has always done, and so does a button whose
 * command is not one this instrument has.
 */
bool Config::parseHardwareControls(File &file)
{
    const size_t capacityHardware = JSON_OBJECT_SIZE(1) + 2000;
    const size_t capacityFilter = JSON_OBJECT_SIZE(1) + 100;
    StaticJsonDocument<capacityHardware> doc;
    StaticJsonDocument<capacityFilter> filter;

    resetHardwareControls();

    filter["hardware"] = true;

    if (file.seek(0) == false) {
        System::logger.write(
            LOG_ERROR, "Config::parseHardwareControls: cannot rewind the file");
        return (false);
    }

    DeserializationError err =
        deserializeJson(doc, file, DeserializationOption::Filter(filter));

    if (err) {
        System::logger.write(LOG_ERROR,
                             "Config::parseHardwareControls: parsing failed: %s",
                             err.c_str());
        return (false);
    }

    JsonObject buttons = doc["hardware"]["buttons"];

    if (!buttons) {
        System::logger.write(
            LOG_INFO,
            "Config::parseHardwareControls: no button assignments");
        return (true);
    }

    for (JsonPair keyValue : buttons) {
        const char *buttonName = keyValue.key().c_str();
        const uint8_t buttonId =
            HardwareControl::translateButtonName(buttonName);

        if (buttonId >= NR_OF_HW_BUTTONS) {
            System::logger.write(
                LOG_WARNING,
                "Config::parseHardwareControls: no such button: %s",
                buttonName);
            continue;
        }

        JsonObject jButton = keyValue.value().as<JsonObject>();

        if (!jButton) {
            continue;
        }

        /* One number for the button, whichever of its two commands wants one.
         * A button that runs two commands needing different numbers is not
         * something anyone has asked for, and a second field for it would be
         * a field to explain. */
        const int16_t parameter = jButton["parameter"] | (int16_t)0;

        assignButton(hardwareControl.button[buttonId],
                     jButton["primary"],
                     parameter,
                     buttonName,
                     "primary");
        assignButton(hardwareControl.knobTouch[buttonId],
                     jButton["knobTouch"],
                     parameter,
                     buttonName,
                     "knobTouch");
    }

    return (true);
}

void Config::assignButton(ButtonAction &action,
                          const char *commandName,
                          int16_t parameter,
                          const char *buttonName,
                          const char *role)
{
    if (!commandName) {
        return;
    }

    const UiCommand::Type command = UiCommand::translate(commandName);

    if (command == UiCommand::Type::None) {
        System::logger.write(LOG_WARNING,
                             "Config::parseHardwareControls: %s %s: no such "
                             "command: %s",
                             buttonName,
                             role,
                             commandName);
        return;
    }

    action.command = command;
    action.parameter = parameter;

    System::logger.write(LOG_INFO,
                         "Config::parseHardwareControls: %s %s = %s (%d)",
                         buttonName,
                         role,
                         commandName,
                         parameter);
}

void Config::resetUiFeatures(void)
{
    uiFeatures.touchSwitchControlSets = true;
    uiFeatures.activeControlSetMode = ActiveControlSetMode::global;
    uiFeatures.activeControlSetType = ActiveControlSetType::dim;
    uiFeatures.keepPresetState = false;
    uiFeatures.loadPresetStateOnStartup = false;
}

void Config::resetHardwareControls(void)
{
    hardwareControl.reset();
}

void Config::useDefault(void)
{
    resetPresetBanks();
    resetUiFeatures();
    resetHardwareControls();
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
