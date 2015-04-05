#include "MemoryIO.h"
#include <functional>
#include <AddressBook/AddressBook.h>

template<>
struct MemoryIOHandler<0x2002> {
    static tCPU::byte Read(PPU* ppu) {
        tCPU::byte value = ppu->getStatusRegister();
        PrintDbg("MemoryIOHandler<0x%04X>::Read(); status register = %d")
                % 0x2002 % (int) value;
        return value;
    }
};

MemoryIO::MemoryIO(PPU* ppu) : ppu(ppu) {
    ioPortWriters.clear();
    ioPortReaders.clear();

    // bind parameters to methods
    registerHandler(0x2002, std::bind(MemoryIOHandler<0x2002>::Read, ppu));
    //registerHandler(0x2002, MemoryIOHandler<0x2002>::Read);
}

bool
MemoryIO::write(tCPU::word address, tCPU::byte value) {
    return 0;
}

tCPU::byte
MemoryIO::read(tCPU::word address) {
    switch(address) {
        case 0x2002:
            return MemoryIOHandler<0x2002>::Read(ppu);

        default:
            PrintDbg("MemoryIO::Read(); address = 0x%04X not supported") % address;
            return 0;
    }

//    auto reader = ioPortReaders.find(address);
//    if(reader != ioPortReaders.end()) {
//        return reader->second();
//    }
//    return 0;
}
