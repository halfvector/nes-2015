#pragma once

#include "Memory.h"
#include "Registers.h"

class Stack {
public:

    Stack(Memory* mem, Registers* reg)
            : mem(mem), reg(reg) {}

    void pushStack(tCPU::word value);
    tCPU::byte popStack();
    tCPU::word popStackWord();

protected:
    Memory* mem;
    Registers* reg;
};
