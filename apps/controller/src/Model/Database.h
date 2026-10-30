#pragma once

#include "System.h"
#include "Hardware.h"

#define MAX_SNAPSHOT_NAME_LENGTH 14

struct SnapshotRecord {
    uint16_t slot;
    uint8_t bankNumber;
    uint32_t colour;
    char name[MAX_SNAPSHOT_NAME_LENGTH + 1];
};

#define DB_RECORD (byte *)(void *)&

class Database
{
public:
    static constexpr uint8_t maxDatabaseFilenameLength = 48;

    Database(const char *newFilename)
        : dataOffset(sizeof(Header)), isOpen(false)
    {
        copyString(filename, newFilename, maxDatabaseFilenameLength);

#if DEBUG
        System::logger.write(
            LOG_ERROR, "Database::constructor: file=%s", filename);
#endif

        if (!Hardware::sdcard.exists(filename)) {
            System::logger.write(
                LOG_ERROR,
                "Database::open Database file does not exist: file='%s'",
                filename);
        }
    }

    ~Database()
    {
#if DEBUG
        System::logger.write(
            LOG_ERROR, "Database::destructor: file=%s", filename);
#endif

        if (file) {
            file.close();
        }
    }

    bool create(uint16_t numRecords, uint16_t recordSize)
    {
        header.numRecords = numRecords;
        header.recordSize = recordSize;

        if (!Hardware::sdcard.deleteFile(filename)) {
            System::logger.write(
                LOG_ERROR, "Database::create: cannot remove dangling file");
        }

        /*
         * open database file
         */
        file = Hardware::sdcard.createOutputStream(
            filename, FILE_WRITE | O_CREAT | O_TRUNC);

        if (!file) {
            System::logger.write(
                LOG_ERROR,
                "Database::create: Cannot open the file for writing: file=%s",
                filename);
            return (false);
        } else {
            isOpen = true;
        }

        /*
         * write header
         */
        if (!writeHeader()) {
            System::logger.write(
                LOG_ERROR,
                "Database::create: write header failed: file=%s",
                filename);
            close();
            return (false);
        }

        size_t dbFileSize = numRecords * (recordSize + 1) + sizeof(Header);

        for (size_t i = 0; i < dbFileSize; i++) {
            file.write(0);
        }

        close();

        return (true);
    }

    bool open(void)
    {
        if (isOpen) {
            System::logger.write(
                LOG_ERROR, "Database::open: Database: it was open already");
            return (false);
        }

        file = Hardware::sdcard.createOutputStream(filename, FILE_WRITE);

        if (!file) {
            System::logger.write(
                LOG_ERROR,
                "Database::open: Cannot open the file for reading/writing: file='%s'",
                filename);
            return (false);
        } else if (file.size() == 0) {
            System::logger.write(LOG_ERROR,
                                 "Database::open: File is broken: file='%s'",
                                 filename);
            return (false);
        } else {
            isOpen = true;
        }

        if (!readHeader()) {
            System::logger.write(LOG_ERROR,
                                 "Database::open Cannot read header: file=%s",
                                 filename);
        }

#if DEBUG
        System::logger.write(LOG_ERROR, "Database::open: file=%s", filename);
#endif

        return (true);
    }

    uint16_t getNumRecords(void)
    {
        return (header.numRecords);
    }

    uint16_t getRecordSize(void)
    {
        return (header.recordSize);
    }

    bool update(uint16_t id, byte *record)
    {
        size_t address = calculateAddress(id);

        return (writeRecord(address, record));
    }

    bool select(uint16_t id, byte *record)
    {
        size_t address = calculateAddress(id);

        return (readRecord(address, record));
    }

    bool remove(uint16_t id)
    {
        size_t address = calculateAddress(id);

        return (deleteRecord(address));
    }

    void close(void)
    {
#if DEBUG
        System::logger.write(LOG_ERROR, "Database::closed: file=%s", filename);
#endif
        file.close();
        isOpen = true;
    }

private:
    size_t calculateAddress(uint16_t id)
    {
        return (dataOffset + (id * (header.recordSize + 1)));
    }

    bool writeHeader(void)
    {
        file.seek(headerOffset);
        file.write(DB_RECORD header, sizeof(Header));
        file.sync();
        return (true);
    }

    bool readHeader(void)
    {
        file.seek(headerOffset);
        file.read(DB_RECORD header, sizeof(Header));
        return (true);
    }

    bool writeRecord(size_t address, byte *record)
    {
        if (!file.seek(address)) {
            System::logger.write(LOG_ERROR,
                                 "write: cannot seek: %d, max size=%d",
                                 address,
                                 file.size());
            return (false);
        }

        file.write(1);
        int rc = file.write(record, header.recordSize);
        file.sync();

        //logMessage ("write address=%d, written=%d, (%s)", address, rc, ((SnapshotRecord *) record)->name);
        return ((rc == header.recordSize) ? true : false);
    }

    bool readRecord(size_t address, byte *record)
    {
        int rc = 0;

        if (!file.seek(address)) {
            System::logger.write(LOG_ERROR,
                                 "read: cannot seek: %d, max size=%d",
                                 address,
                                 file.size());
            return (false);
        }

        int isNotNull = file.read();

        if (isNotNull) {
            rc = file.read(record, header.recordSize);
        }

#ifdef DEBUG
        System::logger.write(LOG_ERROR,
                             "read address=%d, read=%d, (%s)",
                             address,
                             rc,
                             ((SnapshotRecord *)record)->name);
#endif
        return (((isNotNull == 1) && (rc > 0)) ? true : false);
    }

    bool deleteRecord(size_t address)
    {
        if (!file.seek(address)) {
            System::logger.write(LOG_ERROR,
                                 "delete: cannot seek: %d, max size=%d",
                                 address,
                                 file.size());
            return (false);
        }

        int rc = file.write(0);
        file.sync();

        return ((rc == 1) ? true : false);
    }

    /*
		 * Variables
		 */
    char filename[maxDatabaseFilenameLength + 1];
    File file;

    struct Header {
        Header() : numRecords(0), recordSize(0){};
        uint16_t numRecords;
        uint16_t recordSize;
    } header;

    static constexpr size_t headerOffset = 0;
    size_t dataOffset;
    bool isOpen;
};
