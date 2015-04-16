#include "MemoryIO.h"
#include <functional>
#include <AddressBook/AddressBook.h>

template<>
struct MemoryIOHandler<0x2002> {
    static tCPU::byte read(PPU* ppu) {
        tCPU::byte value = ppu->getStatusRegister();
        PrintMemory("MemoryIOHandler<0x%04X>::Read(); status register = %d")
                % 0x2002 % (int) value;
        return value;
    }
};

template<>
struct MemoryIOHandler<0x2000> {
    static void write(PPU* ppu, tCPU::byte value) {
        PrintMemory("MemoryIOHandler<0x%04X>::write(); writing port $2000 register with value = %02X")
                   % 0x2000 % (int) value;
        ppu->setControlRegister(value);
    }
};

MemoryIO::MemoryIO(PPU* ppu) : ppu(ppu) {
    ioPortWriters.clear();
    ioPortReaders.clear();

    // bind parameters to methods
    registerHandler(0x2002, std::bind(MemoryIOHandler<0x2002>::read, ppu));
    //registerHandler(0x2002, MemoryIOHandler<0x2002>::Read);
}

bool
MemoryIO::write(tCPU::word address, tCPU::byte value) {
    switch(address) {
        case 0x2000:
            MemoryIOHandler<0x2000>::write(ppu, value);
            break;

        default:
            PrintMemory("MemoryIO::write(); address = 0x%04X not supported") % address;
            return false;
    }

    return true;
}

tCPU::byte
MemoryIO::read(tCPU::word address) {
    switch(address) {
        case 0x2002:
            return MemoryIOHandler<0x2002>::read(ppu);

        default:
            PrintMemory("MemoryIO::Read(); address = 0x%04X not supported") % address;
            return 0;
    }

//    auto reader = ioPortReaders.find(address);
//    if(reader != ioPortReaders.end()) {
//        return reader->second();
//    }
//    return 0;
}
