#include "Platform.h"
#include "Logging.h"
#include "Memory.h"

/**
 * Calculate real memory address, accounting for memory mirroring.
 */
tCPU::word
Memory::getRealMemoryAddress(tCPU::word address) {
    // mirrors
    if (address >= 0x0800 && address <= 0x0FFF) {
        address -= 0x0800;
    } else if (address >= 0x1000 && address <= 0x17FF) {
        address -= 0x1000;
    } else if (address >= 0x1800 && address <= 0x1FFF) {
        address -= 0x1800;
    }

    // repeating mirrors of the below i/o registers
    if (address >= 0x2008 && address < 0x3FFF) {
        address = 0x2000 + (address % 8);
        PrintDbg("Adjusted Repeating I/O Mirror to 0x%04X") % address;
    }

    return address;
}

/**
 * Read a byte from memory
 */
tCPU::byte
Memory::readByte(tCPU::word originalAddress) {
    tCPU::word address = getRealMemoryAddress(originalAddress);
    PrintMemory("Reading from memory address: 0x%08X (resolved to 0x%08X)") % originalAddress % address;

    if ((address >= 0x2000 && address <= 0x2007) || (address >= 0x4000 && address <= 0x401F)) {
        // i/o registers
        PrintMemory("Address 0x%08X is a Memory Mapped I/O Port") % address;
        return readFromIOPort(address);
    } else {
        // fun fact: stack memory
        if (address >= 0x0100 && address <= 0x01FF) {
            PrintMemory("Address 0x%08X is the Stack") % address;
        }

        // regular memory
        return memory[address];
    }
}

/**
 * Read a byte from memory without attempting to resolve address
 */
tCPU::byte
Memory::readByteDirectly(tCPU::word address) {
    PrintMemory("Reading from memory address: 0x%08X without address resolution") % address;

    if ((address >= 0x2000 && address <= 0x2007) || (address >= 0x4000 && address <= 0x401F)) {
        // i/o registers
        PrintMemory("Address 0x%08X is a Memory Mapped I/O Port") % address;
        return readFromIOPort(address);
    } else {
        // fun fact: stack memory
        if (address >= 0x0100 && address <= 0x01FF) {
            PrintMemory("Address 0x%08X is the Stack") % address;
        }

        // regular memory
        return memory[address];
    }
}

tCPU::word
Memory::readWord(tCPU::word absoluteAddress) {
    tCPU::word value = (readByte(absoluteAddress + 1) << 8) | readByte(absoluteAddress);
    PrintDbg("Read word=0x%04X from 0x%X") % value % absoluteAddress;
    return value;
}

tCPU::byte
Memory::readFromIOPort(tCPU::word Index) {
    // TODO: implement i/o handlers
    return 0;
}