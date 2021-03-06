#pragma once

#include "Memory.h"
#include "Logging.h"
#include "Registers.h"
#include "MemoryLookup.h"

struct AddressModeProperties {
    int8_t offset;
    int8_t cycles;
    const char* addressLine;
};

enum InstructionMnemonic {
    ADC = 0x6D, AND = 0x2D, ASL = 0x0E, BCC = 0x90, BCS = 0xB0, BEQ = 0xF0, BIT = 0x2C,
    BMI = 0x30, BNE = 0xD0, BPL = 0x10, BRK = 0x00, BVC = 0x50, BVS = 0x70, CLC = 0x18,
    CLD = 0xD8, CLI = 0x58, CLV = 0xB8, CMP = 0xCD, CPX = 0xEC, CPY = 0xCC, DEC = 0xCE,
    DEX = 0xCA, DEY = 0x88, EOR = 0x4D, INC = 0xEE, INX = 0xE8, INY = 0xC8, JMP = 0x4C,
    JSR = 0x20, LDA = 0xAD, LDX = 0xAE, LDY = 0xAC, LSR = 0x4E, NOP = 0xEA, ORA = 0x0D,
    PHA = 0x48, PHP = 0x08, PLA = 0x68, PLP = 0x28, ROL = 0x2E, ROR = 0x6E, RTI = 0x40,
    RTS = 0x60, SBC = 0xED, SEC = 0x38, SED = 0xF8, SEI = 0x78, STA = 0x8D, STX = 0x8E,
    STY = 0x8C, TAX = 0xAA, TAY = 0xA8, TSX = 0xBA, TXA = 0x8A, TXS = 0x9A, TYA = 0x98,

    // Extra opcodes
    LAX = 0xAF, SAX = 0x8F, SHY = 0x8C
};

template<ProcessorStatusFlags Register>
struct BranchIf {
    static void is(InstructionContext *ctx, bool expectedState);
    static bool BranchTaken;
};

struct BranchState {
    static bool BranchTaken;
};

struct tInstructionBase {
    InstructionMnemonic opcode;
};

template<uint8_t opcode, enum AddressMode mode>
struct InstructionImplementationA {
    static void execute(InstructionContext *ctx) {
        PrintError("InstructionImplementationA::execute(); Unimplemented opcode = %02X in address mode = %s",
                   (int) opcode, AddressModeTitle[static_cast<int>(mode)]);
        throw std::runtime_error("Unimplemented opcode");
    }
};

typedef tCPU::word (*MemoryResolver)(InstructionContext *ctx);

template<uint8_t opcode, AddressMode mode>
struct InstructionImplementationX {
    static void execute(InstructionContext *ctx, MemoryResolver resolver) {
        PrintError("Unimplemented opcode = %02X", (int) opcode);
        throw std::runtime_error("Unimplemented opcode");
    }
};

template<uint8_t opcode, enum AddressMode mode>
struct InstructionImplementation {
    static void execute(InstructionContext *ctx) {
        InstructionImplementationX<opcode, mode>::execute(ctx, &MemoryAddressResolve<mode>::GetEffectiveAddress);
    }
};

/**
 * CPU Instruction and all its dirty details
 */
struct Opcode {
    unsigned char Bytes;
    unsigned char Cycles;
    bool PageBoundaryCondition;    // add another cycle on page boundary cross
    const char *Description;
    const char *Mnemonic;
    enum AddressMode AddressMode;
    bool Invalid;
    unsigned short AddressModeMask = 0;

    uint8_t opcode;

    // typedef void (tInstructionBase::*methodPtr)(InstructionContext*);
    typedef void (*methodPtr)(InstructionContext *);

    methodPtr execute = nullptr;

    void set(const char *mnemonic, unsigned char bytes, unsigned char cycles, bool PBC,
             const char *description, enum AddressMode addressMode, bool invalid = false) {
        Mnemonic = mnemonic;
        Bytes = bytes;
        Cycles = cycles;
        Description = description;
        PageBoundaryCondition = PBC;
        AddressMode = addressMode;
        Invalid = invalid;
    }
};

#define DEFINE_INSTRUCTION(opcode) \
    template<enum AddressMode mode> void \
    InstructionImplementation<opcode, mode>::execute(InstructionContext *ctx) \


class Instructions {
public:

    Instructions(Opcode *opcodes, AddressModeProperties *modes);

    void initialize();

    void execute(int opcode, InstructionContext *ctx);

protected:

    Opcode *opcodes;
    AddressModeProperties *modes;

    void configureOpcodes();

    void configureMemoryAddressModes();

    void generateOpcodeVariants();
};
