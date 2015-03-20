#include "MemoryIO.h"

template<>
struct MemoryIOHandler<0x2002> {
    static tCPU::byte Read() {
        PrintDbg("IO_Handler<0x%04X>::Read(); Reading from I/O Port at $%04X")
                % 0x2002 % 0x2002;

        // FIXME: implement PPU get status register
        //return PPU::Singleton()->GetStatusRegister();
        return 0;
    }

    enum {
        Hooked = 1
    };
    enum {
        AccessMode = IOAccessMode::READ_ONLY
    };
};

MemoryIO::MemoryIO() {
    ioPortWriters.clear();
    ioPortReaders.clear();

    registerHandler(2002, MemoryIOHandler<0x2002>::Read);
}

bool
MemoryIO::write(tCPU::word address, tCPU::byte value) {
    return 0;
}

tCPU::byte
MemoryIO::read(tCPU::word address) {
    return 0;
}
