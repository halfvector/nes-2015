#include "CPU.h"

CPU::CPU(Registers* registers, Memory* memory, Stack* stack)
        : registers(registers), memory(memory), stack(stack) {

}

/**
 * Load ROM into emulator memory
 * Only supports NROM 32kB ROMs (no memory mappers)
 */
void
CPU::load(Cartridge& rom) {

    instructions = new Instructions(opcodes, modes);
    instructions->initialize();

    ctx = new InstructionContext();
    ctx->mem = memory;
    ctx->registers = registers;
    ctx->stack = stack;

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
void
CPU::writePrgPage(int pageIdx, uint8_t buffer[]) {
    tCPU::dword pageAddress = 0x8000 + 0x4000 * pageIdx;
    PrintCpu("Writing 16k PRG ROM to Page %d (@ 0x%08X)") % pageIdx % (int) pageAddress;

//    for(int j = 0; j < PRG_ROM_PAGE_SIZE; j+=8) {
//        PrintDbg("%05X %02X %02X %02X %02X %02X %02X %02X %02X")
//                % (int) (pageAddress + j)
//                % (int) buffer[j+0]
//                % (int) buffer[j+1]
//                % (int) buffer[j+2]
//                % (int) buffer[j+3]
//                % (int) buffer[j+4]
//                % (int) buffer[j+5]
//                % (int) buffer[j+6]
//                % (int) buffer[j+7];
//    }

    memcpy(memory->getByteArray() + pageAddress, buffer, 0x4000);
}

/**
 * Write 8kB page to PPU memory
 */
void
CPU::writeChrPage(uint8_t buffer[]) {
    memcpy(memory->getByteArray(), buffer, 0x2000);
}

void
CPU::run() {
    reset();
    PrintCpu("Reset program-counter to 0x%X") % registers->PC;

    //while(cpuAlive) {
    for(int i = 0; i < 30; i ++) {
        // grab next instruction
        tCPU::byte opCode = memory->readByteDirectly(registers->PC);
        executeOpcode(opCode);


        // ...
    }
}

/**
 * Reset program counter
 */
void
CPU::reset() {
    registers->PC = memory->readWord(RESET_VECTOR_ADDR);
}

int
CPU::executeOpcode(int code) {
    unsigned char opcodeSize = opcodes[code].Bytes;
    AddressMode mode = opcodes[code].AddressMode;
    const char* mnemonic = opcodes[code].Mnemonic;
    const char* title = AddressModeTitle[static_cast<uint8_t>(mode)];

//    sprintf( StatusBuffer, "C:%d Z:%d V:%d N:%d I:%d B:%d S:%d | A:$%02X X:$%02X Y:$%02X",
//             g_Registers.P.C, g_Registers.P.Z, g_Registers.P.V, g_Registers.P.N, g_Registers.P.I, g_Registers.P.B, g_Registers.A, g_Registers.X, g_Registers.Y, g_Registers.S );


    std::string instruction;

    if (opcodeSize == 1) {

        instruction = (boost::format("%08X: %02X          %s " + modes[mode].addressLine)
                  % (int) registers->PC
                  % code
                  % mnemonic).str();
    } else if (opcodeSize == 2) {
        unsigned char data1 = memory->readByte(registers->PC + 1);
        instruction = (boost::format("%08X: %02X %02X       %s " + modes[mode].addressLine)
                  % (int) registers->PC
                  % code % (int) data1
                  % mnemonic % (int) data1).str();
    } else if (opcodeSize == 3) {
        unsigned char lowByte = memory->readByte(registers->PC + 1);
        unsigned char highByte = memory->readByte(registers->PC + 2);
        instruction = (boost::format("%08X: %02X %02X %02X    %s " + modes[mode].addressLine)
                  % (int) registers->PC
                  % code % (int) lowByte % (int) highByte
                  % mnemonic % (int) highByte % (int) lowByte).str();
    }

    auto cpuState = boost::format("A:%02X X:%02X Y:%02X P:%02X SP:%02X CYCLE:%05d")
            % (int) ctx->registers->A
            % (int) ctx->registers->X
            % (int) ctx->registers->Y
            % (int) ctx->registers->P.asByte()      // processor status summary
            % (int) ctx->registers->S               // stack pointer
            % (int) numCycles
    ;

    PrintCpu("%-45s %s")
        % instruction % cpuState;

    // update program counter
    registers->LastPC = registers->PC;
    registers->PC += opcodeSize;

    instructions->execute(code, ctx);

    // opcode cycle count + any page boundary penalty
    uint8_t cycles = opcodes[code].Cycles;
    if(opcodes[code].PageBoundaryCondition && MemoryAddressResolveBase::PageBoundaryCrossed) {
        cycles ++;
    }
    numCycles += cycles;

    return cycles;
}

uint64_t
CPU::getCycleRuntime() {
    return numCycles;
}
