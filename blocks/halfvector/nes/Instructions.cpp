#include <stdint.h>
#include "Instructions.h"

Instructions::Instructions(Opcode *opcodes, AddressModeProperties* modes) {
    this->opcodes = opcodes;
    this->modes = modes;
}

void
Instructions::initialize() {
    configureMemoryAddressModes();
    configureOpcodes();
    generateOpcodeVariants();

    PrintInfo("All opcodes generated");
}

void
Instructions::execute(int opcode, InstructionContext *ctx) {
    assert(opcodes[opcode].execute != nullptr);
    opcodes[opcode].execute(ctx);
}

/**
 * Initialize address-mode opcode offset, cycle count, and output format
 */
void Instructions::configureMemoryAddressModes() {
    for (int i = 0; i < 16; i++) {
        modes[i].offset = 0;
        modes[i].cycles = 0;
    }

    modes[ADDR_MODE_ACCUMULATOR].offset = -0x04;
    modes[ADDR_MODE_ABSOLUTE].offset = +0x00;
    modes[ADDR_MODE_IMMEDIATE].offset = -0x04;
    modes[ADDR_MODE_IMMEDIATE_TO_XY].offset = -0x0C;
    modes[ADDR_MODE_ZEROPAGE].offset = -0x08;
    modes[ADDR_MODE_ZEROPAGE_INDEXED_X].offset = +0x08;
    modes[ADDR_MODE_ZEROPAGE_INDEXED_Y].offset = +0x08;
    modes[ADDR_MODE_ABSOLUTE_INDEXED_X].offset = +0x10;
    modes[ADDR_MODE_ABSOLUTE_INDEXED_Y].offset = +0x0C;
    modes[ADDR_MODE_INDEXED_INDIRECT].offset = -0x0C;
    modes[ADDR_MODE_INDIRECT_INDEXED].offset = +0x04;
    modes[ADDR_MODE_INDIRECT_ABSOLUTE].offset = +0x20;

    // these address modes take a different number of cycles
    modes[ADDR_MODE_IMMEDIATE].cycles = -2;
    modes[ADDR_MODE_IMMEDIATE_TO_XY].cycles = -2;
    modes[ADDR_MODE_ZEROPAGE].cycles = -1;
    modes[ADDR_MODE_INDIRECT_INDEXED].cycles = +1;
    modes[ADDR_MODE_RELATIVE].cycles = +0;
    modes[ADDR_MODE_INDEXED_INDIRECT].cycles = +2;
    modes[ADDR_MODE_INDIRECT_ABSOLUTE].cycles = +2;

    modes[ADDR_MODE_NONE].addressLine = "";
    modes[ADDR_MODE_ABSOLUTE].addressLine = "$%02X%02X"; // "nnnn";
    modes[ADDR_MODE_IMMEDIATE].addressLine = "#$%02X"; // "#nn";
    modes[ADDR_MODE_ZEROPAGE].addressLine = "$%02X"; // "nn";
    modes[ADDR_MODE_RELATIVE].addressLine = "$%08X"; // "disp";
    modes[ADDR_MODE_INDEXED_INDIRECT].addressLine = "($%02X,X)"; // "(nn,X)";
    modes[ADDR_MODE_INDIRECT_INDEXED].addressLine = "($%02X),Y"; // "(nn),Y";
    modes[ADDR_MODE_INDIRECT_ABSOLUTE].addressLine = "($%02X%02X)"; // "(nnnn)";
    modes[ADDR_MODE_ABSOLUTE_INDEXED_X].addressLine = "$%02X%02X,X"; // "nnnn,X";
    modes[ADDR_MODE_ABSOLUTE_INDEXED_Y].addressLine = "$%02X%02X,Y"; // "nnnn,Y";
    modes[ADDR_MODE_ZEROPAGE_INDEXED_X].addressLine = "$%02X,X"; // "nn,X";
    modes[ADDR_MODE_ZEROPAGE_INDEXED_Y].addressLine = "$%02X,Y"; // "nn,Y";
    modes[ADDR_MODE_ACCUMULATOR].addressLine = "A";
    modes[ADDR_MODE_IMMEDIATE_TO_XY].addressLine = "???";
}

