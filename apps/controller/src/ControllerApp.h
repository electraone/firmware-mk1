#pragma once

#include "App.h"
#include "MainWindow.h"
#include "MidiInputCallback.h"
#include "Model.h"
#include "MainDelegate.h"
#include "SysexApi.h"
#include "MidiApi.h"
#include "Config/Config.h"
#include "MidiLearn.h"
#include "MidiRouter.h"

class Controller : public App, private MidiInputCallback
{
public:
    Controller()
        : model(getApplicationSandbox(), appConfig),
          midi(model.currentPreset),
          mainWindow(MainWindow(model, midi, appConfig)),
          delegate(mainWindow),
          sysexApi(mainWindow),
          midiApi(appConfig.midiControls, mainWindow),
          midiLearn(model.currentPreset),
          midiRouter(appConfig.router)
    {
    }

    const char *getApplicationName(void) const override
    {
        return ("Controller");
    }

    const char *getApplicationVersion(void) const override
    {
        return ("3.5.4");
    }

    const char *getApplicationSandbox(void) const override
    {
        return ("ctrlv2");
    }

    void initialise(void) override;

private:
    void handleIncomingMidiMessage(const MidiInput &midiInput,
                                   const MidiMessage &midiMessage) override;
    bool handleCtrlFileReceived(uint8_t port,
                                LocalFile file,
                                ElectraCommand::Object fileType) override;
    bool handleCtrlFileRemoved(uint8_t bankNumber,
                               uint8_t slot,
                               ElectraCommand::Object fileType) override;
    void handleElectraSysex(uint8_t port,
                            const SysexBlock &sysexBlock) override;

    // Patch requests
    void runUserTask(void);

    // Config handling
    bool applyChangesToConfig(LocalFile file);
    bool loadConfig(void);
    void configureApp(void);
    uint8_t getUsbHostPortAssigment(const char *productName) override;

    // Model
    Model model;

    // Config
    Config appConfig;

    // UI
    MainWindow mainWindow;
    MainDelegate &delegate;

    // Interfaces
    Midi midi;
    SysexApi sysexApi;
    MidiApi midiApi;
    MidiLearn midiLearn;

    // Midi Router
    MidiRouter midiRouter;
};

// This macro instructs main() routine to launch the app.
START_ELECTRA_APPLICATION(Controller)
