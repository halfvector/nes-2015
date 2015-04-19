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
        PrintMemory("Adjusting memory address for mirror");
        address -= 0x0800;
    } else if (address >= 0x1000 && address <= 0x17FF) {
        PrintMemory("Adjusting memory address for mirror");
        address -= 0x1000;
    } else if (address >= 0x1800 && address <= 0x1FFF) {
        PrintMemory("Adjusting memory address for mirror");
        address -= 0x1800;
    }

    // repeating mirrors of the below i/o registers
    if (address >= 0x2008 && address < 0x3FFF) {
        address = 0x2000 + (address % 8);
        PrintMemory("Adjusted Repeating I/O Mirror to 0x%04X") % address;
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
        PrintMemory("* Reading from Memory Mapped I/O Port (0x%04X)") % (int) address;
        return readFromIOPort(address);
    } else {
        // fun fact: stack memory
        if (address >= 0x0100 && address <= 0x01FF) {
            PrintMemory("* Reading from Stack (0x%04X)") % (int) address;
        }

        // regular memory
        tCPU::byte value = memory[address];
        PrintMemory("Read 0x%02X from $%04X") % (int) value % (int) address;
        return value;
    }
}

// reads word at absolute address
// expects word to be stored as [LSB][MSB] at address and address+1
tCPU::word
Memory::readWord(tCPU::word absoluteAddress) {
    tCPU::word lowByte = readByte(absoluteAddress);
    tCPU::word highByte = readByte(absoluteAddress + 1);
    tCPU::word value = ((highByte << 8) & 0xFF00) | lowByte;
    PrintMemory("Read 0x%04X from address $%04X")
            % value % (int) absoluteAddress;
    return value;
}

/**
 * Resolves memory address through mirroring
 * Determines if memory address is an I/O port, stack access, or heap write
 */
bool
Memory::writeByte(tCPU::word originalAddress, tCPU::byte value) {
    tCPU::word address = getRealMemoryAddress(originalAddress);

    if ((address >= 0x2000 && address <= 0x2007) || (address >= 0x4000 && address <= 0x401F)) {
        // i/o registers
        PrintMemory("* Writing to Memory Mapped I/O Port (0x%04X)") % (int) address;
        return writeToIOPort(address, value);
    } else {
        // fun fact: stack memory
        if (address >= 0x0100 && address <= 0x01FF) {
            PrintMemory("* Writing to Stack (0x%04X)") % (int) address;
        }

        // regular memory
        memory[address] = value;
        PrintMemory("Wrote 0x%02X to $%04X") % (int) value % (int) address;
        return true;
    }
}

// c++11 suffix operator to create short int literals (how is this still not a language feature?)
//inline uint16_t operator "" _us(unsigned long long int value) {
//    return static_cast<uint16_t>(value);
//}

void
Memory::writeWord(tCPU::word address, tCPU::word value) {
    // break word down into bytes
    tCPU::byte lowByte = value & 0xFF;
    tCPU::byte highByte = ((value & 0xFF00) >> 8) & 0xFF;

    // write bytes
    writeByte(address, lowByte);
    writeByte(address+1, highByte);

    PrintMemory("Wrote 0x%04X to $%04X") % (int) value % (int) address;
}

/**
 * Writes a byte to the specified address. Does not perform address transformation.
 */
bool
Memory::writeByteDirectly(tCPU::word address, tCPU::byte value) {

}

tCPU::byte
Memory::readFromIOPort(const tCPU::word address) {
    return MMIO->read(address);
}


bool
Memory::writeToIOPort(const tCPU::word address, tCPU::byte value) {
    //return MemoryIO<static_cast<int>(address)>::write(value);
    return MMIO->write(address, value);
}

tCPU::byte*
Memory::getByteArray() {
    return memory;
}
