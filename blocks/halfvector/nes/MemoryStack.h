#pragma once

#include "Memory.h"
#include "Registers.h"

class Stack {
public:

    Stack(shared_ptr <Memory> mem, shared_ptr <Registers> reg)
            : mem(mem), reg(reg) {}

    void pushStack(tCPU::word value);
    tCPU::byte popStack();

protected:
    shared_ptr<Memory> mem;
    shared_ptr<Registers> reg;
};
