/*
* Electra One MIDI Controller Firmware
* See COPYRIGHT file at the top of the source tree.
*
* This product includes software developed by the
* Electra One Project (http://electra.one/).
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.
*/

/**
 * @file ElectraCommand.h
 *
 * @brief A definition of E1 Sysex Command and object bytes.
 */

#pragma once
#include "helpers.h"
#include <inttypes.h>
#include <stdint.h>

class ElectraCommand
{
public:
    enum class Type {
        FileUpload = 0x01, // File transfer
        FileRequest = 0x02, // (Fetch) Request in the application
        MidiLearnSwitch = 0x03,
        Update = 0x04,
        Remove = 0x05,
        Swap = 0x06,
        Create = 0x07,
        Execute = 0x08,
        Switch = 0x09,
        UpdateRuntime = 0x14,
        DataContainter = 0x7D,
        Event = 0x7E,
        SystemCall = 0x7F,
        Unknown = 0x00
    };

    enum class Object {
        MidiLearnOn = 0x00,
        MidiLearnOff = 0x01,
        FilePresetLegacy = 0x00,
        FilePreset = 0x01,
        FileConfig = 0x02,
        FileSnapshot = 0x03,
        PresetList = 0x04,
        SnapshotList = 0x05,
        SnapshotInfo = 0x06,
        Control = 0x07,
        PresetSlot = 0x08,
        SnapshotSlot = 0x09,
        Page = 0x0A,
        ControlSet = 0x0B,
        FileLua = 0x0C,
        Function = 0x0D,
        Value = 0x0E,
        UsbHostList = 0x10,
        Location = 0x34, /* a directory, by descriptor */
        File = 0x35, /* one file, by descriptor */
        Directory = 0x36, /* a directory to make or remove, by descriptor */
        AppState = 0x75, /* settings and configuration, as one thing */
        Screenshot = 0x76,
        StatusBar = 0x77,
        Reboot = 0x78,
        EventSubscription = 0x79,
        FileUi = 0x70,
        Window = 0x7A,
        ControlPort = 0x7B,
        AppInfo = 0x7C,
        Logger = 0x7D,
        RuntimeInfo = 0x7E,
        ElectraInfo = 0x7F,
        Unknown = 0x3F
    };

    enum class Event {
        Nack = 0x00,
        Ack = 0x01,
        PresetSwitch = 0x02,
        SnapshotChange = 0x03,
        SnapshotBankSwitch = 0x04,
        PresetListChange = 0x05,
        PageSwitch = 0x06,
        ControlSetSwitch = 0x07,
        PresetBankSwitch = 0x08,
        UsbHostChange = 0x09,
        Unknown = 0x7F
    };

    ElectraCommand()
        : type(Type::Unknown),
          object(Object::Unknown),
          event(Event::Unknown),
          byte1(0),
          byte2(0),
          byte3(0),
          byte4(0)
    {
    }

    Object translateObject(uint8_t objectByte)
    {
        if (objectByte == 0x00) {
            return (Object::FilePreset);
        } else if (objectByte == 0x01) {
            return (Object::FilePreset);
        } else if (objectByte == 0x02) {
            return (Object::FileConfig);
        } else if (objectByte == 0x03) {
            return (Object::FileSnapshot);
        } else if (objectByte == 0x04) {
            return (Object::PresetList);
        } else if (objectByte == 0x05) {
            return (Object::SnapshotList);
        } else if (objectByte == 0x06) {
            return (Object::SnapshotInfo);
        } else if (objectByte == 0x07) {
            return (Object::Control);
        } else if (objectByte == 0x08) {
            return (Object::PresetSlot);
        } else if (objectByte == 0x09) {
            return (Object::SnapshotSlot);
        } else if (objectByte == 0x0A) {
            return (Object::Page);
        } else if (objectByte == 0x0B) {
            return (Object::ControlSet);
        } else if (objectByte == 0x0C) {
            return (Object::FileLua);
        } else if (objectByte == 0x0D) {
            return (Object::Function);
        } else if (objectByte == 0x0E) {
            return (Object::Value);
        } else if (objectByte == 0x10) {
            return (Object::UsbHostList);
        } else if (objectByte == 0x70) {
            return (Object::FileUi);
        } else if (objectByte == 0x34) {
            return (Object::Location);
        } else if (objectByte == 0x35) {
            return (Object::File);
        } else if (objectByte == 0x36) {
            return (Object::Directory);
        } else if (objectByte == 0x75) {
            return (Object::AppState);
        } else if (objectByte == 0x76) {
            return (Object::Screenshot);
        } else if (objectByte == 0x77) {
            return (Object::StatusBar);
        } else if (objectByte == 0x78) {
            return (Object::Reboot);
        } else if (objectByte == 0x79) {
            return (Object::EventSubscription);
        } else if (objectByte == 0x7A) {
            return (Object::Window);
        } else if (objectByte == 0x7B) {
            return (Object::ControlPort);
        } else if (objectByte == 0x7C) {
            return (Object::AppInfo);
        } else if (objectByte == 0x7D) {
            return (Object::Logger);
        } else if (objectByte == 0x7E) {
            return (Object::RuntimeInfo);
        } else if (objectByte == 0x7F) {
            return (Object::ElectraInfo);
        } else {
            return (Object::Unknown);
        }
    }

