#pragma once

#include "App.h"
#include "MainWindow.h"
#include "MidiInputCallback.h"
#include "Model.h"
#include "MainDelegate.h"
#include "SysexApi.h"
#include "MidiApi.h"
#include "CommandManager.h"
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
          commandManager(mainWindow),
          sysexApi(mainWindow, commandManager),
          midiApi(appConfig.midiControls, mainWindow, commandManager),
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
        return ("5.0.6");
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
    void handleStagedFilesApplied(uint8_t port,
                                  bool currentSlotWritten) override;
    bool handleCtrlFileRemoved(uint8_t bankNumber,
                               uint8_t slot,
                               ElectraCommand::Object fileType) override;
    void handleElectraSysex(uint8_t port,
                            const SysexBlock &sysexBlock) override;

    // Patch requests
    void runUserTask(void);
    void runScheduled(void) override;

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

    /* Where every surface that can ask the instrument for something ends up:
     * the panel's buttons, the MIDI control assignments, a host over SysEx and
     * a script. */
    CommandManager commandManager;

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
