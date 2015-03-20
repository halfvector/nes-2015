#pragma once

#include "Logging.h"
#include "Platform.h"


typedef void (*WriteIO)(tCPU::byte);
typedef tCPU::byte (*ReadIO)(void);

enum IOAccessMode { READ_WRITE, READ_ONLY, WRITE_ONLY };

template<tCPU::word Address>
struct MemoryIOHandler {
    static tCPU::byte read() {
        PrintWarning("MemoryIO::Read(); Unhandled Memory Mapped I/O at 0x%04X") % Address;
        return 0;
    }

    static bool write(tCPU::byte Value) {
        PrintWarning("MemoryIO::Write(); Unhandled Memory Mapped I/O at 0x%04X") % Address;
        return false;
    }
};

/**
* Register read/write handlers for memory mapped i/o operations
* each memory address will have an appropriate read/write handler
*/
struct MemoryIO {
    MemoryIO();

    void registerHandler(tCPU::word ioPort, WriteIO writer) {
        ioPortWriters[ioPort] = writer;
    }

    void registerHandler(tCPU::word ioPort, ReadIO reader) {
        ioPortReaders[ioPort] = reader;
    }
    bool write(tCPU::word address, tCPU::byte value);

    tCPU::byte read(tCPU::word address);

    std::map<tCPU::word, WriteIO> ioPortWriters;
    std::map<tCPU::word, ReadIO> ioPortReaders;
};
