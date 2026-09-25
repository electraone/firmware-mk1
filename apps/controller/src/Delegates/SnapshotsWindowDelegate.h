#pragma once

class SnapshotsWindowDelegate
{
public:
    virtual void openSnapshotBanks(void) = 0;
    virtual void closeSnapshotBanks(void) = 0;
    virtual void switchSnapshotBank(uint8_t newBankNumber) = 0;
    virtual void switchMode(void) = 0;

    virtual void snapshotRemoved(uint8_t bankNumber, uint8_t slot) = 0;
    virtual void snapshotSaved(uint8_t bankNumber,
                               uint8_t slot,
                               const char *name,
                               uint32_t colour) = 0;
    virtual void snapshotsSwapped(uint8_t sourceBankNumber,
                                  uint8_t sourceSlot,
                                  uint8_t destBankNumber,
                                  uint8_t destSlot) = 0;
    virtual void snapshotBankSwitched(uint8_t newBankNumber) = 0;
};
