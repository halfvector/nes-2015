#pragma once

#include <cstdint>
#include "Cartridge.h"
#include "Logging.h"
#include "Memory.h"
#include "Platform.h"
#include "Instructions.h"
#include "Registers.h"

class CPU {
public:
    CPU();

    void load(Cartridge);
    void run();

protected:
    tCPU::byte cpuMemory[0x100000];
    tCPU::byte ppuMemory[0x4000];
    Opcode opcodes[0x100];

    Memory *cpuMemoryAccessor;
    Registers registers;
    bool cpuAlive = true;

    void writePrgPage(int i, uint8_t buffer[]);
    void writeChrPage(uint8_t buffer[]);
    void reset();
};