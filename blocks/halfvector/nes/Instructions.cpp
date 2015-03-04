#include <stdint.h>
#include "Instructions.h"
#include "Memory.h"
#include "CPU.h"
#include "OpcodeGenerator.h"

Instructions::Instructions(Opcode *opcodes) {
    this->opcodes = opcodes;
}

void
Instructions::execute(int opcode, InstructionContext *ctx) {
    assert(opcodes[opcode].execute != nullptr);
    opcodes[opcode].execute(ctx);
}

void
Instructions::initialize() {
    prepareAddressModes();

    // Unrecognized opcodes will take up 1 byte and 1 cycle
    for (uint8_t i = 0; i < 256; i++)
        opcodes[i].set("---", 1, 1, 0, "Unknown Opcode", ADDR_MODE_NONE, true);

    // Reference for instruction variants
    opcodes[0xE9].set("SBC", 2, 2, 0, "SBC #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0xE5].set("SBC", 2, 3, 0, "SBC nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xF5].set("SBC", 2, 4, 0, "SBC nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0xED].set("SBC", 3, 4, 0, "SBC nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xFD].set("SBC", 3, 4, 1, "SBC nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0xF9].set("SBC", 3, 4, 1, "SBC nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0xE1].set("SBC", 2, 6, 0, "SBC (nn,X)", ADDR_MODE_INDEXED_INDIRECT);
    opcodes[0xF1].set("SBC", 2, 5, 1, "SBC (nn),Y", ADDR_MODE_INDIRECT_INDEXED);
    opcodes[0xB0].set("BCS", 2, 2, 1, "BCS disp", ADDR_MODE_RELATIVE);
    opcodes[0x70].set("BVS", 2, 2, 1, "BVS disp", ADDR_MODE_RELATIVE);
    opcodes[0xD8].set("CLD", 1, 2, 0, "CLD", ADDR_MODE_NONE);
    opcodes[0xAA].set("TAX", 1, 2, 0, "TAX", ADDR_MODE_NONE);
    opcodes[0xB8].set("CLV", 1, 2, 0, "CLV", ADDR_MODE_NONE);
    opcodes[0x88].set("DEY", 1, 2, 0, "DEY", ADDR_MODE_NONE);
    opcodes[0xF8].set("SED", 1, 2, 0, "SED", ADDR_MODE_NONE);
    opcodes[0xC9].set("CMP", 2, 2, 0, "CMP #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0xC5].set("CMP", 2, 3, 0, "CMP nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xD5].set("CMP", 2, 4, 0, "CMP nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0xCD].set("CMP", 3, 4, 0, "CMP nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xDD].set("CMP", 3, 4, 1, "CMP nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0xD9].set("CMP", 3, 4, 1, "CMP nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0xC1].set("CMP", 2, 6, 0, "CMP (nn,X)", ADDR_MODE_INDEXED_INDIRECT);
    opcodes[0xD1].set("CMP", 2, 5, 1, "CMP (nn),Y", ADDR_MODE_INDIRECT_INDEXED);
    opcodes[0x30].set("BMI", 2, 2, 1, "BMI disp", ADDR_MODE_RELATIVE);
    opcodes[0x20].set("JSR", 3, 6, 0, "JSR nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xF0].set("BEQ", 2, 2, 1, "BEQ disp", ADDR_MODE_RELATIVE);
    opcodes[0xE6].set("INC", 2, 5, 0, "INC nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xF6].set("INC", 2, 6, 0, "INC nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0xEE].set("INC", 3, 6, 0, "INC nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xFE].set("INC", 3, 7, 0, "INC nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x98].set("TYA", 1, 2, 0, "TYA", ADDR_MODE_NONE);
    opcodes[0x29].set("AND", 2, 2, 0, "AND #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0x25].set("AND", 2, 3, 0, "AND nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x35].set("AND", 2, 4, 0, "AND nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x2D].set("AND", 3, 4, 0, "AND nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x3D].set("AND", 3, 4, 1, "AND nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x39].set("AND", 3, 4, 1, "AND nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0x21].set("AND", 2, 6, 0, "AND (nn,X)", ADDR_MODE_INDEXED_INDIRECT);
    opcodes[0x31].set("AND", 2, 5, 1, "AND (nn),Y", ADDR_MODE_INDIRECT_INDEXED);
    opcodes[0xD0].set("BNE", 2, 2, 1, "BNE disp", ADDR_MODE_RELATIVE);
    opcodes[0x68].set("PLA", 1, 4, 0, "PLA", ADDR_MODE_NONE);
    opcodes[0x60].set("RTS", 1, 6, 0, "RTS", ADDR_MODE_NONE);
    opcodes[0xCA].set("DEX", 1, 2, 0, "DEX", ADDR_MODE_NONE);
    opcodes[0x86].set("STX", 2, 3, 0, "STX nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x96].set("STX", 2, 4, 0, "STX nn,Y", ADDR_MODE_ZEROPAGE_INDEXED_Y);
    opcodes[0x8E].set("STX", 3, 4, 0, "STX nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x90].set("BCC", 2, 2, 1, "BCC disp", ADDR_MODE_RELATIVE);
    opcodes[0x6A].set("ROR", 1, 2, 0, "ROR A", ADDR_MODE_ACCUMULATOR);
    opcodes[0x66].set("ROR", 2, 5, 0, "ROR nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x76].set("ROR", 2, 6, 0, "ROR nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x6E].set("ROR", 3, 6, 0, "ROR nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x7E].set("ROR", 3, 7, 0, "ROR nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0xC0].set("CPY", 2, 2, 0, "CPY #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0xC4].set("CPY", 2, 3, 0, "CPY nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xCC].set("CPY", 3, 4, 0, "CPY nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x49].set("EOR", 2, 2, 0, "EOR #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0x45].set("EOR", 2, 3, 0, "EOR nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x55].set("EOR", 2, 4, 0, "EOR nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x4D].set("EOR", 3, 4, 0, "EOR nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x5D].set("EOR", 3, 4, 1, "EOR nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x59].set("EOR", 3, 4, 1, "EOR nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0x41].set("EOR", 2, 6, 0, "EOR (nn,X)", ADDR_MODE_INDEXED_INDIRECT);
    opcodes[0x51].set("EOR", 2, 5, 1, "EOR (nn),Y", ADDR_MODE_INDIRECT_INDEXED);
    opcodes[0x40].set("RTI", 1, 6, 0, "RTI", ADDR_MODE_NONE);
    opcodes[0x38].set("SEC", 1, 2, 0, "SEC", ADDR_MODE_NONE);
    opcodes[0xBA].set("TSX", 1, 2, 0, "TSX", ADDR_MODE_NONE);
    opcodes[0x09].set("ORA", 2, 2, 0, "ORA #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0x05].set("ORA", 2, 3, 0, "ORA nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x15].set("ORA", 2, 4, 0, "ORA nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x0D].set("ORA", 3, 4, 0, "ORA nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x1D].set("ORA", 3, 4, 1, "ORA nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x19].set("ORA", 3, 4, 1, "ORA nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0x01].set("ORA", 2, 6, 0, "ORA (nn,X)", ADDR_MODE_INDEXED_INDIRECT);
    opcodes[0x11].set("ORA", 2, 5, 1, "ORA (nn),Y", ADDR_MODE_INDIRECT_INDEXED);
    opcodes[0xC6].set("DEC", 2, 5, 0, "DEC nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xD6].set("DEC", 2, 6, 0, "DEC nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0xCE].set("DEC", 3, 6, 0, "DEC nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xDE].set("DEC", 3, 7, 0, "DEC nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x10].set("BPL", 2, 2, 1, "BPL disp", ADDR_MODE_RELATIVE);
    opcodes[0xC8].set("INY", 1, 2, 0, "INY", ADDR_MODE_NONE);
    opcodes[0x0A].set("ASL", 1, 2, 0, "ASL A", ADDR_MODE_ACCUMULATOR);
    opcodes[0x06].set("ASL", 2, 5, 0, "ASL nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x16].set("ASL", 2, 6, 0, "ASL nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x0E].set("ASL", 3, 6, 0, "ASL nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x1E].set("ASL", 3, 7, 0, "ASL nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x58].set("CLI", 1, 2, 0, "CLI", ADDR_MODE_NONE);
    opcodes[0xEA].set("NOP", 1, 2, 0, "NOP", ADDR_MODE_NONE);
    opcodes[0x4A].set("LSR", 1, 2, 0, "LSR A", ADDR_MODE_ACCUMULATOR);
    opcodes[0x46].set("LSR", 2, 5, 0, "LSR nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x56].set("LSR", 2, 6, 0, "LSR nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x4E].set("LSR", 3, 6, 0, "LSR nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x5E].set("LSR", 3, 7, 0, "LSR nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x24].set("BIT", 2, 3, 0, "BIT nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x2C].set("BIT", 3, 4, 0, "BIT nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x2A].set("ROL", 1, 2, 0, "ROL A", ADDR_MODE_ACCUMULATOR);
    opcodes[0x26].set("ROL", 2, 5, 0, "ROL nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x36].set("ROL", 2, 6, 0, "ROL nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x2E].set("ROL", 3, 6, 0, "ROL nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x3E].set("ROL", 3, 7, 0, "ROL nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0xA2].set("LDX", 2, 2, 0, "LDX #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0xA6].set("LDX", 2, 3, 0, "LDX nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xB6].set("LDX", 2, 4, 0, "LDX nn,Y", ADDR_MODE_ZEROPAGE_INDEXED_Y);
    opcodes[0xAE].set("LDX", 3, 4, 0, "LDX nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xBE].set("LDX", 3, 4, 1, "LDX nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0xE8].set("INX", 1, 2, 0, "INX", ADDR_MODE_NONE);
    opcodes[0x18].set("CLC", 1, 2, 0, "CLC", ADDR_MODE_NONE);
    opcodes[0x4C].set("JMP", 3, 3, 0, "JMP nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x6C].set("JMP", 3, 5, 0, "JMP (nnnn)", ADDR_MODE_INDIRECT_ABSOLUTE);
    opcodes[0x48].set("PHA", 1, 3, 0, "PHA", ADDR_MODE_NONE);
    opcodes[0x78].set("SEI", 1, 2, 0, "SEI", ADDR_MODE_NONE);
    opcodes[0x85].set("STA", 2, 3, 0, "STA nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x95].set("STA", 2, 4, 0, "STA nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x8D].set("STA", 3, 4, 0, "STA nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x9D].set("STA", 3, 5, 0, "STA nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x99].set("STA", 3, 5, 0, "STA nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0x81].set("STA", 2, 6, 0, "STA (nn,X)", ADDR_MODE_INDEXED_INDIRECT);
    opcodes[0x91].set("STA", 2, 6, 0, "STA (nn),Y", ADDR_MODE_INDIRECT_INDEXED);
    opcodes[0x84].set("STY", 2, 3, 0, "STY nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x94].set("STY", 2, 4, 0, "STY nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x8C].set("STY", 3, 4, 0, "STY nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x9A].set("TXS", 1, 2, 0, "TXS", ADDR_MODE_NONE);
    opcodes[0x69].set("ADC", 2, 2, 0, "ADC #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0x65].set("ADC", 2, 3, 0, "ADC nn", ADDR_MODE_ZEROPAGE);
    opcodes[0x75].set("ADC", 2, 4, 0, "ADC nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0x6D].set("ADC", 3, 4, 0, "ADC nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0x7D].set("ADC", 3, 4, 1, "ADC nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0x79].set("ADC", 3, 4, 1, "ADC nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0x61].set("ADC", 2, 6, 0, "ADC (nn,X)", ADDR_MODE_INDEXED_INDIRECT);
    opcodes[0x71].set("ADC", 2, 5, 1, "ADC (nn),Y", ADDR_MODE_INDIRECT_INDEXED);
    opcodes[0x00].set("BRK", 1, 7, 0, "BRK", ADDR_MODE_NONE);
    opcodes[0x50].set("BVC", 2, 2, 1, "BVC disp", ADDR_MODE_RELATIVE);
    opcodes[0x28].set("PLP", 1, 4, 0, "PLP", ADDR_MODE_NONE);
    opcodes[0xA8].set("TAY", 1, 2, 0, "TAY", ADDR_MODE_NONE);
    opcodes[0xE0].set("CPX", 2, 2, 0, "CPX #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0xE4].set("CPX", 2, 3, 0, "CPX nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xEC].set("CPX", 3, 4, 0, "CPX nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xA0].set("LDY", 2, 2, 0, "LDY #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0xA4].set("LDY", 2, 3, 0, "LDY nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xB4].set("LDY", 2, 4, 0, "LDY nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0xAC].set("LDY", 3, 4, 0, "LDY nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xBC].set("LDY", 3, 4, 1, "LDY nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0xA9].set("LDA", 2, 2, 0, "LDA #nn", ADDR_MODE_IMMEDIATE);
    opcodes[0xA5].set("LDA", 2, 3, 0, "LDA nn", ADDR_MODE_ZEROPAGE);
    opcodes[0xB5].set("LDA", 2, 4, 0, "LDA nn,X", ADDR_MODE_ZEROPAGE_INDEXED_X);
    opcodes[0xAD].set("LDA", 3, 4, 0, "LDA nnnn", ADDR_MODE_ABSOLUTE);
    opcodes[0xBD].set("LDA", 3, 4, 1, "LDA nnnn,X", ADDR_MODE_ABSOLUTE_INDEXED_X);
    opcodes[0xB9].set("LDA", 3, 4, 1, "LDA nnnn,Y", ADDR_MODE_ABSOLUTE_INDEXED_Y);
    opcodes[0xA1].set("LDA", 2, 6, 0, "LDA (nn,X)", ADDR_MODE_INDEXED_INDIRECT);
    opcodes[0xB1].set("LDA", 2, 5, 1, "LDA (nn),Y", ADDR_MODE_INDIRECT_INDEXED);
    opcodes[0x08].set("PHP", 1, 3, 0, "PHP", ADDR_MODE_NONE);
    opcodes[0x8A].set("TXA", 1, 2, 0, "TXA", ADDR_MODE_NONE);

    generateOpcodeVariants();

    PrintInfo("All opcodes generated");
}

/**
 * Initialize address-mode opcode offset, cycle count, and output format
 */
void Instructions::prepareAddressModes() {
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

    addressModes[ADDR_MODE_NONE].addressLine = "";
    addressModes[ADDR_MODE_ABSOLUTE].addressLine = "nnnn";
    addressModes[ADDR_MODE_IMMEDIATE].addressLine = "#nn";
    addressModes[ADDR_MODE_ZEROPAGE].addressLine = "nn";
    addressModes[ADDR_MODE_RELATIVE].addressLine = "disp";
    addressModes[ADDR_MODE_INDEXED_INDIRECT].addressLine = "(nn,X)";
    addressModes[ADDR_MODE_INDIRECT_INDEXED].addressLine = "(nn),Y";
    addressModes[ADDR_MODE_INDIRECT_ABSOLUTE].addressLine = "(nnnn)";
    addressModes[ADDR_MODE_ABSOLUTE_INDEXED_X].addressLine = "nnnn,X";
    addressModes[ADDR_MODE_ABSOLUTE_INDEXED_Y].addressLine = "nnnn,Y";
    addressModes[ADDR_MODE_ZEROPAGE_INDEXED_X].addressLine = "nn,X";
    addressModes[ADDR_MODE_ZEROPAGE_INDEXED_Y].addressLine = "nn,Y";
    addressModes[ADDR_MODE_ACCUMULATOR].addressLine = "A";
    addressModes[ADDR_MODE_IMMEDIATE_TO_XY].addressLine = "???";
}

/**
 * Instruction opcode-variance generator utilizing template specialization and template unrolling
 */
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

        //PrintInfo("  opcode=%i variant=%i offset=%i") % opcode % (int)opcodeVariant % (int)props[mode].offset;

        opcodes[opcodeVariant].execute = InstructionImplementation<opcode, mode>::execute;

        UnrollInstructions<opcode, static_cast<AddressMode>(mode - 1)>::unroll(opcodes, props);
    }
};

/**
 * Terminating condition for template unroll - reached the last address mode
 */
template<InstructionMnemonic opcode>
struct UnrollInstructions<opcode, ADDR_MODE_NONE> {
    static void unroll(Opcode *opcodes, AddressModeProperties *props) {
        PrintInfo("Unrolled all memory mode variants for opcode = 0x%02X") % opcode;
    }
};

/**
 * Calculate address-mode mask and kick-off instruction-variance generator via template unroll
 */
template<InstructionMnemonic opcode>
struct tAddressModeMask {
    static void Create(Opcode *opcodes, AddressModeProperties *props, int Mask1 = 0, int Mask2 = 0, int Mask3 = 0, int Mask4 = 0, int Mask5 = 0, int Mask6 = 0, int Mask7 = 0, int Mask8 = 0) {
        uint16_t mask = AddressModeMask[Mask1] + AddressModeMask[Mask2]
                + AddressModeMask[Mask3] + AddressModeMask[Mask4] + AddressModeMask[Mask5]
                + AddressModeMask[Mask6] + AddressModeMask[Mask7] + AddressModeMask[Mask8];

        //PrintInfo("  assigning mask=%d") % std::bitset<8>(mask).to_string('0', '1');

        opcodes[opcode].AddressModeMask = mask;

        UnrollInstructions<opcode, static_cast<AddressMode>(ADDRESS_MODE_SIZE-1)>::unroll(opcodes, props);
    }
};

/**
 * For each cpu instruction, generate its opcode variants given supported memory address modes
 */
void
Instructions::generateOpcodeVariants() {
    Unroll<ADC>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<AND>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<ASL>::start(opcodes, addressModes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<BCC>::start(opcodes, addressModes, ADDR_MODE_RELATIVE);
    Unroll<BCS>::start(opcodes, addressModes, ADDR_MODE_RELATIVE);
    Unroll<BEQ>::start(opcodes, addressModes, ADDR_MODE_RELATIVE);
    Unroll<BIT>::start(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    Unroll<BMI>::start(opcodes, addressModes, ADDR_MODE_RELATIVE);
    Unroll<BNE>::start(opcodes, addressModes, ADDR_MODE_RELATIVE);
    Unroll<BPL>::start(opcodes, addressModes, ADDR_MODE_RELATIVE);
    Unroll<BRK>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<BVC>::start(opcodes, addressModes, ADDR_MODE_RELATIVE);
    Unroll<BVS>::start(opcodes, addressModes, ADDR_MODE_RELATIVE);
    Unroll<CLC>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<CLD>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<CLI>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<CLV>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<CMP>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<CPX>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    Unroll<CPY>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    Unroll<DEC>::start(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<DEX>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<DEY>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<EOR>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<INC>::start(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<INX>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<INY>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<JMP>::start(opcodes, addressModes, ADDR_MODE_ABSOLUTE, ADDR_MODE_INDIRECT_ABSOLUTE);
    Unroll<JSR>::start(opcodes, addressModes, ADDR_MODE_ABSOLUTE);
    Unroll<LDA>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<LDX>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_Y, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_Y);
    Unroll<LDY>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<LSR>::start(opcodes, addressModes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<NOP>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<ORA>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<PHA>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<PHP>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<PLA>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<PLP>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<ROL>::start(opcodes, addressModes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<ROR>::start(opcodes, addressModes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<RTI>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<RTS>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<SBC>::start(opcodes, addressModes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<SEC>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<SED>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<SEI>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<STA>::start(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<STX>::start(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_Y, ADDR_MODE_ABSOLUTE);
    Unroll<STY>::start(opcodes, addressModes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE);
    Unroll<TAX>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<TAY>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<TSX>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<TXA>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<TXS>::start(opcodes, addressModes, ADDR_MODE_NONE);
    Unroll<TYA>::start(opcodes, addressModes, ADDR_MODE_NONE);
}

auto Instruction_SEI = [](AddressMode mode, InstructionContext *ctx) {
    //ctx->registers->P.I = 1;
};

auto Instruction_JMP = [](AddressMode mode, InstructionContext *ctx) {
    //ctx->registers->PC = tMemoryAddressLookup<AddressMode>::GetEffectiveAddress();
};

template<enum AddressMode mode>
struct InstructionImplementation<SEI, mode> {
    static void execute(InstructionContext *ctx) {
        PrintInfo("SEI!");
        ctx->registers->P.I = 1;
    }
};

//template<enum AddressMode mode>
//struct InstructionImplementation<PLA, mode> {
//    void execute(InstructionContext *ctx);
//};
//
//template<enum AddressMode mode>
//void InstructionImplementation<PLA, mode>::execute(InstructionContext *ctx) {
//    PrintInfo("PLA");
//}
//
//
//#define INSTRUCTION(opcode, method_impl) \
//    template<enum AddressMode mode> \
//    struct InstructionImplementation<opcode, mode> { \
//        static void execute(InstructionContext *ctx); \
//    }; \
//    \
//    template<enum AddressMode mode> \
//    void InstructionImplementation<opcode, mode>::execute(InstructionContext *ctx) \
//        method_impl \
//
//INSTRUCTION(CLI, {
//    PrintInfo("yey1");
//})
//
//INSTRUCTION(ORA, {
//    PrintInfo("yey2");
//})

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
