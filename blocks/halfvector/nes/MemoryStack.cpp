#include "MemoryStack.h"

/**
 * Manage stack portion of memory
 */

/*
 * Push word. Increment stack by 2 bytes.
 */
void
Stack::pushStackWord(tCPU::word value) {
    assert(reg->S < 253 && "Stack overflow");

    PrintMemory("Pushing onto stack: 0x%04X / stack pointer: $%02X") % (int) value % (int) reg->S;
    mem->writeWord(0x1FF - reg->S - 1, value);
    reg->S += 2;
}

/**
 * Pop word. Decrement stack by 2 bytes;
 */
tCPU::word Stack::popStackWord() {
    assert(reg->S >= 2 && "Stack underflow");

    reg->S -= 2;
    tCPU::word value = mem->readWord(0x1FF - reg->S - 1);
    PrintMemory("Popped from stack: 0x%04X / stack pointer: $%02X") % (int) value % (int) reg->S;
    return value;
}

void
Stack::pushStackByte(tCPU::byte value) {
    assert(reg->S < 254 && "Stack overflow");

    PrintMemory("Pushing onto stack: 0x%04X / stack pointer: $%02X") % (int) value % (int) reg->S;
    mem->writeByte(0x1FF - reg->S - 1, value);
    reg->S ++;
}

tCPU::byte
Stack::popStackByte() {
    assert(reg->S >= 1 && "Stack overflow");

    reg->S --;
    tCPU::word value = mem->readWord(0x1FF - reg->S - 1);
    PrintMemory("Popped from stack: 0x%02X / stack pointer: $%02X") % (int) value % (int) reg->S;
    return value;
}
