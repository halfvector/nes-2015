#include <stdint.h>
#include "Instructions.h"
#include "Memory.h"
#include "CPU.h"

Instructions::Instructions(Opcode *opcodes) {
    this->opcodes = opcodes;
}

#define SET_OPCODE_DATA(opcode, size, cycles, pbcondition, description, mnemonic, addrmode) \
    opcodes[opcode].set(mnemonic,size,cycles,pbcondition,description,addrmode);

void
Instructions::execute(int opcode, InstructionContext *ctx) {
    assert(opcodes[opcode].execute != nullptr);

    opcodes[opcode].execute(ctx);
}

void
Instructions::initialize() {
    prepareAddressModes();

    // unknown opcodes to take up 1 byte and 1 cycle
    for (unsigned int i = 0; i < 256; i++)
        opcodes[i].set("---", 1, 1, 0, "Unknown Opcode", ADDR_MODE_NONE, true);

    // 151 OpCodes
    SET_OPCODE_DATA(0xE9, 2, 2, 0, "SBC #nn", "SBC", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0xE5, 2, 3, 0, "SBC nn", "SBC", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xF5, 2, 4, 0, "SBC nn,X", "SBC", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0xED, 3, 4, 0, "SBC nnnn", "SBC", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xFD, 3, 4, 1, "SBC nnnn,X", "SBC", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0xF9, 3, 4, 1, "SBC nnnn,Y", "SBC", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0xE1, 2, 6, 0, "SBC (nn,X)", "SBC", ADDR_MODE_INDEXED_INDIRECT);
    SET_OPCODE_DATA(0xF1, 2, 5, 1, "SBC (nn),Y", "SBC", ADDR_MODE_INDIRECT_INDEXED);
    SET_OPCODE_DATA(0xB0, 2, 2, 1, "BCS disp", "BCS", ADDR_MODE_RELATIVE);
    SET_OPCODE_DATA(0x70, 2, 2, 1, "BVS disp", "BVS", ADDR_MODE_RELATIVE);
    SET_OPCODE_DATA(0xD8, 1, 2, 0, "CLD", "CLD", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xAA, 1, 2, 0, "TAX", "TAX", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xB8, 1, 2, 0, "CLV", "CLV", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x88, 1, 2, 0, "DEY", "DEY", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xF8, 1, 2, 0, "SED", "SED", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xC9, 2, 2, 0, "CMP #nn", "CMP", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0xC5, 2, 3, 0, "CMP nn", "CMP", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xD5, 2, 4, 0, "CMP nn,X", "CMP", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0xCD, 3, 4, 0, "CMP nnnn", "CMP", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xDD, 3, 4, 1, "CMP nnnn,X", "CMP", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0xD9, 3, 4, 1, "CMP nnnn,Y", "CMP", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0xC1, 2, 6, 0, "CMP (nn,X)", "CMP", ADDR_MODE_INDEXED_INDIRECT);
    SET_OPCODE_DATA(0xD1, 2, 5, 1, "CMP (nn),Y", "CMP", ADDR_MODE_INDIRECT_INDEXED);
    SET_OPCODE_DATA(0x30, 2, 2, 1, "BMI disp", "BMI", ADDR_MODE_RELATIVE);
    SET_OPCODE_DATA(0x20, 3, 6, 0, "JSR nnnn", "JSR", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xF0, 2, 2, 1, "BEQ disp", "BEQ", ADDR_MODE_RELATIVE);
    SET_OPCODE_DATA(0xE6, 2, 5, 0, "INC nn", "INC", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xF6, 2, 6, 0, "INC nn,X", "INC", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0xEE, 3, 6, 0, "INC nnnn", "INC", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xFE, 3, 7, 0, "INC nnnn,X", "INC", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x98, 1, 2, 0, "TYA", "TYA", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x29, 2, 2, 0, "AND #nn", "AND", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0x25, 2, 3, 0, "AND nn", "AND", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x35, 2, 4, 0, "AND nn,X", "AND", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x2D, 3, 4, 0, "AND nnnn", "AND", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x3D, 3, 4, 1, "AND nnnn,X", "AND", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x39, 3, 4, 1, "AND nnnn,Y", "AND", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0x21, 2, 6, 0, "AND (nn,X)", "AND", ADDR_MODE_INDEXED_INDIRECT);
    SET_OPCODE_DATA(0x31, 2, 5, 1, "AND (nn),Y", "AND", ADDR_MODE_INDIRECT_INDEXED);
    SET_OPCODE_DATA(0xD0, 2, 2, 1, "BNE disp", "BNE", ADDR_MODE_RELATIVE);
    SET_OPCODE_DATA(0x68, 1, 4, 0, "PLA", "PLA", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x60, 1, 6, 0, "RTS", "RTS", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xCA, 1, 2, 0, "DEX", "DEX", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x86, 2, 3, 0, "STX nn", "STX", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x96, 2, 4, 0, "STX nn,Y", "STX", ADDR_MODE_ZEROPAGE_INDEXED_Y);
    SET_OPCODE_DATA(0x8E, 3, 4, 0, "STX nnnn", "STX", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x90, 2, 2, 1, "BCC disp", "BCC", ADDR_MODE_RELATIVE);
    SET_OPCODE_DATA(0x6A, 1, 2, 0, "ROR A", "ROR", ADDR_MODE_ACCUMULATOR);
    SET_OPCODE_DATA(0x66, 2, 5, 0, "ROR nn", "ROR", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x76, 2, 6, 0, "ROR nn,X", "ROR", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x6E, 3, 6, 0, "ROR nnnn", "ROR", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x7E, 3, 7, 0, "ROR nnnn,X", "ROR", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0xC0, 2, 2, 0, "CPY #nn", "CPY", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0xC4, 2, 3, 0, "CPY nn", "CPY", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xCC, 3, 4, 0, "CPY nnnn", "CPY", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x49, 2, 2, 0, "EOR #nn", "EOR", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0x45, 2, 3, 0, "EOR nn", "EOR", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x55, 2, 4, 0, "EOR nn,X", "EOR", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x4D, 3, 4, 0, "EOR nnnn", "EOR", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x5D, 3, 4, 1, "EOR nnnn,X", "EOR", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x59, 3, 4, 1, "EOR nnnn,Y", "EOR", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0x41, 2, 6, 0, "EOR (nn,X)", "EOR", ADDR_MODE_INDEXED_INDIRECT);
    SET_OPCODE_DATA(0x51, 2, 5, 1, "EOR (nn),Y", "EOR", ADDR_MODE_INDIRECT_INDEXED);
    SET_OPCODE_DATA(0x40, 1, 6, 0, "RTI", "RTI", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x38, 1, 2, 0, "SEC", "SEC", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xBA, 1, 2, 0, "TSX", "TSX", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x09, 2, 2, 0, "ORA #nn", "ORA", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0x05, 2, 3, 0, "ORA nn", "ORA", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x15, 2, 4, 0, "ORA nn,X", "ORA", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x0D, 3, 4, 0, "ORA nnnn", "ORA", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x1D, 3, 4, 1, "ORA nnnn,X", "ORA", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x19, 3, 4, 1, "ORA nnnn,Y", "ORA", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0x01, 2, 6, 0, "ORA (nn,X)", "ORA", ADDR_MODE_INDEXED_INDIRECT);
    SET_OPCODE_DATA(0x11, 2, 5, 1, "ORA (nn),Y", "ORA", ADDR_MODE_INDIRECT_INDEXED);
    SET_OPCODE_DATA(0xC6, 2, 5, 0, "DEC nn", "DEC", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xD6, 2, 6, 0, "DEC nn,X", "DEC", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0xCE, 3, 6, 0, "DEC nnnn", "DEC", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xDE, 3, 7, 0, "DEC nnnn,X", "DEC", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x10, 2, 2, 1, "BPL disp", "BPL", ADDR_MODE_RELATIVE);
    SET_OPCODE_DATA(0xC8, 1, 2, 0, "INY", "INY", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x0A, 1, 2, 0, "ASL A", "ASL", ADDR_MODE_ACCUMULATOR);
    SET_OPCODE_DATA(0x06, 2, 5, 0, "ASL nn", "ASL", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x16, 2, 6, 0, "ASL nn,X", "ASL", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x0E, 3, 6, 0, "ASL nnnn", "ASL", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x1E, 3, 7, 0, "ASL nnnn,X", "ASL", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x58, 1, 2, 0, "CLI", "CLI", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xEA, 1, 2, 0, "NOP", "NOP", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x4A, 1, 2, 0, "LSR A", "LSR", ADDR_MODE_ACCUMULATOR);
    SET_OPCODE_DATA(0x46, 2, 5, 0, "LSR nn", "LSR", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x56, 2, 6, 0, "LSR nn,X", "LSR", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x4E, 3, 6, 0, "LSR nnnn", "LSR", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x5E, 3, 7, 0, "LSR nnnn,X", "LSR", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x24, 2, 3, 0, "BIT nn", "BIT", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x2C, 3, 4, 0, "BIT nnnn", "BIT", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x2A, 1, 2, 0, "ROL A", "ROL", ADDR_MODE_ACCUMULATOR);
    SET_OPCODE_DATA(0x26, 2, 5, 0, "ROL nn", "ROL", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x36, 2, 6, 0, "ROL nn,X", "ROL", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x2E, 3, 6, 0, "ROL nnnn", "ROL", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x3E, 3, 7, 0, "ROL nnnn,X", "ROL", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0xA2, 2, 2, 0, "LDX #nn", "LDX", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0xA6, 2, 3, 0, "LDX nn", "LDX", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xB6, 2, 4, 0, "LDX nn,Y", "LDX", ADDR_MODE_ZEROPAGE_INDEXED_Y);
    SET_OPCODE_DATA(0xAE, 3, 4, 0, "LDX nnnn", "LDX", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xBE, 3, 4, 1, "LDX nnnn,Y", "LDX", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0xE8, 1, 2, 0, "INX", "INX", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x18, 1, 2, 0, "CLC", "CLC", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x4C, 3, 3, 0, "JMP nnnn", "JMP", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x6C, 3, 5, 0, "JMP (nnnn)", "JMP", ADDR_MODE_INDIRECT_ABSOLUTE);
    SET_OPCODE_DATA(0x48, 1, 3, 0, "PHA", "PHA", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x78, 1, 2, 0, "SEI", "SEI", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x85, 2, 3, 0, "STA nn", "STA", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x95, 2, 4, 0, "STA nn,X", "STA", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x8D, 3, 4, 0, "STA nnnn", "STA", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x9D, 3, 5, 0, "STA nnnn,X", "STA", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x99, 3, 5, 0, "STA nnnn,Y", "STA", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0x81, 2, 6, 0, "STA (nn,X)", "STA", ADDR_MODE_INDEXED_INDIRECT);
    SET_OPCODE_DATA(0x91, 2, 6, 0, "STA (nn),Y", "STA", ADDR_MODE_INDIRECT_INDEXED);
    SET_OPCODE_DATA(0x84, 2, 3, 0, "STY nn", "STY", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x94, 2, 4, 0, "STY nn,X", "STY", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x8C, 3, 4, 0, "STY nnnn", "STY", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x9A, 1, 2, 0, "TXS", "TXS", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x69, 2, 2, 0, "ADC #nn", "ADC", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0x65, 2, 3, 0, "ADC nn", "ADC", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0x75, 2, 4, 0, "ADC nn,X", "ADC", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0x6D, 3, 4, 0, "ADC nnnn", "ADC", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0x7D, 3, 4, 1, "ADC nnnn,X", "ADC", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0x79, 3, 4, 1, "ADC nnnn,Y", "ADC", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0x61, 2, 6, 0, "ADC (nn,X)", "ADC", ADDR_MODE_INDEXED_INDIRECT);
    SET_OPCODE_DATA(0x71, 2, 5, 1, "ADC (nn),Y", "ADC", ADDR_MODE_INDIRECT_INDEXED);
    SET_OPCODE_DATA(0x00, 1, 7, 0, "BRK", "BRK", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x50, 2, 2, 1, "BVC disp", "BVC", ADDR_MODE_RELATIVE);
    SET_OPCODE_DATA(0x28, 1, 4, 0, "PLP", "PLP", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xA8, 1, 2, 0, "TAY", "TAY", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0xE0, 2, 2, 0, "CPX #nn", "CPX", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0xE4, 2, 3, 0, "CPX nn", "CPX", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xEC, 3, 4, 0, "CPX nnnn", "CPX", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xA0, 2, 2, 0, "LDY #nn", "LDY", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0xA4, 2, 3, 0, "LDY nn", "LDY", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xB4, 2, 4, 0, "LDY nn,X", "LDY", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0xAC, 3, 4, 0, "LDY nnnn", "LDY", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xBC, 3, 4, 1, "LDY nnnn,X", "LDY", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0xA9, 2, 2, 0, "LDA #nn", "LDA", ADDR_MODE_IMMEDIATE);
    SET_OPCODE_DATA(0xA5, 2, 3, 0, "LDA nn", "LDA", ADDR_MODE_ZEROPAGE);
    SET_OPCODE_DATA(0xB5, 2, 4, 0, "LDA nn,X", "LDA", ADDR_MODE_ZEROPAGE_INDEXED_X);
    SET_OPCODE_DATA(0xAD, 3, 4, 0, "LDA nnnn", "LDA", ADDR_MODE_ABSOLUTE);
    SET_OPCODE_DATA(0xBD, 3, 4, 1, "LDA nnnn,X", "LDA", ADDR_MODE_ABSOLUTE_INDEXED_X);
    SET_OPCODE_DATA(0xB9, 3, 4, 1, "LDA nnnn,Y", "LDA", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    SET_OPCODE_DATA(0xA1, 2, 6, 0, "LDA (nn,X)", "LDA", ADDR_MODE_INDEXED_INDIRECT);
    SET_OPCODE_DATA(0xB1, 2, 5, 1, "LDA (nn),Y", "LDA", ADDR_MODE_INDIRECT_INDEXED);
    SET_OPCODE_DATA(0x08, 1, 3, 0, "PHP", "PHP", ADDR_MODE_NONE);
    SET_OPCODE_DATA(0x8A, 1, 2, 0, "TXA", "TXA", ADDR_MODE_NONE);

    assignAddressModes();

    PrintInfo("All OpCodes Initialized");
}

void Instructions::prepareAddressModes() {// initialize address mode offsets
    for (int i = 0; i < 16; i++) {
        addressModes[i].offset = 0;
        addressModes[i].cycles = 0;
    }

    addressModes[ADDR_MODE_ACCUMULATOR].offset = -0x04;
    addressModes[ADDR_MODE_ABSOLUTE].offset = +0x00;
    addressModes[ADDR_MODE_IMMEDIATE].offset = -0x04;
    addressModes[ADDR_MODE_IMMEDIATE_TO_XY].offset = -0x0C;
    addressModes[ADDR_MODE_ZEROPAGE].offset = -0x08;
    addressModes[ADDR_MODE_ZEROPAGE_INDEXED_X].offset = +0x08;
    addressModes[ADDR_MODE_ZEROPAGE_INDEXED_Y].offset = +0x08;
    addressModes[ADDR_MODE_ABSOLUTE_INDEXED_X].offset = +0x10;
    addressModes[ADDR_MODE_ABSOLUTE_INDEXED_Y].offset = +0x0C;
    addressModes[ADDR_MODE_INDEXED_INDIRECT].offset = -0x0C;
    addressModes[ADDR_MODE_INDIRECT_INDEXED].offset = +0x04;
    addressModes[ADDR_MODE_INDIRECT_ABSOLUTE].offset = +0x20;

    // these address modes take a different number of cycles
    addressModes[ADDR_MODE_IMMEDIATE].cycles = -2;
    addressModes[ADDR_MODE_IMMEDIATE_TO_XY].cycles = -2;
    addressModes[ADDR_MODE_ZEROPAGE].cycles = -1;
    addressModes[ADDR_MODE_INDIRECT_INDEXED].cycles = +1;
    addressModes[ADDR_MODE_RELATIVE].cycles = +0;
    addressModes[ADDR_MODE_INDEXED_INDIRECT].cycles = +2;
    addressModes[ADDR_MODE_INDIRECT_ABSOLUTE].cycles = +2;
}

template<InstructionMnemonic opcode, enum AddressMode mode>
struct UnrollInstructions {
    static void unroll(Opcode *opcodes, AddressModeProperties *props) {
        // calculate opcode variant given a memory address mode
        uint8_t opcodeVariant = opcode + props[mode].offset;

        // there are some exceptions to the formula above.
        // * LDX with absolute indexed Y register address mode uses +0x10 offset instead of +0xC

        if (opcode == LDX && mode == ADDR_MODE_ABSOLUTE_INDEXED_Y) {
            opcodeVariant = opcode + uint8_t(0x10); // 0xBE
        }

        PrintInfo("  opcode=%i variant=%i offset=%i") % opcode % (int)opcodeVariant % (int)props[mode].offset;

        opcodes[opcodeVariant].execute = InstructionImplementation<opcode, mode>::instructionImplementation;

        UnrollInstructions<opcode, static_cast<AddressMode>(mode - 1)>::unroll(opcodes, props);
    }
};

// terminating condition for specialized template unroll
template<InstructionMnemonic opcode>
struct UnrollInstructions<opcode, ADDR_MODE_NONE> {
    static void unroll(Opcode *opcodes, AddressModeProperties *props) {
        PrintInfo("Unroll completed for opcode=%i") % opcode;
    }
};

template<InstructionMnemonic opcode>
struct tAddressModeMask {
    static void Create(Opcode *opcodes, AddressModeProperties *props, int Mask1 = 0, int Mask2 = 0, int Mask3 = 0, int Mask4 = 0, int Mask5 = 0, int Mask6 = 0, int Mask7 = 0, int Mask8 = 0) {
        uint16_t mask = AddressModeMask[Mask1] + AddressModeMask[Mask2]
                + AddressModeMask[Mask3] + AddressModeMask[Mask4] + AddressModeMask[Mask5]
                + AddressModeMask[Mask6] + AddressModeMask[Mask7] + AddressModeMask[Mask8];

        PrintInfo("assigning mask=%d") % std::bitset<8>(mask).to_string('0', '1');

        opcodes[opcode].AddressModeMask = mask;

        UnrollInstructions<opcode, static_cast<AddressMode>(ADDRESS_MODE_SIZE-1)>::unroll(opcodes, props);
    }
};

void
Instructions::assignAddressModes() {

    // generate opcode-variances for each address mode

    tAddressModeMask<ADC>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    tAddressModeMask<AND>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    tAddressModeMask<ASL>::Create(opcodes, addressModes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    tAddressModeMask<BCC>::Create(opcodes, addressModes, ADDR_MODE_RELATIVE);
    tAddressModeMask<BCS>::Create(opcodes, addressModes, ADDR_MODE_RELATIVE);
    tAddressModeMask<BEQ>::Create(opcodes, addressModes, ADDR_MODE_RELATIVE);
    tAddressModeMask<BIT>::Create(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    tAddressModeMask<BMI>::Create(opcodes, addressModes, ADDR_MODE_RELATIVE);
    tAddressModeMask<BNE>::Create(opcodes, addressModes, ADDR_MODE_RELATIVE);
    tAddressModeMask<BPL>::Create(opcodes, addressModes, ADDR_MODE_RELATIVE);
    tAddressModeMask<BRK>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<BVC>::Create(opcodes, addressModes, ADDR_MODE_RELATIVE);
    tAddressModeMask<BVS>::Create(opcodes, addressModes, ADDR_MODE_RELATIVE);
    tAddressModeMask<CLC>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<CLD>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<CLI>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<CLV>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<CMP>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    tAddressModeMask<CPX>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    tAddressModeMask<CPY>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    tAddressModeMask<DEC>::Create(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    tAddressModeMask<DEX>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<DEY>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<EOR>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    tAddressModeMask<INC>::Create(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    tAddressModeMask<INX>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<INY>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<JMP>::Create(opcodes, addressModes, ADDR_MODE_ABSOLUTE, ADDR_MODE_INDIRECT_ABSOLUTE);
    tAddressModeMask<JSR>::Create(opcodes, addressModes, ADDR_MODE_ABSOLUTE);
    tAddressModeMask<LDA>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    tAddressModeMask<LDX>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_Y, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_Y);
    tAddressModeMask<LDY>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    tAddressModeMask<LSR>::Create(opcodes, addressModes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    tAddressModeMask<NOP>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<ORA>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    tAddressModeMask<PHA>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<PHP>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<PLA>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<PLP>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<ROL>::Create(opcodes, addressModes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    tAddressModeMask<ROR>::Create(opcodes, addressModes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    tAddressModeMask<RTI>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<RTS>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<SBC>::Create(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    tAddressModeMask<SEC>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<SED>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<SEI>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<STA>::Create(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    tAddressModeMask<STX>::Create(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_Y, ADDR_MODE_ABSOLUTE);
    tAddressModeMask<STY>::Create(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE);
    tAddressModeMask<TAX>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<TAY>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<TSX>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<TXA>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<TXS>::Create(opcodes, addressModes, ADDR_MODE_NONE);
    tAddressModeMask<TYA>::Create(opcodes, addressModes, ADDR_MODE_NONE);
}

void
Instructions::applyAddressModes(int opcode, int Mask1, int Mask2, int Mask3, int Mask4, int Mask5, int Mask6, int Mask7, int Mask8) {
    unsigned short mask = AddressModeMask[Mask1] + AddressModeMask[Mask2]
            + AddressModeMask[Mask3] + AddressModeMask[Mask4] + AddressModeMask[Mask5]
            + AddressModeMask[Mask6] + AddressModeMask[Mask7] + AddressModeMask[Mask8];

    opcodes[opcode].AddressModeMask = mask;

    applyAddressModeMask(opcode, mask);

}


void
Instructions::applyAddressModeMask(uint16_t opcode, uint16_t mask) {
    for (int i = 0; i < ADDRESS_MODE_SIZE; i++) {
        if (mask & AddressModeMask[i]) {
            // opcode supports this address mode
            unsigned short opcodeVariant = opcode + addressModes[opcode].offset;

            // exceptions:
            // LDX with absolute indexed Y register address mode uses +0x10 offset instead of +0xC

            if (opcode == LDX && i == ADDR_MODE_ABSOLUTE_INDEXED_Y) {
                opcodeVariant = opcode + uint16_t(0x10); // 0xBE
            }

            //opcodes[opcodeVariant].execute = tInstructionLookup<opcode>::Instruction<i>::Execute;

            // register opcode execution
            // it will be responsible for updating cpu registers and flags

//            opcodes[opcodeVariant].execute =
//
//            CPU_Instructions::Singleton()->AddInstruction( OpCodeVariant, tInstructionLookup< Mnemonic >::Instruction<AddressMode>::Execute );
//
//            int VariantCycles = CPU_Instructions::Singleton()->OpcodeData[Mnemonic].Cycles + CPU_Instructions::Singleton()->m_addressModes[AddressMode];
//            // accumulator instructions are always 2 cycles long
//            if( AddressMode == ADDR_MODE_ACCUMULATOR )
//                VariantCycles = 2;
//
//            int OldCycles = CPU_Instructions::Singleton()->OpcodeData[OpCodeVariant].Cycles;
        }

//        if( Mask )
//        {
//            tUnrollOpCodeModes<Mnemonic>::Helper<(eAddressMode) (AddressMode-1)>::Unroll();
//        } else
//        {	// no address modes supported
//            CPU_Instructions::Singleton()->AddInstruction( Mnemonic, tInstructionLookup< Mnemonic >::Instruction<ADDR_MODE_NONE>::Execute );
//        }
    }
}


auto Instruction_SEI = [](AddressMode mode, InstructionContext *ctx) {
    //ctx->registers->P.I = 1;
};

auto Instruction_JMP = [](AddressMode mode, InstructionContext *ctx) {
    //ctx->registers->PC = tMemoryAddressLookup<AddressMode>::GetEffectiveAddress();
};

template<enum AddressMode mode>
struct InstructionImplementation<SEI, mode> {
    static void instructionImplementation(InstructionContext *ctx) {

    }
};


//DEFINE_INSTRUCTION(SEI) {
//    PrintInfo("Executing SEI!");
//}

//
//// shortcut macro:
//#define DECLARE_INSTRUCTION(opcode) \
//    template<enum AddressMode mode> void Instruction<mode, SEI>::execute(InstructionContext* ctx)
//
//// partial specialization macro usage:
//DECLARE_INSTRUCTION(SEI) {
//    ctx->registers->P.I = 1;
//}
//
//// execute partial specialized method:
//void testInstructions() {
//    Instruction<ADDR_MODE_ABSOLUTE, SEI>::execute(nullptr);
//}
//
//struct Instruction<SEI> {
//    static void Execute(Memory* mem, Registers* reg) {
//        reg->P.I = 1;
//    }
//};
