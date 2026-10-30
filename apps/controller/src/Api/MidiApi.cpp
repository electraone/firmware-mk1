#include "MidiApi.h"
#include "System.h"

MidiApi::MidiApi(const MidiControls &newMidiControls, MainDelegate &newDelegate)
    : midiControls(newMidiControls),
      delegate(newDelegate),
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

bool MidiApi::processMidiControl(MidiMessage::Type type, uint8_t data1)
{
    for (const auto &midiControl : midiControls) {
        if (midiControl.midiMessageType == type) {
            if (midiControl.parameterNumber == data1) {
                delegate.closeAllWindows();
                uint8_t presetId = 0;
                switch (midiControl.eventType) {
                    case AppEventType::switchPreset:
                        presetId = midiControl.eventParameter1
                                   - 1; // zero based index is needed
                        delegate.switchPreset(presetId / 12, presetId % 12);
                        return (true);

                    case AppEventType::switchPage:
                        delegate.switchPage(midiControl.eventParameter1);
                        return (true);

                    case AppEventType::switchPageNext:
                        delegate.switchPageNext();
                        return (true);

                    case AppEventType::switchPagePrev:
                        delegate.switchPagePrev();
                        return (true);

                    case AppEventType::switchPresetNext:
                        delegate.switchPresetNext();
                        return (true);

                    case AppEventType::switchPresetPrev:
                        delegate.switchPresetPrev();
                        return (true);

                    default:
                        break;
                }
            }
        }
    }
    return (false);
}
