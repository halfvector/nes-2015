#pragma once

#include "Memory.h"
#include "Registers.h"

class Stack {
public:

    Stack(Memory* mem, Registers* reg)
            : mem(mem), reg(reg) {}

    void pushStackWord(tCPU::word value);
    void pushStackByte(tCPU::byte value);
    tCPU::word popStackWord();
    tCPU::byte popStackByte();

protected:
    Memory* mem;
    Registers* reg;
};
