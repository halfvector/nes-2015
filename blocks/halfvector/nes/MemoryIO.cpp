#include "MemoryIO.h"
#include "Exceptions.h"
#include <functional>

template<>
struct MemoryIOHandler<0x2002> {
    static tCPU::byte read(PPU* ppu) {
        tCPU::byte value = ppu->getStatusRegister();
        PrintMemoryIO("Reading port 0x%04X; status register = 0x%02X", 0x2002, (int) value);
        return value;
    }
};

template<>
struct MemoryIOHandler<0x2000> {
    static tCPU::byte read(PPU* ppu) {
        tCPU::byte value = ppu->getControlRegister1();
        PrintMemoryIO("Reading port 0x%04X; status register = 0x%02X", 0x2000, (int) value);
        return value;
    }
    static void write(PPU* ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2000 - PPU Control Register 1", (int) value);
        ppu->setControlRegister1(value);
    }
};

template<>
struct MemoryIOHandler<0x2001> {
    static void write(PPU* ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2001 - PPU Control Register 2", (int) value);
        ppu->setControlRegister2(value);
    }
};

template<>
struct MemoryIOHandler<0x2003> {
    static void write(PPU* ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2003 - Sprite RAM Address", (int) value);
        ppu->setSprRamAddress(value);
    }
};

template<>
struct MemoryIOHandler<0x2004> {
    static tCPU::byte read(PPU* ppu) {
        tCPU::byte value = ppu->readSpriteMemory();
        PrintMemoryIO("Read 0x%02X from port $2004 - Sprite RAM I/O Register", (int) value);
        return value;
    }
    static void write(PPU* ppu, tCPU::byte value) {
        PrintInfo("Write 0x%02X to port $2004 - Sprite RAM I/O Register", (int) value);
        ppu->writeSpriteMemory(value);
    }
};

template<>
struct MemoryIOHandler<0x2005> {
    static tCPU::byte read(PPU* ppu) {
        tCPU::byte value = ppu->readSpriteMemory();
        PrintMemoryIO("Read 0x%02X from port $2005 - Sprite RAM I/O Register", (int) value);
        return value;
    }
    static void write(PPU* ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2005 - VRAM Address Register 2", (int) value);
        ppu->setVRamAddressRegister1(value);
    }
};

template<>
struct MemoryIOHandler<0x2006> {
    static void write(PPU* ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2006 - VRAM Address Register 2", (int) value);
        ppu->setVRamAddressRegister2(value);
    }
};

template<>
struct MemoryIOHandler<0x2007> {
    static tCPU::byte read(PPU* ppu) {
        tCPU::byte value = ppu->readFromVRam();
        PrintMemoryIO("Read 0x%02X from port $2007 - VRAM I/O Register", (int) value);
        return value;
    }
    static void write(PPU* ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2007 - VRAM I/O Register", (int) value);
        ppu->writeToVRam(value);
    }
};

template<>
struct MemoryIOHandler<0x4014> {
    static void write(PPU* ppu, Memory* memory, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $2006 - VRAM Sprite DMA Xfer", (int) value);
        ppu->StartSpriteXferDMA(memory, value);
    }
};

MemoryIO::MemoryIO(PPU* ppu) : ppu(ppu) {
}

class MemoryIOPortException : public ExceptionBase<MemoryIOPortException> {
public:
    MemoryIOPortException(std::string const &str)
            : ExceptionBase(str) {}
};

bool
MemoryIO::write(tCPU::word address, tCPU::byte value) {
    switch(address) {
        case 0x2000:
            MemoryIOHandler<0x2000>::write(ppu, value);
            break;

        case 0x2001:
            MemoryIOHandler<0x2001>::write(ppu, value);
            break;

        case 0x2003:
            MemoryIOHandler<0x2003>::write(ppu, value);
            break;

        case 0x2004:
            MemoryIOHandler<0x2004>::write(ppu, value);
            break;

        case 0x2005:
            MemoryIOHandler<0x2005>::write(ppu, value);
            break;

        case 0x2006:
            MemoryIOHandler<0x2006>::write(ppu, value);
            break;

        case 0x2007:
            MemoryIOHandler<0x2007>::write(ppu, value);
            break;

        case 0x4000 ... 0x4013:
            PrintUnimplementedIO("Skipping Unimplemented I/O Port: APU $40XX - APU");
            break;

        case 0x4014:
            MemoryIOHandler<0x4014>::write(ppu, memory, value);
            break;

        case 0x4015:
            PrintUnimplementedIO("Skipping Unimplemented I/O Port: APU $4015 - APU Sound / Vertical Clock Signal Register");
            break;

        case 0x4016:
            PrintUnimplementedIO("Skipping Unimplemented I/O Port: $4016 - Joypad 1");
            break;

        case 0x4017:
            PrintUnimplementedIO("Skipping Unimplemented I/O Port: $4017 - Joypad 2");
            break;

        default:
            MemoryIOPortException::emit("MemoryIO::write(); address = 0x%04X not supported", address);
    }

    return true;
}

tCPU::byte
MemoryIO::read(tCPU::word address) {
    switch(address) {
        case 0x2000:
            return MemoryIOHandler<0x2000>::read(ppu);
            
        case 0x2002:
            return MemoryIOHandler<0x2002>::read(ppu);

        case 0x2004:
            return MemoryIOHandler<0x2004>::read(ppu);

        case 0x2007:
            return MemoryIOHandler<0x2007>::read(ppu);

        case 0x4016:
            PrintUnimplementedIO("Skipping Unimplemented I/O Port: PPU $4016 - Joypad 1");
            break;

        case 0x4017:
            PrintUnimplementedIO("Skipping Unimplemented I/O Port: PPU $4017 - Joypad 2");
            break;

        default:
            MemoryIOPortException::emit("MemoryIO::read(); address = 0x%04X not supported", address);
    }

//    auto reader = ioPortReaders.find(address);
//    if(reader != ioPortReaders.end()) {
//        return reader->second();
//    }
    return 0;
}

void
MemoryIO::setMemory(Memory *memory) {
    this->memory = memory;
}
