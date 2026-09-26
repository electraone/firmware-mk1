#pragma once

// #include "Electra.h"
#include "Router.h"
#include "UsbHostAssigment.h"
#include "PresetBank.h"
#include "MidiControl.h"
#include "UiFeatures.h"
#include "HardwareControl.h"
#include <ArduinoJson.h>
#include <vector>
#include <array>

class Config
{
public:
    Config();

    bool load(void);
    bool load(const char *filename);
    void useDefault(void);
    void resetPresetBanks(void);
    void resetUiFeatures(void);
    void resetHardwareControls(void);
    uint8_t getUsbHostAssigment(const char *pattern);

    static constexpr uint8_t numPresetBanks = 6;

    Router router;
    std::vector<UsbHostAssigment> usbHostAssigments;
    std::vector<MidiControl> midiControls;
    std::array<PresetBank, numPresetBanks> presetBanks;
    UiFeatures uiFeatures;
    HardwareControl hardwareControl;

private:
    bool parse(File &file);
    bool parseRouter(File &file);
    bool parsePresetBanks(File &file);
    bool parseUsbHostAssigments(File &file);
    bool parseMidiControl(File &file);
    bool parseUiFeatures(File &file);
    bool parseHardwareControls(File &file);

    static void assignButton(ButtonAction &action,
                             const char *commandName,
                             int16_t parameter,
                             const char *buttonName,
                             const char *role);

    static const char *translatePresetBankColour(uint32_t rgb888);
};

typedef std::vector<UsbHostAssigment> UsbHostAssigments;
typedef std::vector<MidiControl> MidiControls;
typedef std::array<PresetBank, Config::numPresetBanks> PresetBanks;