    Event translateEvent(uint8_t eventByte)
    {
        if (eventByte == 0x00) {
            return (Event::Nack);
        } else if (eventByte == 0x01) {
            return (Event::Ack);
        } else if (eventByte == 0x02) {
            return (Event::PresetSwitch);
        } else if (eventByte == 0x03) {
            return (Event::SnapshotChange);
        } else if (eventByte == 0x04) {
            return (Event::SnapshotBankSwitch);
        } else if (eventByte == 0x05) {
            return (Event::PresetListChange);
        } else if (eventByte == 0x06) {
            return (Event::PageSwitch);
        } else if (eventByte == 0x07) {
            return (Event::ControlSetSwitch);
        } else if (eventByte == 0x08) {
            return (Event::PresetBankSwitch);
        } else if (eventByte == 0x09) {
            return (Event::UsbHostChange);
        } else {
            return (Event::Unknown);
        }
    }

    Type translateType(uint8_t commandByte)
    {
        if (commandByte == 0x01) {
            return (Type::FileUpload);
        } else if (commandByte == 0x02) {
            return (Type::FileRequest);
        } else if (commandByte == 0x03) {
            return (Type::MidiLearnSwitch);
        } else if (commandByte == 0x04) {
            return (Type::Update);
        } else if (commandByte == 0x05) {
            return (Type::Remove);
        } else if (commandByte == 0x06) {
            return (Type::Swap);
        } else if (commandByte == 0x07) {
            return (Type::Create);
        } else if (commandByte == 0x08) {
            return (Type::Execute);
        } else if (commandByte == 0x09) {
            return (Type::Switch);
        } else if (commandByte == 0x14) {
            return (Type::UpdateRuntime);
        } else if (commandByte == 0x7D) {
            return (Type::DataContainter);
        } else if (commandByte == 0x7E) {
            return (Type::Event);
        } else if (commandByte == 0x7F) {
            return (Type::SystemCall);
        } else {
            return (Type::Unknown);
        }
    }

    void set(uint8_t commandByte,
             uint8_t objectByte,
             uint8_t newByte1,
             uint8_t newByte2,
             uint8_t newByte3,
             uint8_t newByte4)
    {
        type = translateType(commandByte);
        if (type
            == Type::MidiLearnSwitch) // TODO: ugly fix because the MIDI learn
        // sysx is not ok
        {
            if (objectByte == 1) {
                object = Object::MidiLearnOn;
            } else {
                object = Object::MidiLearnOff;
            }
        } else {
            object = translateObject(objectByte);
        }
        event = translateEvent(objectByte);

        byte1 = newByte1;
        byte2 = newByte2;
        byte3 = newByte3;
        byte4 = newByte4;
    }

    bool isFileUpload(void)
    {
        return (type == Type::FileUpload);
    }

    bool isFileRequest(void)
    {
        return (type == Type::FileRequest);
    }

    bool isMidiLearnSwitch(void)
    {
        return (type == Type::MidiLearnSwitch);
    }

    bool isUpdate(void)
    {
        return (type == Type::Update);
    }

    bool isRemove(void)
    {
        return (type == Type::Remove);
    }

    bool isSwap(void)
    {
        return (type == Type::Swap);
    }

    bool isCreate(void)
    {
        return (type == Type::Create);
    }

    bool isExecute(void)
    {
        return (type == Type::Execute);
    }

    bool isSwitch(void)
    {
        return (type == Type::Switch);
    }

    bool isUpdateRuntime(void)
    {
        return (type == Type::UpdateRuntime);
    }

    bool isDataContainer(void)
    {
        return (type == Type::DataContainter);
    }

    bool isEvent(void)
    {
        return (type == Type::Event);
    }

    bool isSystemCall(void)
    {
        return (type == Type::SystemCall);
    }

    Object getObject(void)
    {
        return (object);
    }

    uint8_t getByte1(void)
    {
        return (byte1);
    }

    uint8_t getByte2(void)
    {
        return (byte2);
    }

    uint8_t getByte3(void)
    {
        return (byte3);
    }

    uint8_t getByte4(void)
    {
        return (byte4);
    }

    Type getType(void)
    {
        return (type);
    }

    Event getEvent(void)
    {
        return (event);
    }

private:
    Type type;
    Object object;
    Event event;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    uint8_t byte4;
};
