#pragma once

#include <cstdint>
#include "Cartridge.h"
#include "Logging.h"
#include "Memory.h"
#include "Platform.h"
#include "Instructions.h"
#include "Registers.h"

class CPU {
public:
    CPU(Memory*);

    void load(Cartridge);
    void run();

protected:
    Memory* memory;
    Opcode opcodes[0x100];
    AddressModeProperties modes[16];

    Instructions* instructions;
    InstructionContext* ctx;

    Registers registers;
    bool cpuAlive = true;

    void writePrgPage(int i, uint8_t buffer[]);
    void writeChrPage(uint8_t buffer[]);
    void reset();
    void executeOpcode(int code);
};

static const int RESET_VECTOR_ADDR = 0xFFFC;
static const int NMI_VECTOR_ADDR = 0xFFFA;