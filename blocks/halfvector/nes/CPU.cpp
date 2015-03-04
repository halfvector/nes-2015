#include "CPU.h"
#include "Logging.h"

CPU::CPU() {
    cpuMemoryAccessor = new Memory(cpuMemory);
}

/**
 * Load ROM into emulator memory
 * Only supports NROM 32kB ROMs (no memory mappers)
 */
void
CPU::load(Cartridge rom) {
    // write program pages
    if (rom.header.numPrgPages == 1) {
        // special case: just one program data page, duplicate it
        writePrgPage(0, rom.programDataPages[0].buffer);
        writePrgPage(1, rom.programDataPages[0].buffer);
    } else {
        // write both pages of the 32kB rom
        writePrgPage(0, rom.programDataPages[0].buffer);
        writePrgPage(1, rom.programDataPages[1].buffer);
    }

    for (uint8_t i = 0; i < rom.header.numChrPages; i++) {
        writeChrPage(rom.characterDataPages[i].buffer);
    }
}

/**
 * Write 16kB page to CPU memory
 */
void CPU::writePrgPage(int pageIdx, uint8_t buffer[]) {
    tCPU::word pageAddress = 0x8000 + 0x4000 * pageIdx;
    PrintInfo("Writing 16k PRG ROM to Page %d (@ 0x%08X)") % pageIdx % pageAddress;

    memcpy(cpuMemory + pageAddress, buffer, 0x4000);
}

/**
 * Write 8kB page to PPU memory
 */
void CPU::writeChrPage(uint8_t buffer[]) {
    memcpy(ppuMemory, buffer, 0x2000);
}

void CPU::run() {
    reset();

    while(cpuAlive) {
        // grab next instruction
        tCPU::byte opCode = cpuMemoryAccessor->readByteDirectly(registers.PC);

        // ...
    }
}

/**
 * Reset program counter
 */
void CPU::reset() {
    registers.PC = cpuMemoryAccessor->readWord(0xFFFC);
}
