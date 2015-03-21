#include "CPU.h"
#include "Logging.h"

CPU::CPU(Memory* memory) : memory(memory) {
    instructions = new Instructions(opcodes, modes);
    instructions->initialize();

    ctx = new InstructionContext();
    ctx->mem = memory;
    ctx->registers = &registers;
}

/**
 * Load ROM into emulator memory
 * Only supports NROM 32kB ROMs (no memory mappers)
 */
void
CPU::load(Cartridge rom) {
    // write program pages
    if (rom.header.numPrgPages == 1) {
        // special case: we have just one program data page (16kB ROM)
        // duplicate page so reset address will work from page 2
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
    tCPU::dword pageAddress = 0x8000 + 0x4000 * pageIdx;
    PrintInfo("Writing 16k PRG ROM to Page %d (@ 0x%08X)") % pageIdx % (int) pageAddress;

    memcpy(memory->getByteArray() + pageAddress, buffer, 0x4000);
}

/**
 * Write 8kB page to PPU memory
 */
void CPU::writeChrPage(uint8_t buffer[]) {
    memcpy(memory->getByteArray(), buffer, 0x2000);
}

void CPU::run() {
    reset();
    PrintDbg("Reset program-counter to 0x%X") % registers.PC;

    //while(cpuAlive) {
    for(int i = 0; i < 30; i ++) {
        // grab next instruction
        tCPU::byte opCode = memory->readByteDirectly(registers.PC);
        executeOpcode(opCode);


        // ...
    }
}

/**
 * Reset program counter
 */
void CPU::reset() {
    registers.PC = memory->readWord(RESET_VECTOR_ADDR);
}

void CPU::executeOpcode(int code) {
    unsigned char opcodeSize = opcodes[code].Bytes;
    AddressMode mode = opcodes[code].AddressMode;
    const char* mnemonic = opcodes[code].Mnemonic;
    const char* title = AddressModeTitle[static_cast<uint8_t>(mode)];

    if(opcodeSize == 1) {
        PrintInfo("%08X: %02X\t\t%s " + modes[mode].addressLine)
                % (int) registers.PC
                % code
                % mnemonic;
    } else if(opcodeSize == 2) {
        unsigned char data1 = memory->readByte(registers.PC+1);
        PrintInfo("%08X: %02X %02X\t\t%s " + modes[mode].addressLine)
                % (int) registers.PC
                % code % (int) data1
                % mnemonic % (int) data1;
    } else if(opcodeSize == 3) {
        unsigned char lowByte = memory->readByte(registers.PC+1);
        unsigned char highByte = memory->readByte(registers.PC+2);
        PrintInfo("%08X: %02X %02X %02X\t%s " + modes[mode].addressLine)
                % (int) registers.PC
                % code % (int) lowByte % (int) highByte
                % mnemonic % (int) highByte % (int) lowByte;
    }

    // update program counter
    registers.LastPC = registers.PC;
    registers.PC += opcodeSize;

    instructions->execute(code, ctx);
}
