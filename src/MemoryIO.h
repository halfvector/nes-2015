#pragma once

#include <map>
#include "Logging.h"
#include "Platform.h"
#include "PPU.h"
#include "Joypad.h"
#include "Audio.h"

template<tCPU::word Address>
struct MemoryIOHandler {
    static tCPU::byte read() {
        PrintWarning("MemoryIO::Read(); Unhandled Memory Mapped I/O at 0x%04X", Address);
        throw std::runtime_error("Unexpected warning");
    }

    static bool write(tCPU::byte Value) {
        PrintWarning("MemoryIO::Write(); Unhandled Memory Mapped I/O at 0x%04X", Address);
        throw std::runtime_error("Unexpected warning");
    }
};

/**
* Register read/write handlers for memory mapped i/o operations
* each memory address will have an appropriate read/write handler
*/
struct MemoryIO {
    MemoryIO(PPU *ppu, Joypad *joypad, Audio* apu);

    bool write(tCPU::word address, tCPU::byte value);

    tCPU::byte read(tCPU::word address);

    void setMemory(Memory *memory);

    static int cpuCyclesPenalty;

protected:
    PPU* ppu;
    Audio* apu;
    Memory* memory;
    Joypad* joypad;
};
