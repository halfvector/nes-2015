#include "MemoryStack.h"

/**
 * Manage stack portion of memory
 */

/*
 * Push word. Increment stack by 2 bytes.
 */
void
Stack::pushStackWord(tCPU::word value) {
    assert(reg->S > 2 && "Stack overflow");

    PrintMemory("Pushing onto stack: 0x%04X / stack pointer: $%02X") % (int) value % (int) reg->S;
    mem->writeByte(0x100 + reg->S - 0, (value >> 8) & 0xFF); // high byte
    mem->writeByte(0x100 + reg->S - 1, value & 0xFF); // low byte
    reg->S -= 2;
}

/**
 * Pop word. Decrement stack by 2 bytes;
 */
tCPU::word
Stack::popStackWord() {
    assert(reg->S <= 0xFD && "Stack underflow");

    reg->S += 2;
    tCPU::word value = 0;
    value |= ((tCPU::word) mem->readByte(0x100 + reg->S - 0)) << 8; // high byte
    value |= mem->readByte(0x100 + reg->S - 1); // low byte
    PrintMemory("Popped from stack: 0x%04X / stack pointer: $%02X") % (int) value % (int) reg->S;
    return value;
}

void
Stack::pushStackByte(tCPU::byte value) {
    assert(reg->S > 1 && "Stack overflow");

    PrintMemory("Pushing onto stack: 0x%04X / stack pointer: $%02X") % (int) value % (int) reg->S;
    mem->writeByte(0x100 + reg->S, value);
    reg->S --;
}

tCPU::byte
Stack::popStackByte() {
    assert(reg->S <= 0xFE && "Stack overflow");

    reg->S ++;
    tCPU::byte value = mem->readByte(0x100 + reg->S);
    PrintMemory("Popped from stack: 0x%02X / stack pointer: $%02X") % (int) value % (int) reg->S;
    return value;
}
