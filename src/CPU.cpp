#include "CPU.h"
#include <cstdio>

CPU::CPU(Registers *registers, Memory *memory, Stack *stack)
        : registers(registers), memory(memory), stack(stack) {

}

/**
 * Load ROM into emulator memory
 * Only supports NROM 32kB ROMs (no memory mappers)
 */
void
CPU::load(Cartridge &rom) {

    instructions = new Instructions(opcodes, modes);
    instructions->initialize();

    ctx = new InstructionContext();
    ctx->mem = memory;
    ctx->registers = registers;
    ctx->stack = stack;
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

    if (Loggy::Enabled == Loggy::DEBUG) {
        AddressMode mode = opcodes[code].AddressMode;
        const char *mnemonic = opcodes[code].Mnemonic;
        const char *title = AddressModeTitle[static_cast<uint8_t>(mode)];

        char instruction[256], fmt[256];
        memset(fmt, 0, 256);

        if (opcodeSize == 1) {
            char *end = strcat(fmt, "%08X: %02X          %s ");
            strcat(end, modes[mode].addressLine);
            sprintf(instruction, fmt, registers->PC, code, mnemonic);
        } else if (opcodeSize == 2) {
            unsigned char data1 = memory->readByte(registers->PC + 1);
            char* end = strcat(fmt, "%08X: %02X %02X       %s ");
            strcat(end, modes[mode].addressLine);
            sprintf(instruction, fmt, registers->PC, code, data1, mnemonic, data1);
        } else if (opcodeSize == 3) {
            unsigned char lowByte = memory->readByte(registers->PC + 1);
            unsigned char highByte = memory->readByte(registers->PC + 2);
            char* end = strcat(fmt, "%08X: %02X %02X %02X    %s ");
            strcat(end, modes[mode].addressLine);
            sprintf(instruction, fmt, registers->PC, code, lowByte,
                    highByte, mnemonic, highByte, lowByte);
        }

        char cpuState[200];

        sprintf(cpuState, "A:%02X X:%02X Y:%02X P:%02X SP:%02X CYCLE:%05d (Carry:%d Zero:%d Sign:%d)",
                (int) ctx->registers->A, (int) ctx->registers->X, (int) ctx->registers->Y,
                (int) ctx->registers->P.asByte(),      // processor status summary
                (int) ctx->registers->S,             // stack pointer
                (int) numCycles, ctx->registers->P.C, ctx->registers->P.Z, ctx->registers->P.N
        );

        PrintDbg("%-45s %s", instruction, cpuState);
    }

    // update program counter
    registers->LastPC = registers->PC;
    registers->PC += opcodeSize;

    MemoryAddressResolveBase::PageBoundaryCrossed = false;
    MemoryIO::cpuCyclesPenalty = 0;

    instructions->execute(code, ctx);

    // opcode cycle count + any page boundary penalty
    uint8_t cycles = opcodes[code].Cycles;
    if (opcodes[code].PageBoundaryCondition && MemoryAddressResolveBase::PageBoundaryCrossed) {
        cycles++;
    }

    // add branch penalty
    if (BranchState::BranchTaken) {
        cycles++;
    }

    // TODO: add APU/PPU cpu delays
//    cycles += MemoryIO::cpuCyclesPenalty;

    // number of bytes read to execute opcode also counts as cycles
    numCycles += cycles;

    return cycles;
}

uint64_t
CPU::getCycleRuntime() {
    return numCycles;
}
