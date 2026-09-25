#pragma once

#include "Snapshot.h"
#include "LocalFile.h"

class Snapshots
{
public:
    explicit Snapshots(const char *newAppSandbox);
    virtual ~Snapshots() = default;

    bool initialise(const char *newProjectId);
    const char *getCurrentProjectId(void) const;
    void setDestProjectId(const char *newProjectId);
    const char *getDestProjectId(void) const;
    void setDestBankNumber(uint8_t newBankNumber);
    uint8_t getDestBankNumber(void) const;
    void setDestSlot(uint8_t newSlot);
    uint8_t getDestSlot(void) const;

    void sendList(uint8_t port, const char *projectId);
    void sendSnapshot(uint8_t port,
                      const char *projectId,
                      uint8_t bankNumber,
                      uint8_t slot);

    Snapshot importSnapshot(LocalFile file);
    void sendSnapshotMessages(const char *projectId,
                              uint8_t bankNumber,
                              uint8_t slot);
    void saveSnapshot(const char *projectId,
                      uint8_t bankNumber,
                      uint8_t slot,
                      const char *newName,
                      uint32_t newColour);
    void updateSnapshot(const char *projectId,
                        uint8_t bankNumber,
                        uint8_t slot,
                        const char *newName,
                        uint32_t newColour);
    void
        removeSnapshot(const char *projectId, uint8_t bankNumber, uint8_t slot);
    void swapSnapshot(const char *projectId,
                      uint8_t sourceBankNumber,
                      uint8_t sourceSlot,
                      uint8_t destBankNumber,
                      uint8_t destSlot);

    void createSnapshotFilename(char *buffer,
                                const char *projectId,
                                uint8_t bankNumber,
                                uint8_t slot);

private:
    bool createSnapshotDir(const char *projectId);
    bool createSnapshotDatabase(const char *projectId);
    void updateSnapshotDb(const char *projectId,
                          uint8_t bankNumber,
                          uint8_t slot,
                          const char *name,
                          uint32_t colour);
    void removeSnapshotDb(const char *projectId,
                          uint8_t bankNumber,
                          uint8_t slot);
    void swapSnapshotDb(const char *projectId,
                        uint8_t sourceBankNumber,
                        uint8_t sourceSlot,
                        uint8_t destBankNumber,
                        uint8_t destSlot);

    const char *appSandbox;
    char destProjectId[20 + 1];
    uint8_t destBankNumber;
    uint8_t destSlot;
};
