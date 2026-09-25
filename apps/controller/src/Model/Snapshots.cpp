#include "Snapshots.h"
#include "Database.h"
#include "ParameterMap.h"
#include "MidiOutput.h"

Snapshots::Snapshots(const char *newAppSandbox)
    : appSandbox(newAppSandbox), destBankNumber(0), destSlot(0)
{
    copyString(destProjectId, "noProjectId", 20);
}

bool Snapshots::initialise(const char *newProjectId)
{
    System::sysExBusy = true;
    if (!createSnapshotDir(newProjectId)) {
        System::logger.write(
            LOG_ERROR,
            "Snapshots::initialise: cannot create snapshot directories");
        System::sysExBusy = false;
        return (false);
    }

    if (!createSnapshotDatabase(newProjectId)) {
        System::logger.write(
            LOG_ERROR,
            "Snapshots::initialise: cannot create snapshot database");
        System::sysExBusy = false;
        return (false);
    }
    System::sysExBusy = false;

    return (true);
}

void Snapshots::setDestProjectId(const char *newProjectId)
{
    copyString(destProjectId, newProjectId, 20);
}

const char *Snapshots::getDestProjectId(void) const
{
    return (destProjectId);
}

void Snapshots::setDestBankNumber(uint8_t newBankNumber)
{
    destBankNumber = newBankNumber;
}

uint8_t Snapshots::getDestBankNumber(void) const
{
    return (destBankNumber);
}

void Snapshots::setDestSlot(uint8_t newSlot)
{
    destSlot = newSlot;
}

uint8_t Snapshots::getDestSlot(void) const
{
    return (destSlot);
}

void Snapshots::sendList(uint8_t port, const char *projectId)
{
    char dbFile[MAX_FILENAME_LENGTH + 1];
    char tempSnapshotFilename[MAX_FILENAME_LENGTH + 1];

    System::sysExBusy = true;

    snprintf(dbFile,
             MAX_FILENAME_LENGTH,
             "%s/snaps/%s/snapshot.db",
             appSandbox,
             projectId);

    snprintf(tempSnapshotFilename,
             MAX_FILENAME_LENGTH,
             "%s/%08ld.tmp",
             appSandbox,
             millis());

    Database dbSnapshot(dbFile);

    if (!dbSnapshot.open()) {
        System::logger.write(
            LOG_ERROR, "Snapshots::sendList: cannot open the snapshot storage");
        System::sysExBusy = false;
        return;
    }

    File snapshotJsonFile = Hardware::sdcard.createOutputStream(
        tempSnapshotFilename, FILE_WRITE | O_CREAT | O_TRUNC);

    if (!snapshotJsonFile) {
        System::logger.write(LOG_ERROR,
                             "Snapshots::sendList: cannot open transfer file");
        dbSnapshot.close();
        System::sysExBusy = false;
        return;
    }

    SnapshotRecord snapRec;
    bool firstRecord = true;
    char buf[256]; // \todo fix buffer overflow issue

    buf[0] = 0xf0;
    buf[1] = 0x00;
    buf[2] = 0x21;
    buf[3] = 0x45;
    buf[4] = 0x01;
    buf[5] = (char)ElectraCommand::Object::SnapshotList;

    sprintf(buf + 6,
            "{\"version\":1,\"projectId\":\"%s\",\"snapshots\":[",
            projectId);

    MidiOutput::sendSysExPartial(MidiInterface::Type::MidiUsbDev,
                                 port,
                                 (uint8_t *)buf,
                                 strlen(buf + 6) + 6,
                                 false);

    for (uint16_t i = 0; i < dbSnapshot.getNumRecords(); i++) {
        if (dbSnapshot.select(i, DB_RECORD snapRec)) {
            sprintf(
                buf,
                "%s{\"slot\":%d,\"bankNumber\":%d,\"name\":\"%s\",\"color\":\"%06X\"}",
                (firstRecord) ? "" : ",",
                snapRec.slot,
                snapRec.bankNumber,
                snapRec.name,
                snapRec.colour & 0xffffff);
            firstRecord = false;
            MidiOutput::sendSysExPartial(MidiInterface::Type::MidiUsbDev,
                                         port,
                                         (uint8_t *)buf,
                                         strlen(buf),
                                         false);
        }
    }

    sprintf(buf, "]}");
    buf[2] = 0xf7;

    MidiOutput::sendSysExPartial(
        MidiInterface::Type::MidiUsbDev, port, (uint8_t *)buf, 3, false);

    System::sysExBusy = false;
}

