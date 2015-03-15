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

    return readByteDirectly(address);
}

/**
 * Read a byte from memory without attempting to resolve address
 */
tCPU::byte
Memory::readByteDirectly(tCPU::word address) {
    //PrintMemory("Reading from memory address: 0x%08X without address resolution") % address;

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
    tCPU::dword lowByte = readByte(absoluteAddress);
    tCPU::dword highByte = readByte(absoluteAddress + 1);
    tCPU::dword value = (highByte << 8) | lowByte;
    PrintDbg("Memory::readWord(); Read word=0x%08X (0x%04X + 0x%04X) from 0x%X")
            % value % (int) highByte % (int) lowByte % (int) absoluteAddress;
    return value;
}

bool
Memory::writeByte(tCPU::word originalAddress, tCPU::byte value) {
    return writeByteDirectly(getRealMemoryAddress(originalAddress), value);
}

bool
Memory::writeByteDirectly(tCPU::word address, tCPU::byte value) {
    if ((address >= 0x2000 && address <= 0x2007) || (address >= 0x4000 && address <= 0x401F)) {
        // i/o registers
        PrintMemory("Address 0x%08X is a Memory Mapped I/O Port") % address;
        return writeToIOPort(address, value);
    } else {
        // fun fact: stack memory
        if (address >= 0x0100 && address <= 0x01FF) {
            PrintMemory("Address 0x%08X is the Stack") % address;
        }

        // regular memory
        PrintMemory("Writing %02X to 0x%08X") % (int) value % (int) address;
        memory[address] = value;
    }
}

tCPU::byte
Memory::readFromIOPort(tCPU::word Index) {
    // TODO: implement i/o handlers
    return 0;
}

bool
Memory::writeToIOPort(tCPU::word Index, tCPU::byte value) {
    // TODO: implement i/o handlers
    return 0;
}