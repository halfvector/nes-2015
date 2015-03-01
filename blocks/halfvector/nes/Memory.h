#pragma once

#include "Platform.h"

/**
* Wraps memory reads and writes
* handles Memory Mapped I/O
*/

class Memory {
public:

    Memory(tCPU::byte *memory) {
        this->memory = memory;
    }

    tCPU::word getRealMemoryAddress(tCPU::word address);

    tCPU::byte readByte(tCPU::word address);

    tCPU::word readWord(tCPU::word absoluteAddress);

    tCPU::byte readFromIOPort(tCPU::word address);

protected:

    tCPU::byte *memory;
};

