#pragma once

#include <cstdint>

class Input
{
public:
    Input() : valueId(0), potId(0)
    {
    }

    Input(uint8_t newValueId, uint8_t newPotId)
        : valueId(newValueId), potId(newPotId)
    {
    }

    virtual ~Input() = default;

    void setPotId(uint8_t newPotId)
    {
        potId = newPotId;
    }

    uint8_t getPotId(void) const
    {
        return (potId);
    }

    uint8_t getValueId(void) const
    {
        return (valueId);
    }

    void print(uint8_t logLevel = LOG_TRACE) const
    {
        System::logger.write(
            logLevel, "        --[Input]------------------------------------");
        System::logger.write(logLevel, "        address: 0x%08x", this);
        System::logger.write(logLevel, "        potId: %d", getPotId());
        System::logger.write(logLevel, "        valueId: %d", getValueId());
    }

private:
    struct {
        uint8_t valueId : 4;
        uint8_t potId : 4;
    };
};