void Snapshots::sendSnapshot(uint8_t port,
                             const char *projectId,
                             uint8_t bankNumber,
                             uint8_t slot)
{
    char snapshotFilename[MAX_FILENAME_LENGTH + 1];

    System::sysExBusy = true;
    createSnapshotFilename(snapshotFilename, projectId, bankNumber, slot);

    if (Hardware::sdcard.exists(snapshotFilename)) {
        MidiOutput::sendSysExFile(
            port, snapshotFilename, ElectraCommand::Object::FileSnapshot);
    }
    System::sysExBusy = false;
}

Snapshot Snapshots::importSnapshot(LocalFile file)
{
    char snapshotFilename[MAX_FILENAME_LENGTH + 1];
    createSnapshotFilename(
        snapshotFilename, destProjectId, destBankNumber, destSlot);

    Snapshot snapshot(file.getFilepath());

    System::logger.write(
        LOG_ERROR,
        "importSnapshot: file=%s, projectId=%s, bankNumber=%d, slot=%d, name=%s, colour=%x",
        snapshotFilename,
        destProjectId,
        destBankNumber,
        destSlot,
        snapshot.getName(),
        snapshot.getColour());

    System::sysExBusy = true;
    if (!Hardware::sdcard.exists(snapshotFilename)) {
        if (file.rename(snapshotFilename)) {
            updateSnapshotDb(destProjectId,
                             destBankNumber,
                             destSlot,
                             snapshot.getName(),
                             snapshot.getColour());
        }
    }
    System::sysExBusy = false;
    return (snapshot);
}

void Snapshots::sendSnapshotMessages(const char *projectId,
                                     uint8_t bankNumber,
                                     uint8_t slot)
{
    char filename[MAX_FILENAME_LENGTH + 1];
    createSnapshotFilename(filename, projectId, bankNumber, slot);
    System::sysExBusy = true;
    if (Hardware::sdcard.exists(filename)) {
        parameterMap.load(filename);
    }
    System::sysExBusy = false;
}

void Snapshots::saveSnapshot(const char *projectId,
                             uint8_t bankNumber,
                             uint8_t slot,
                             const char *newName,
                             uint32_t newColour)
{
    char filename[MAX_FILENAME_LENGTH + 1];
    createSnapshotFilename(filename, projectId, bankNumber, slot);
    System::sysExBusy = true;
    parameterMap.save(filename);
    updateSnapshotDb(projectId, bankNumber, slot, newName, newColour);
    System::sysExBusy = false;
}

void Snapshots::updateSnapshot(const char *projectId,
                               uint8_t bankNumber,
                               uint8_t slot,
                               const char *newName,
                               uint32_t newColour)
{
    System::sysExBusy = true;
    updateSnapshotDb(projectId, bankNumber, slot, newName, newColour);
    System::sysExBusy = false;
}

void Snapshots::removeSnapshot(const char *projectId,
                               uint8_t bankNumber,
                               uint8_t slot)
{
    System::sysExBusy = true;
    removeSnapshotDb(projectId, bankNumber, slot);
    System::sysExBusy = false;
}

void Snapshots::swapSnapshot(const char *projectId,
                             uint8_t sourceBankNumber,
                             uint8_t sourceSlot,
                             uint8_t destBankNumber,
                             uint8_t destSlot)
{
    System::sysExBusy = true;
    swapSnapshotDb(
        projectId, sourceBankNumber, sourceSlot, destBankNumber, destSlot);
    System::sysExBusy = false;
}

void Snapshots::updateSnapshotDb(const char *projectId,
                                 uint8_t bankNumber,
                                 uint8_t slot,
                                 const char *name,
                                 uint32_t colour)
{
    char dbFile[MAX_FILENAME_LENGTH + 1];

    snprintf(dbFile,
             MAX_FILENAME_LENGTH,
             "%s/snaps/%s/snapshot.db",
             appSandbox,
             projectId);
    Database dbSnapshot(dbFile);

    if (!dbSnapshot.open()) {
        System::logger.write(LOG_ERROR,
                             "updateSnapshotDb: cannot open database: file=%s",
                             dbFile);
        return;
    }

    uint16_t id = (bankNumber * 36) + slot;

    SnapshotRecord snapRec;

    snapRec.slot = slot;
    snapRec.bankNumber = bankNumber;
    snapRec.colour = colour;
    copyString(snapRec.name, name, MAX_SNAPSHOT_NAME_LENGTH);

    dbSnapshot.update(id, DB_RECORD snapRec);
    dbSnapshot.close();
}

