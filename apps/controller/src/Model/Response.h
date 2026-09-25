#pragma once

#include "Rule.h"
#include <cstdint>

class Response
{
public:
    Response() : id(0){};
    virtual ~Response() = default;

    void setId(uint8_t newId)
    {
        id = newId;
    }

    uint8_t getId(void) const
    {
        return (id);
    }

    std::vector<uint8_t> headers;
    Rules rules;

private:
    uint8_t id;
};
