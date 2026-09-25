#pragma once

#include "Selection.h"
#include "SnapshotsWindowDelegate.h"

class SnapshotBankSelection : public Selection
{
public:
    SnapshotBankSelection(SnapshotsWindowDelegate &newDelegate,
                          uint8_t newBankNumber,
                          uint32_t newColour,
                          uint32_t newActiveColour);
    virtual ~SnapshotBankSelection() = default;

private:
    void setActiveBank(uint8_t newBankNumber);

    SnapshotsWindowDelegate &delegate;
};