/**
* Configure cycle count, byte count, output format, and memory mode for all opcode variants
*/
void Instructions::configureOpcodes() {
    for (uint8_t i = 0; i < 255; i++)
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
}

/**
 * Per-memory-address opcode-variant generator
 */

template<InstructionMnemonic opcode, enum AddressMode mode>
struct UnrollInstructions {
    static void unroll(Opcode *opcodes, AddressModeProperties *props) {
        // Validate address-mode is supported by instruction
        if((opcodes[opcode].AddressModeMask & AddressModeMask[mode]) == AddressModeMask[mode]) {
            // Calculate opcode variant given a memory address mode
            uint8_t opcodeVariant = opcode + props[mode].offset;

            // There are some exceptions to the formulaic variant generation:
            // + LDX with absolute indexed Y register address mode uses +0x10 offset instead of +0xC
            if (opcode == LDX && mode == ADDR_MODE_ABSOLUTE_INDEXED_Y) {
                opcodeVariant = uint8_t(opcode + 0x10); // 0xBE
            }

//            PrintInfo("  + opcode=0x%02X variant=0x%02X offset=%d mode=%d mask=%d")
//                % (int) opcode % (int) opcodeVariant % (int) props[mode].offset
//                % (int) mode % (int) AddressModeMask[mode];
            assert(opcodes[opcodeVariant].execute == nullptr);
            opcodes[opcodeVariant].execute = &InstructionImplementation<opcode, mode>::execute;
        }

        UnrollInstructions<opcode, static_cast<AddressMode>(mode-1)>::unroll(opcodes, props);
    }
};

/**
 * Exit condition for template unroll
 */
template<InstructionMnemonic opcode>
struct UnrollInstructions<opcode, ADDR_MODE_NONE> {
    static void unroll(Opcode *opcodes, AddressModeProperties *props) {

        // Validate address-mode is supported by instruction
        if(opcodes[opcode].AddressModeMask == AddressModeMask[ADDR_MODE_NONE]) {
            // Calculate opcode variant given a memory address mode
            uint8_t opcodeVariant = opcode + props[ADDR_MODE_NONE].offset;

//            PrintInfo("  + opcode=0x%02X variant=0x%02X offset=%d mode=%d mask=%d")
//                    % (int) opcode % (int) opcodeVariant % (int) props[ADDR_MODE_NONE].offset
//                    % (int) ADDR_MODE_NONE % (int) AddressModeMask[ADDR_MODE_NONE];
            assert(opcodes[opcodeVariant].execute == nullptr);
            opcodes[opcodeVariant].execute = &InstructionImplementation<opcode, ADDR_MODE_NONE>::execute;
        }

//        PrintInfo("  -> Unroll completed for opcode=0x%02X") % (int)opcode;

        // is this right?
        // it overwrites immediate mode (AddressMode=1) variant
//        opcodes[opcode].execute = &InstructionImplementation<opcode, ADDR_MODE_NONE>::execute;
    }
};

/**
 * Entry-point into the template unroll
 */
template<InstructionMnemonic opcode>
struct Unroll {
    static void start(Opcode *opcodes, AddressModeProperties *props, int mask1 = 0, int mask2 = 0, int mask3 = 0, int mask4 = 0, int mask5 = 0, int mask6 = 0, int mask7 = 0, int mask8 = 0) {
        uint16_t mask = AddressModeMask[mask1] + AddressModeMask[mask2]
                + AddressModeMask[mask3] + AddressModeMask[mask4] + AddressModeMask[mask5]
                + AddressModeMask[mask6] + AddressModeMask[mask7] + AddressModeMask[mask8];

//        PrintInfo("assigning mask=%d") % std::bitset<8>(mask).to_string('0', '1');

        opcodes[opcode].AddressModeMask = mask;

        UnrollInstructions<opcode, static_cast<AddressMode>(ADDR_MODE_LAST - 1)>::unroll(opcodes, props);
    }
};

/**
 * For each cpu instruction, generate its opcode variants given a set of memory address modes
 */