void Snapshots::removeSnapshotDb(const char *projectId,
                                 uint8_t bankNumber,
                                 uint8_t slot)
{
    bool status;
    char snapshotFilename[MAX_FILENAME_LENGTH + 1];

    snprintf(snapshotFilename,
             MAX_FILENAME_LENGTH,
             "%s/snaps/%s/s%02d%02d.snp",
             appSandbox,
             projectId,
             bankNumber,
             slot);
    if (Hardware::sdcard.exists(snapshotFilename)) {
        status = Hardware::sdcard.deleteFile(snapshotFilename);
        System::logger.write(LOG_ERROR,
                             "removing file %s: %s",
                             snapshotFilename,
                             (status == true) ? "OK" : "fail");
    }

    uint16_t id = (bankNumber * 36) + slot;

    // update database on success
    snprintf(snapshotFilename,
             MAX_FILENAME_LENGTH,
             "%s/snaps/%s/snapshot.db",
             appSandbox,
             projectId);
    Database dbSnapshot(snapshotFilename); // x

    dbSnapshot.open();
    System::logger.write(
        LOG_ERROR,
        "ElectraApp::removeSnapshotDb: removing projectId=%s, bankNumber=%d, slot=%d, id=%d",
        projectId,
        bankNumber,
        slot,
        id);
    dbSnapshot.remove(id);
    dbSnapshot.close();
}

void Snapshots::swapSnapshotDb(const char *projectId,
                               uint8_t sourceBankNumber,
                               uint8_t sourceSlot,
                               uint8_t destBankNumber,
                               uint8_t destSlot)
{
    bool status;

    char sourceFilename[MAX_FILENAME_LENGTH + 1];
    char tmpFilename[MAX_FILENAME_LENGTH + 1];
    char destFilename[MAX_FILENAME_LENGTH + 1];

    createSnapshotFilename(
        sourceFilename, projectId, sourceBankNumber, sourceSlot);
    snprintf(tmpFilename,
             MAX_FILENAME_LENGTH,
             "%s/snaps/%s/s%02d%02d.tmp",
             appSandbox,
             projectId,
             sourceBankNumber,
             sourceSlot);
    createSnapshotFilename(destFilename, projectId, destBankNumber, destSlot);

    if (Hardware::sdcard.exists(tmpFilename)) {
        status = Hardware::sdcard.deleteFile(tmpFilename);
        System::logger.write(LOG_ERROR,
                             "swapSnapshotDb: removing tmp file %s: %s",
                             tmpFilename,
                             (status == true) ? "OK" : "fail");
    }

    if (!Hardware::sdcard.renameFile(sourceFilename, tmpFilename)) {
        System::logger.write(LOG_ERROR,
                             "swapSnapshotDb: cannot rename %s to %s",
                             sourceFilename,
                             tmpFilename);
    }
    System::logger.write(LOG_ERROR,
                         "swapSnapshotDb: move %s to %s",
                         sourceFilename,
                         tmpFilename);

    if (Hardware::sdcard.exists(destFilename)) {
        if (!Hardware::sdcard.renameFile(destFilename, sourceFilename)) {
            System::logger.write(LOG_ERROR,
                                 "cannot rename %s to %s",
                                 destFilename,
                                 sourceFilename);
        }
        System::logger.write(LOG_ERROR,
                             "swapSnapshotDb: move %s to %s",
                             destFilename,
                             sourceFilename);
    }

    if (!Hardware::sdcard.renameFile(tmpFilename, destFilename)) {
        System::logger.write(LOG_ERROR,
                             "swapSnapshotDb: cannot rename %s to %s",
                             tmpFilename,
                             destFilename);
    }
    System::logger.write(
        LOG_ERROR, "swapSnapshotDb: move %s to %s", tmpFilename, destFilename);

    // update database on success
    SnapshotRecord sourceRec;
    SnapshotRecord destRec;

    uint16_t sourceId = (sourceBankNumber * 36) + sourceSlot;
    uint16_t destId = (destBankNumber * 36) + destSlot;

    char snapshotFilename[MAX_FILENAME_LENGTH + 1];
    snprintf(snapshotFilename,
             MAX_FILENAME_LENGTH,
             "%s/snaps/%s/snapshot.db",
             appSandbox,
             projectId);
    Database dbSnapshot(snapshotFilename); // x

    dbSnapshot.open();

    dbSnapshot.select(sourceId, DB_RECORD sourceRec);
    bool destUsed = dbSnapshot.select(destId, DB_RECORD destRec);

    if (destUsed) {
        uint8_t tmpBankNumber = destRec.bankNumber;
        uint8_t tmpSlot = destRec.slot;

        //uint8_t tmpColour = destRec.colour;
        //char tmpName[MAX_SNAPSHOT_NAME_LENGTH + 1];
        //copyString (tmpName, destRec.name, MAX_SNAPSHOT_NAME_LENGTH);

        destRec.bankNumber = sourceRec.bankNumber;
        destRec.slot = sourceRec.slot;
        //destRec.colour = sourceRec.colour;
        //copyString (destRec.name, sourceRec.name, MAX_SNAPSHOT_NAME_LENGTH);

        sourceRec.bankNumber = tmpBankNumber;
        sourceRec.slot = tmpSlot;
        //sourceRec.colour = tmpColour;
        //copyString (sourceRec.name, tmpName, MAX_SNAPSHOT_NAME_LENGTH);

        dbSnapshot.update(sourceId, DB_RECORD destRec);
        dbSnapshot.update(destId, DB_RECORD sourceRec);
    } else {
        dbSnapshot.remove(sourceId);
        sourceRec.bankNumber = destBankNumber;
        sourceRec.slot = destSlot;
        dbSnapshot.update(destId, DB_RECORD sourceRec);
    }

    System::logger.write(
        LOG_ERROR,
        "swapSnapshotDb: swapping snaphosts: projectId=%s, sourceId=%d, sourceBankNumber=%d, sourceSlot=%d, destId=%d, destBankNumber=%d, destSlot=%d",
        projectId,
        sourceId,
        sourceBankNumber,
        sourceSlot,
        destId,
        destBankNumber,
        destSlot);
    dbSnapshot.close();
}

