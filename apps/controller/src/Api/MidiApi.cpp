#include "MidiApi.h"
#include "System.h"

MidiApi::MidiApi(const MidiControls &newMidiControls,
                 MainDelegate &newDelegate,
                 CommandManager &newCommandManager)
    : midiControls(newMidiControls),
      delegate(newDelegate),
      commandManager(newCommandManager),
      programChangeMode(Presets)
{
}

void MidiApi::process(const MidiMessage &midiMessage)
{
    if (!processMidiControl(midiMessage.getType(), midiMessage.getData1())) {
        if (midiMessage.isBankSelect()) {
            processBankSelect(midiMessage.getData2());
        } else if (midiMessage.isProgramChange()) {
            processProgramChange(midiMessage.getData1());
        }
    }
}

void MidiApi::processBankSelect(uint8_t bankNumber)
{
    // Bank numbers 1 .. 12 are for the snapshot banks
    if ((0 < bankNumber) && (bankNumber <= 12)) {
        System::logger.write(
            LOG_ERROR, "processBankSelect: switching to the snapshot mode");
        programChangeMode = Snapshots;
        delegate.setCurrentSnapshotBank(bankNumber - 1);
    } else if (bankNumber == 0) {
        System::logger.write(LOG_ERROR,
                             "processBankSelect: switching to the preset mode");
        programChangeMode = Presets;
    }
}

void MidiApi::processProgramChange(uint8_t programNumber)
{
    if (programChangeMode == Presets) {
        if (programNumber < 72) {
            uint8_t bankNumber = programNumber / 12;
            uint8_t slot = programNumber % 12;
            delegate.switchPreset(bankNumber, slot);
        }
    } else {
        if ((0 < programNumber) && (programNumber <= 36)) {
            uint8_t bankNumber = delegate.getCurrentSnapshotBank();
            const char *projectId = delegate.getCurrentProjectId();
            delegate.loadSnapshot(projectId, bankNumber, programNumber - 1);
        }
    }
}

/*
 * Any command the instrument has, not the handful of events this used to know:
 * the configuration names one and CommandManager decides what it means, which
 * is the same road a button on the panel and a host over SysEx take.
 */
bool MidiApi::processMidiControl(MidiMessage::Type type, uint8_t data1)
{
    for (const auto &midiControl : midiControls) {
        if ((midiControl.midiMessageType == type)
            && (midiControl.parameterNumber == data1)) {
            /* Whatever is open goes first, as it always has: a message from
             * off the instrument means the hands are somewhere else. The
             * window a command opens is opened after this. */
            delegate.closeAllWindows();

            return (commandManager.run(midiControl.command,
                                       midiControl.parameter));
        }
    }

    return (false);
}
