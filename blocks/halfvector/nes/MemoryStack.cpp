#include "MemoryStack.h"

/**
 * Manage stack portion of memory
 */

/*
 * Push word. Increment stack by 2 bytes.
 */
void
Stack::pushStack(unsigned short value) {
    PrintMemory("Pushing onto stack: 0x%04X") % (int) value;
    unsigned short address = 0x1FF - reg->S - 1;
    mem->writeWord(address, value);

    if(reg->S < 255) {
    	reg->S += 2;
    } else {
    	PrintError("Stack overflow");
    	throw new std::runtime_error("Stack overflow");
    }
}

/**
 * Pop word. Decrement stack by 2 bytes;
 */
tCPU::word Stack::popStackWord() {
    assert(reg->S >= 2 && "Stack underflow");
    reg->S -= 2;

    tCPU::word value = mem->readWord(0x1FF - reg->S - 1);
    PrintMemory("Popped from stack: 0x%04X") % (int) value;
    return value;
}

unsigned char Stack::popStack() {
    throw new std::runtime_error("popStack() not implemented");
}
