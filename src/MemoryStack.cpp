#include "MemoryStack.h"

const unsigned short stackOffset = 0x100;

/**
 * Manage stack portion of memory
 */

inline std::uint16_t operator "" _us(unsigned long long value) {
    return static_cast<std::uint16_t>(value);
}

/*
 * Push word. Increment stack by 2 bytes.
 */
void
Stack::pushStackWord(tCPU::word value) {
    assert(reg->S > 1 && "Stack overflow");

    PrintDbg("Pushing onto stack: 0x%04X / stack pointer: $%02X", (int) value, (int) reg->S);
    mem->writeByte(stackOffset + reg->S, (value >> 8) & 0xFF); // high byte
    mem->writeByte(stackOffset + reg->S - 1_us, value & 0xFF_us); // low byte
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
    value |= mem->readByte(stackOffset + reg->S) << 8; // high byte
    value |= mem->readByte(stackOffset + reg->S - 1); // low byte

    mem->writeByte(stackOffset + reg->S, 0);
    mem->writeByte(stackOffset + reg->S - 1, 0);

    PrintDbg("Popped from stack: 0x%04X / stack pointer: $%02X", (int) value, (int) reg->S);
    return value;
}

void
Stack::pushStackByte(tCPU::byte value) {
    assert(reg->S > 0 && "Stack overflow");

    PrintDbg("Pushing onto stack: 0x%04X / stack pointer: $%02X", (int) value, (int) reg->S);
    mem->writeByte(stackOffset + reg->S, value);
    reg->S--;
}

tCPU::byte
Stack::popStackByte() {
    assert(reg->S <= 0xFE && "Stack overflow");

    reg->S++;
    tCPU::byte value = mem->readByte(stackOffset + reg->S);
    PrintDbg("Popped from stack: 0x%02X / stack pointer: $%02X", (int) value, (int) reg->S);

    mem->writeByte(stackOffset + reg->S, 0);

    return value;
}

void
Stack::dump() {
    PrintMemoryIO("Stack position: %0X", (unsigned short) (reg->S));
    for (unsigned short i = 0; i <= 0xFF; i++) {
        PrintMemoryIO("%x: %x", (stackOffset + i), (unsigned short) (mem->readByte(stackOffset + i)));
    }
}
