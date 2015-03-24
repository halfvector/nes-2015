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
    CPU(Registers*, Memory*);

    void load(Cartridge);
    void run();

    void reset();
    void executeOpcode(int code);

protected:
    Memory* memory;
    Registers* registers;
    Opcode opcodes[0x100];

    AddressModeProperties modes[16];
    Instructions* instructions;

    InstructionContext* ctx;

    bool cpuAlive = true;
    void writePrgPage(int i, uint8_t buffer[]);
    void writeChrPage(uint8_t buffer[]);
};

static const int RESET_VECTOR_ADDR = 0xFFFC;
static const int NMI_VECTOR_ADDR = 0xFFFA;