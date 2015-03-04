#pragma once

#include "Platform.h"

enum AddressMode {
    ADDR_MODE_NONE = 0, ADDR_MODE_ABSOLUTE, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE,
    ADDR_MODE_RELATIVE, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED,
    ADDR_MODE_INDIRECT_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y,
    ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ZEROPAGE_INDEXED_Y,
    ADDR_MODE_ACCUMULATOR, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_LAST
};

static const char* AddressModeTitle[] = {
        "None", "Absolute", "Immediate", "Zeropage", "Relative", "Indexed Indirect",
        "Indirect Indexed", "Indirect Absolute", "Absolute Indexed X", "Absolute Indexed Y",
        "Zeropage Indexed X", "Zeropage Indexed Y", "Accumulator", "Immediate to X/Y",
        "THE END"
};

static uint16_t AddressModeMask[] = {
        0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100,
        0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000
};

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
    tCPU::byte readByteDirectly(tCPU::word address);

protected:
    tCPU::byte *memory;
};