bool Snapshots::createSnapshotDir(const char *projectId)
{
    char dirname[MAX_DIRNAME_LENGTH + 1];
    snprintf(dirname, MAX_DIRNAME_LENGTH, "%s/snaps", appSandbox);

    if (!Hardware::sdcard.exists(dirname)) {
        if (!Hardware::sdcard.createDirectory(dirname)) {
            System::logger.write(
                LOG_ERROR,
                "Snapshots::createSnapshotDir: cannot create snaps directory: %s",
                dirname);
        }
        return (false);
    }

    snprintf(dirname, MAX_DIRNAME_LENGTH, "%s/snaps/%s", appSandbox, projectId);

    if (!Hardware::sdcard.exists(dirname)) {
        if (!Hardware::sdcard.createDirectory(dirname)) {
            System::logger.write(
                LOG_ERROR,
                "Snapshots::createSnapshotDir: cannot create preset snaphot storage");
            return (false);
        } else {
            System::logger.write(
                LOG_ERROR,
                "Snapshots::createSnapshotDir: directory created: %s",
                dirname);
        }
    }

    return (true);
}

bool Snapshots::createSnapshotDatabase(const char *projectId)
{
    char snapshotDbFilename[MAX_FILENAME_LENGTH + 1];
    snprintf(snapshotDbFilename,
             MAX_FILENAME_LENGTH,
             "%s/snaps/%s/snapshot.db",
             appSandbox,
             projectId);
    Database dbSnapshot(snapshotDbFilename);

    if (!Hardware::sdcard.exists(snapshotDbFilename)) {
        System::logger.write(
            LOG_ERROR,
            "ElectraApp::createSnapshotDatabase: creating db file %s",
            snapshotDbFilename);
        if (!dbSnapshot.create(432, sizeof(SnapshotRecord))) {
            System::logger.write(LOG_ERROR,
                                 "Snapshots::createSnapshotDatabase: "
                                 "failed to initialize snapshot database: %s",
                                 snapshotDbFilename);
            return (false);
        }
    }
    return (true);
}

void Snapshots::createSnapshotFilename(char *buffer,
                                       const char *newProjectId,
                                       uint8_t newBankNumber,
                                       uint8_t newSlot)
{
    snprintf(buffer,
             MAX_FILENAME_LENGTH,
             "%s/snaps/%s/s%02d%02d.snp",
             appSandbox,
             newProjectId,
             newBankNumber,
             newSlot);
}