void
Instructions::generateOpcodeVariants() {
    Unroll<ADC>::start(opcodes, modes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<AND>::start(opcodes, modes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<ASL>::start(opcodes, modes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<BCC>::start(opcodes, modes, ADDR_MODE_RELATIVE);
    Unroll<BCS>::start(opcodes, modes, ADDR_MODE_RELATIVE);
    Unroll<BEQ>::start(opcodes, modes, ADDR_MODE_RELATIVE);
    Unroll<BIT>::start(opcodes, modes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    Unroll<BMI>::start(opcodes, modes, ADDR_MODE_RELATIVE);
    Unroll<BNE>::start(opcodes, modes, ADDR_MODE_RELATIVE);
    Unroll<BPL>::start(opcodes, modes, ADDR_MODE_RELATIVE);
    Unroll<BRK>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<BVC>::start(opcodes, modes, ADDR_MODE_RELATIVE);
    Unroll<BVS>::start(opcodes, modes, ADDR_MODE_RELATIVE);
    Unroll<CLC>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<CLD>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<CLI>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<CLV>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<CMP>::start(opcodes, modes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<CPX>::start(opcodes, modes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    Unroll<CPY>::start(opcodes, modes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ABSOLUTE);
    Unroll<DEC>::start(opcodes, modes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<DEX>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<DEY>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<EOR>::start(opcodes, modes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<INC>::start(opcodes, modes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<INX>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<INY>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<JMP>::start(opcodes, modes, ADDR_MODE_ABSOLUTE, ADDR_MODE_INDIRECT_ABSOLUTE);
    Unroll<JSR>::start(opcodes, modes, ADDR_MODE_ABSOLUTE);
    Unroll<LDA>::start(opcodes, modes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<LDX>::start(opcodes, modes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_Y, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_Y);
    Unroll<LDY>::start(opcodes, modes, ADDR_MODE_IMMEDIATE_TO_XY, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<LSR>::start(opcodes, modes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<NOP>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<ORA>::start(opcodes, modes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<PHA>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<PHP>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<PLA>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<PLP>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<ROL>::start(opcodes, modes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<ROR>::start(opcodes, modes, ADDR_MODE_ACCUMULATOR, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X);
    Unroll<RTI>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<RTS>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<SBC>::start(opcodes, modes, ADDR_MODE_IMMEDIATE, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<SEC>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<SED>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<SEI>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<STA>::start(opcodes, modes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE, ADDR_MODE_ABSOLUTE_INDEXED_X, ADDR_MODE_ABSOLUTE_INDEXED_Y, ADDR_MODE_INDEXED_INDIRECT, ADDR_MODE_INDIRECT_INDEXED);
    Unroll<STX>::start(opcodes, modes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_Y, ADDR_MODE_ABSOLUTE);
    Unroll<STY>::start(opcodes, modes, ADDR_MODE_ZEROPAGE, ADDR_MODE_ZEROPAGE_INDEXED_X, ADDR_MODE_ABSOLUTE);
    Unroll<TAX>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<TAY>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<TSX>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<TXA>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<TXS>::start(opcodes, modes, ADDR_MODE_NONE);
    Unroll<TYA>::start(opcodes, modes, ADDR_MODE_NONE);
}

/**
 * CPU instruction implementations
 */


#define DEFINE_OPCODE(opcode) \
template<AddressMode mode> struct InstructionImplementationX<opcode, mode> { \
    static void execute(InstructionContext *ctx, MemoryResolver resolver); \
}; \
template<AddressMode mode> \
void InstructionImplementationX<opcode, mode>::execute(InstructionContext *ctx, MemoryResolver resolver)

DEFINE_OPCODE(SEI) {
    ctx->registers->P.I = 1;
}

DEFINE_OPCODE(CLD) {
    ctx->registers->P.D = 0;
}

DEFINE_OPCODE(LDA) {
    PrintDbg("LDA mode=%d") % (int) mode;
    ctx->registers->A = tMemoryOperation<mode>::GetByte(ctx);
    ctx->registers->setZeroBit(ctx->registers->A);
    ctx->registers->setSignBit(ctx->registers->A);
}

DEFINE_OPCODE(LDX) {
    ctx->registers->X = tMemoryOperation<mode>::GetByte(ctx);
    ctx->registers->setZeroBit(ctx->registers->X);
    ctx->registers->setSignBit(ctx->registers->X);

}

DEFINE_OPCODE(LDY) {
    ctx->registers->Y = tMemoryOperation<mode>::GetByte(ctx);
    ctx->registers->setZeroBit(ctx->registers->Y);
    ctx->registers->setSignBit(ctx->registers->Y);

}

DEFINE_OPCODE(STA) {
    PrintDbg("STA mode=%d") % (int) mode;
    tMemoryOperation<mode>::WriteByte(ctx, ctx->registers->A);
}

void writeStack(InstructionContext* ctx, tCPU::byte value) {
    auto address = 0x1FF - ctx->registers->S;
    ctx->mem->writeByte(address, value);
}

tCPU::byte readStack(InstructionContext* ctx) {
    auto address = 0x1FF - ctx->registers->S;
    return ctx->mem->readByte(address);
}

DEFINE_OPCODE(TXS) {
    PrintDbg("TXS mode=%d") % (int) mode;
    writeStack(ctx, ctx->registers->X);
}

/**
 * Generic branch instruction
 * provides implementation for various branch-on-cpu-status-flag
 */
template<ProcessorStatusFlags Register>
struct BranchIf {
    static void is(InstructionContext *ctx, bool expectedState) {
        signed char relativeOffset = ctx->mem->readByte(ctx->registers->LastPC + 1);// g_Memory.GetByteAfterPC();
        unsigned short jmpAddress = ctx->registers->PC + relativeOffset;
        bool flagState = ProcessorStatusFlag<Register>::getState(ctx);

        PrintDbg("-> Status Register (%d) %s: %d; Relative offset: $%X; Absolute jump address = $%08X")
                % (int) Register % ProcessorStatusFlagNames[Register]
                % (int) flagState % (int) relativeOffset % (int) jmpAddress;

        if (flagState == expectedState) {
            PrintDbg("-> Branch Taken");
//            CPU::Singleton()->IncCycles();

            // add another cycle if branch goes to a diff page
            if ((jmpAddress & 0xF0) != (ctx->registers->PC & 0xF0)) {
//                CPU::Singleton()->IncCycles();
            }

            ctx->registers->PC = jmpAddress;
        }
    }
};

DEFINE_OPCODE(BPL) {
    PrintDbg("BPL");
    BranchIf<NEGATIVE_BIT>::is(ctx, false);
}

DEFINE_OPCODE(BMI) {
    PrintDbg("BMI");
    BranchIf<NEGATIVE_BIT>::is(ctx, true);
}

DEFINE_OPCODE(BNE) {
    PrintDbg("BNE");
    BranchIf<ZERO_BIT>::is(ctx, false);
}

DEFINE_OPCODE(BEQ) {
    PrintDbg("BEQ");
    BranchIf<ZERO_BIT>::is(ctx, true);
}

DEFINE_OPCODE(BCS) {
    PrintDbg("BCS");
    BranchIf<CARRY_BIT>::is(ctx, true);
}

DEFINE_OPCODE(BCC) {
    PrintDbg("BCC");
    BranchIf<CARRY_BIT>::is(ctx, false);
}

DEFINE_OPCODE(BVC) {
    PrintDbg("BVC");
    BranchIf<OVERFLOW_BIT>::is(ctx, false);
}

DEFINE_OPCODE(BVS) {
    PrintDbg("BVS");
    BranchIf<OVERFLOW_BIT>::is(ctx, true);
}

/*
// various ways of implementing an opcode
// first is using lambda:

auto Instruction_SEI = [](AddressMode mode, InstructionContext *ctx) {
    ctx->registers->P.I = 1;
};

// second is a partially specialized class definition
template<enum AddressMode mode>
struct InstructionImplementation<SEI, mode> {
    static void execute(InstructionContext *ctx) {
        PrintInfo("SEI!");
        ctx->registers->P.I = 1;
    }
};

// third is a partially speialized class method
template<>
void InstructionImplementationX<LDA>::execute(InstructionContext *ctx, MemoryResolver resolver) {
    PrintInfo("LDA!");
}
*/