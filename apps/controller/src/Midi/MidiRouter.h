#pragma once

#include "MidiInterface.h"
#include "MidiMessage.h"
#include "Config/Router.h"
#include "InstanceCallback.h"

typedef bool (*router_callback_t)(MidiInput &midiInput,
                                  MidiMessage &midiMessage);

typedef void (*router_sysex_callback_t)(MidiInput &midiInput,
                                        const uint8_t *sysExData,
                                        uint16_t sysExSize,
                                        bool complete);

class MidiRouter
{
public:
    explicit MidiRouter(const Router &newRouterConfig)
        : routerConfig(newRouterConfig)
    {
        InstanceCallback<bool(MidiInput & midiInput,
                              MidiMessage & midiMessage)>::callbackFunction =
            std::bind(&MidiRouter::routeMessage,
                      this,
                      std::placeholders::_1,
                      std::placeholders::_2);
        router_callback_t midiRouterCallback = static_cast<router_callback_t>(
            InstanceCallback<bool(MidiInput & midiInput,
                                  MidiMessage & midiMessage)>::callback);

        InstanceCallback<void(MidiInput & midiInput,
                              const uint8_t *sysExData,
                              uint16_t sysExSize,
                              bool complete)>::callbackFunction =
            std::bind(&MidiRouter::routePartialSysex,
                      this,
                      std::placeholders::_1,
                      std::placeholders::_2,
                      std::placeholders::_3,
                      std::placeholders::_4);
        router_sysex_callback_t midiRouterSysexCallback =
            static_cast<router_sysex_callback_t>(
                InstanceCallback<void(MidiInput & midiInput,
                                      const uint8_t *sysExData,
                                      uint16_t sysExSize,
                                      bool complete)>::callback);

        MidiInputCallback::routeMessageCallback = midiRouterCallback;
        MidiInputCallback::routePartialSysexCallback = midiRouterSysexCallback;
    }

    virtual ~MidiRouter() = default;

    bool routeMessage(MidiInput &midiInput, MidiMessage &midiMessage)
    {
        // Do not forward/process the CTRL messages. It is done on purpose for now.
        if (midiInput.getPort() == MIDI_CTRL_PORT) {
            System::logger.write(LOG_ERROR,
                                 "External MIDI control command arrived");
            return (true);
        }

        if (midiInput.getInterfaceType() == MidiInterface::Type::MidiUsbDev) {
            if (routerConfig.usbDevToMidiIo) {
                MidiOutput::send(MidiInterface::Type::MidiIo,
                                 midiInput.getPort(),
                                 midiMessage.getType(),
                                 midiMessage.getChannel(),
                                 midiMessage.getData1(),
                                 midiMessage.getData2());
            }
            if (routerConfig.usbDevToUsbHost) {
                MidiOutput::send(MidiInterface::Type::MidiUsbHost,
                                 midiInput.getPort(),
                                 midiMessage.getType(),
                                 midiMessage.getChannel(),
                                 midiMessage.getData1(),
                                 midiMessage.getData2());
            }
        }

        if (midiInput.getInterfaceType() == MidiInterface::Type::MidiIo) {
            if (routerConfig.midiIoToUsbDev) {
                MidiOutput::send(MidiInterface::Type::MidiUsbDev,
                                 midiInput.getPort(),
                                 midiMessage.getType(),
                                 midiMessage.getChannel(),
                                 midiMessage.getData1(),
                                 midiMessage.getData2());
            }
            if (routerConfig.midiIoToUsbHost) {
                MidiOutput::send(MidiInterface::Type::MidiUsbHost,
                                 midiInput.getPort(),
                                 midiMessage.getType(),
                                 midiMessage.getChannel(),
                                 midiMessage.getData1(),
                                 midiMessage.getData2());
            }
        }

        if (midiInput.getInterfaceType() == MidiInterface::Type::MidiUsbHost) {
            if (routerConfig.usbHostToMidiIo) {
                MidiOutput::send(MidiInterface::Type::MidiIo,
                                 midiInput.getPort(),
                                 midiMessage.getType(),
                                 midiMessage.getChannel(),
                                 midiMessage.getData1(),
                                 midiMessage.getData2());
            }
            if (routerConfig.usbHostToUsbDev) {
                MidiOutput::send(MidiInterface::Type::MidiUsbDev,
                                 midiInput.getPort(),
                                 midiMessage.getType(),
                                 midiMessage.getChannel(),
                                 midiMessage.getData1(),
                                 midiMessage.getData2());
            }
        }

        return (true);
    }

    void routePartialSysex(MidiInput &midiInput,
                           const uint8_t *sysExData,
                           uint16_t sysExSize,
                           bool complete)
    {
        if (midiInput.getInterfaceType() == MidiInterface::Type::MidiUsbDev) {
            if (routerConfig.usbDevToMidiIo) {
                MidiOutput::sendSysExPartial(MidiInterface::Type::MidiIo,
                                             midiInput.getPort(),
                                             sysExData,
                                             sysExSize,
                                             complete);
            }
            if (routerConfig.usbDevToUsbHost) {
                MidiOutput::sendSysExPartial(MidiInterface::Type::MidiUsbHost,
                                             midiInput.getPort(),
                                             sysExData,
                                             sysExSize,
                                             complete);
            }
        }

        if (midiInput.getInterfaceType() == MidiInterface::Type::MidiIo) {
            if (routerConfig.midiIoToUsbDev) {
                MidiOutput::sendSysExPartial(MidiInterface::Type::MidiUsbDev,
                                             midiInput.getPort(),
                                             sysExData,
                                             sysExSize,
                                             complete);
            }
            if (routerConfig.usbDevToUsbHost) {
                MidiOutput::sendSysExPartial(MidiInterface::Type::MidiUsbHost,
                                             midiInput.getPort(),
                                             sysExData,
                                             sysExSize,
                                             complete);
            }
        }

        if (midiInput.getInterfaceType() == MidiInterface::Type::MidiUsbHost) {
            if (routerConfig.usbHostToMidiIo) {
                MidiOutput::sendSysExPartial(MidiInterface::Type::MidiIo,
                                             midiInput.getPort(),
                                             sysExData,
                                             sysExSize,
                                             complete);
            }
            if (routerConfig.usbHostToUsbDev) {
                MidiOutput::sendSysExPartial(MidiInterface::Type::MidiUsbDev,
                                             midiInput.getPort(),
                                             sysExData,
                                             sysExSize,
                                             complete);
            }
        }
    }

private:
    const Router &routerConfig;
};
