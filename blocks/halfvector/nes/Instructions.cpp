#include <stdint.h>
#include "Instructions.h"
#include "MemoryOperation.h"
#include "RegisterOperation.h"

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

    // NOP and Future Expansion / Unofficial Opcodes
    for(int op : {0x1A, 0xEA, 0x3A, 0x5A, 0x7A, 0xDA, 0xFA}) {
        opcodes[op].set("NOP", 1, 2, 0, "NOP", ADDR_MODE_NONE);
        opcodes[op].execute = &InstructionImplementation<NOP, ADDR_MODE_NONE>::execute;
    }

    // NOP-like: Skip next byte
    for(int op : {0x80, 0x82, 0xC2, 0xE2, 0x04, 0x14, 0x34, 0x44, 0x54, 0x64, 0x74, 0xD4, 0xF4}) {
        opcodes[op].set("SKB", 2, 3, 0, "SKB nn", ADDR_MODE_ZEROPAGE);
        opcodes[op].execute = &InstructionImplementation<NOP, ADDR_MODE_ZEROPAGE>::execute;
    }

    // NOP-like: Skip next word
    for(int op : {0x0C, 0x1C, 0x3C, 0x5C, 0x7C, 0xDC, 0xFC}) {
        opcodes[op].set("SKB", 3, 4, 0, "SKB nnnn", ADDR_MODE_ABSOLUTE);
        opcodes[op].execute = &InstructionImplementation<NOP, ADDR_MODE_ABSOLUTE>::execute;
    }

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

            PrintInfo("  + opcode=0x%02X variant=0x%02X offset=%d mode=%d mask=%d")
                % (int) opcode % (int) opcodeVariant % (int) props[mode].offset
                % (int) mode % (int) AddressModeMask[mode];
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

/*
 * Flag manipulation
 */

DEFINE_OPCODE(SEI) {
    ctx->registers->P.I = 1;
}

DEFINE_OPCODE(SEC) {
    ctx->registers->P.C = 1;
}

DEFINE_OPCODE(SED) {
    ctx->registers->P.D = 1;
}

DEFINE_OPCODE(CLD) {
    ctx->registers->P.D = 0;
}

DEFINE_OPCODE(CLC) {
    ctx->registers->P.C = 0;
}

DEFINE_OPCODE(CLI) {
    ctx->registers->P.I = 0;
}

DEFINE_OPCODE(CLV) {
    ctx->registers->P.V = 0;
}

/**
 * Load and Store into registers
 */

DEFINE_OPCODE(LDA) {
    RegisterOperation<ACCUMULATOR>::write(ctx, MemoryOperation<mode>::readByte(ctx));
    ctx->registers->setZeroBit(ctx->registers->A);
    ctx->registers->setSignBit(ctx->registers->A);
}

DEFINE_OPCODE(LDX) {
    RegisterOperation<REGISTER_X>::write(ctx, MemoryOperation<mode>::readByte(ctx));
    ctx->registers->setZeroBit(ctx->registers->X);
    ctx->registers->setSignBit(ctx->registers->X);
}

DEFINE_OPCODE(LDY) {
    RegisterOperation<REGISTER_Y>::write(ctx, MemoryOperation<mode>::readByte(ctx));
    ctx->registers->setZeroBit(ctx->registers->Y);
    ctx->registers->setSignBit(ctx->registers->Y);

}

// store accumulator
DEFINE_OPCODE(STA) {
    MemoryOperation<mode>::writeByte(ctx, ctx->registers->A);
}

// store register X
DEFINE_OPCODE(STX) {
    MemoryOperation<mode>::writeByte(ctx, ctx->registers->X);
}

// store register Y
DEFINE_OPCODE(STY) {
    MemoryOperation<mode>::writeByte(ctx, ctx->registers->Y);
}

/**
 * Push/Pop on stack
 */

// Accumulator -> Stack
DEFINE_OPCODE(PHA) {
    ctx->stack->pushStackByte(ctx->registers->A);
}

// Accumulator <- Stack
DEFINE_OPCODE(PLA) {
    tCPU::byte value = ctx->stack->popStackByte();
    RegisterOperation<ACCUMULATOR>::write(ctx, value);

    ctx->registers->setSignBit(value);
    ctx->registers->setZeroBit(value);
}

// processor status -> Stack
DEFINE_OPCODE(PHP) {
    ctx->stack->pushStackByte(ctx->registers->P.asByte());
}

// processor status <- Stack
DEFINE_OPCODE(PLP) {
    ctx->registers->P.fromByte(ctx->stack->popStackByte());
}

/**
 * Transfer registers
 */

// stack pointer -> X
DEFINE_OPCODE(TSX) {
    tCPU::byte sp = ctx->registers->S;
    RegisterOperation<REGISTER_X>::write(ctx, sp);

    ctx->registers->setSignBit(sp);
    ctx->registers->setZeroBit(sp);
}

// X -> stack pointer
DEFINE_OPCODE(TXS) {
    ctx->registers->S = RegisterOperation<REGISTER_X>::read(ctx);
}

// X -> Accumulator
DEFINE_OPCODE(TXA) {
    tCPU::byte reg = RegisterOperation<REGISTER_X>::read(ctx);
    RegisterOperation<ACCUMULATOR>::write(ctx, reg);

    ctx->registers->setSignBit(reg);
    ctx->registers->setZeroBit(reg);
}

// Y -> Accumulator
DEFINE_OPCODE(TYA) {
    tCPU::byte reg = RegisterOperation<REGISTER_Y>::read(ctx);
    RegisterOperation<ACCUMULATOR>::write(ctx, reg);

    ctx->registers->setSignBit(reg);
    ctx->registers->setZeroBit(reg);
}

// Accumulator -> X
DEFINE_OPCODE(TAX) {
    tCPU::byte reg = RegisterOperation<ACCUMULATOR>::read(ctx);
    RegisterOperation<REGISTER_X>::write(ctx, reg);

    ctx->registers->setSignBit(reg);
    ctx->registers->setZeroBit(reg);
}

// Accumulator -> Y
DEFINE_OPCODE(TAY) {
    tCPU::byte reg = RegisterOperation<ACCUMULATOR>::read(ctx);
    RegisterOperation<REGISTER_Y>::write(ctx, reg);

    ctx->registers->setSignBit(reg);
    ctx->registers->setZeroBit(reg);
}

/**
 * Generic branch instruction
 * provides implementation for various branch-on-cpu-status-flag
 */

template<ProcessorStatusFlags Register>
struct BranchIf {
    static void is(InstructionContext *ctx, bool expectedState) {
        // get signed offset value
        signed char relativeOffset = ctx->mem->readByte(ctx->registers->LastPC + 1);
        tCPU::word jmpAddress = ctx->registers->PC + relativeOffset;
        bool flagState = ProcessorStatusFlag<Register>::getState(ctx);

//        PrintCpu("-> Status Register %s: %d; Relative offset: $%02X; Calculated jump address = $%04X")
//                % ProcessorStatusFlagNames[Register] % (int) flagState
//                % (signed int) relativeOffset % jmpAddress;

        if (flagState == expectedState) {
//            PrintCpu("-> Branch Taken");
//            CPU::Singleton()->IncCycles();

            // add another cycle if branch goes to a diff page
            if ((jmpAddress & 0xF0) != (ctx->registers->PC & 0xF0)) {
//                CPU::Singleton()->IncCycles();
            }

            ctx->registers->PC = jmpAddress;
        }
    };
};

/**
 * Branching
 */

DEFINE_OPCODE(BPL) {
    BranchIf<NEGATIVE_BIT>::is(ctx, false);
}

DEFINE_OPCODE(BMI) {
    BranchIf<NEGATIVE_BIT>::is(ctx, true);
}

DEFINE_OPCODE(BNE) {
    BranchIf<ZERO_BIT>::is(ctx, false);
}

DEFINE_OPCODE(BEQ) {
    BranchIf<ZERO_BIT>::is(ctx, true);
}

DEFINE_OPCODE(BCS) {
    BranchIf<CARRY_BIT>::is(ctx, true);
}

DEFINE_OPCODE(BCC) {
    BranchIf<CARRY_BIT>::is(ctx, false);
}

DEFINE_OPCODE(BVC) {
    BranchIf<OVERFLOW_BIT>::is(ctx, false);
}

DEFINE_OPCODE(BVS) {
    BranchIf<OVERFLOW_BIT>::is(ctx, true);
}


/**
 * Generic compare instruction
 */

template<Register R, AddressMode M>
struct GenericComparator {
    static void execute(InstructionContext* ctx) {
        tCPU::byte mem = MemoryOperation<M>::readByte(ctx);
        tCPU::byte reg = RegisterOperation<R>::read(ctx);
        tCPU::byte result = reg - mem;

        ctx->registers->setSignBit(result);
        ctx->registers->setZeroBit(result);
        ctx->registers->P.C = uint8_t(reg >= mem);
    }
};

/**
 * Comparisons
 */

DEFINE_OPCODE(CPX) {
    GenericComparator<REGISTER_X, mode>::execute(ctx);
}

DEFINE_OPCODE(CPY) {
    GenericComparator<REGISTER_Y, mode>::execute(ctx);
}

DEFINE_OPCODE(CMP) {
    GenericComparator<ACCUMULATOR, mode>::execute(ctx);
}

/*
 * Increment and Decrement
 */

DEFINE_OPCODE(DEX) {
    tCPU::byte value = uint8_t(RegisterOperation<REGISTER_X>::read(ctx) - 1);
    RegisterOperation<REGISTER_X>::write(ctx, value);

    ctx->registers->setSignBit(value);
    ctx->registers->setZeroBit(value);
}

DEFINE_OPCODE(DEY) {
    tCPU::byte value = uint8_t(RegisterOperation<REGISTER_Y>::read(ctx) - 1);
    RegisterOperation<REGISTER_Y>::write(ctx, value);

    ctx->registers->setSignBit(value);
    ctx->registers->setZeroBit(value);
}

DEFINE_OPCODE(DEC) {
    tCPU::byte value = uint8_t(MemoryOperation<mode>::readByte(ctx) - 1);
    MemoryOperation<mode>::writeByte(ctx, value);

    ctx->registers->setSignBit(value);
    ctx->registers->setZeroBit(value);
}

DEFINE_OPCODE(INC) {
    tCPU::byte value = uint8_t(MemoryOperation<mode>::readByte(ctx) + 1);
    MemoryOperation<mode>::writeByte(ctx, value);

    ctx->registers->setSignBit(value);
    ctx->registers->setZeroBit(value);
}

DEFINE_OPCODE(INX) {
    tCPU::byte value = RegisterOperation<REGISTER_X>::read(ctx) + 1;
    RegisterOperation<REGISTER_X>::write(ctx, value);

    ctx->registers->setSignBit(value);
    ctx->registers->setZeroBit(value);
}
DEFINE_OPCODE(INY) {
    tCPU::byte value = RegisterOperation<REGISTER_Y>::read(ctx) + 1;
    RegisterOperation<REGISTER_Y>::write(ctx, value);

    ctx->registers->setSignBit(value);
    ctx->registers->setZeroBit(value);
}

// call subroutine
// push address-1 of next instruction on stack
// then set next instruction to address
DEFINE_OPCODE(JSR) {
    ctx->stack->pushStackWord(--ctx->registers->PC);
    tCPU::word address = MemoryOperation<mode>::GetEffectiveAddress(ctx);

    PrintCpu("Pushed old PC $%04X on stack; Setting new PC to $%04X")
            % ctx->registers->PC % address;

    ctx->registers->PC = address;
}

DEFINE_OPCODE(JMP) {
    tCPU::word address = MemoryOperation<mode>::GetEffectiveAddress(ctx);

    // special case: if address low byte is 0xff, wrap around and read upper byte

//    PrintCpu("Setting new PC to $%04X") % address;
    ctx->registers->PC = address;
}

// return from subroutine
DEFINE_OPCODE(RTS) {
    // pop return address from stack and increment by 1
    tCPU::word address = ctx->stack->popStackWord() + 1;
    ctx->registers->PC = address;
    PrintDbg("Restored old PC from stack: $%04X") % address;


}

// return from interrupt
DEFINE_OPCODE(RTI) {
    // restore processor status from stack
    tCPU::byte processorStatus = ctx->stack->popStackByte();
    ctx->registers->P.fromByte(processorStatus);

    // restore program counter from stack
    ctx->registers->PC = ctx->stack->popStackWord();
    PrintDbg("Restored old PC from stack: 0x%04X") % ctx->registers->PC;
}

/*
 * Bitwise manipulation
 */

// or
DEFINE_OPCODE(ORA) {
    tCPU::byte mem = MemoryOperation<mode>::readByte(ctx);
    tCPU::byte reg = RegisterOperation<ACCUMULATOR>::read(ctx);

    mem |= reg;

    ctx->registers->setSignBit(mem);
    ctx->registers->setZeroBit(mem);

    RegisterOperation<ACCUMULATOR>::write(ctx, mem);
}

// xor
DEFINE_OPCODE(EOR) {
    tCPU::byte mem = MemoryOperation<mode>::readByte(ctx);
    tCPU::byte reg = RegisterOperation<ACCUMULATOR>::read(ctx);

    mem ^= reg;

    ctx->registers->setSignBit(mem);
    ctx->registers->setZeroBit(mem);

    RegisterOperation<ACCUMULATOR>::write(ctx, mem);
}

// and
DEFINE_OPCODE(AND) {
    tCPU::byte mem = MemoryOperation<mode>::readByte(ctx);
    tCPU::byte reg = RegisterOperation<ACCUMULATOR>::read(ctx);

    mem &= reg;

    ctx->registers->setSignBit(mem);
    ctx->registers->setZeroBit(mem);

    RegisterOperation<ACCUMULATOR>::write(ctx, mem);
}

DEFINE_OPCODE(BIT) {
    tCPU::byte mem = MemoryOperation<mode>::readByte(ctx);
    tCPU::byte reg = RegisterOperation<ACCUMULATOR>::read(ctx);

    ctx->registers->setSignBit(mem);
    ctx->registers->setOverflowFlag(mem & 0x40); // 6th bit is set
    ctx->registers->setZeroBit(reg & mem);
}

// shift right one bit
DEFINE_OPCODE(LSR) {
    tCPU::byte mem = MemoryOperation<mode>::readByte(ctx);

    ctx->registers->P.C = Bit<0>::IsSet(mem); // pop lowest bit onto the carry bit
    mem >>= 1;

    ctx->registers->setSignBit(mem);
    ctx->registers->setZeroBit(mem);

    MemoryOperation<mode>::writeByte(ctx, mem);
}

// shift left one bit
DEFINE_OPCODE(ASL) {
    tCPU::byte mem = MemoryOperation<mode>::readByte(ctx);

    ctx->registers->P.C = Bit<7>::IsSet(mem); // pop highest bit onto the carry bit
    mem <<= 1;

    ctx->registers->setSignBit(mem);
    ctx->registers->setZeroBit(mem);

    MemoryOperation<mode>::writeByte(ctx, mem);
}

// rotate shift left one bit
DEFINE_OPCODE(ROL) {
    tCPU::byte mem = MemoryOperation<mode>::readByte(ctx);

    tCPU::byte newCarry = Bit<7>::IsSet(mem); // save highest bit as new carry
    mem <<= 1; // shift left
    mem |= Bit<0>::Set(ctx->registers->P.C); // add lowest bit from old carry
    ctx->registers->P.C = newCarry; // save new carry

    ctx->registers->setSignBit(mem);
    ctx->registers->setZeroBit(mem);

    MemoryOperation<mode>::writeByte(ctx, mem);
}

// rotate right one bit
DEFINE_OPCODE(ROR) {
    tCPU::byte mem = MemoryOperation<mode>::readByte(ctx);

    tCPU::byte newCarry = Bit<0>::IsSet(mem); // save highest bit as new carry
    mem >>= 1; // shift right
    mem |= Bit<7>::Set(ctx->registers->P.C); // add highest bit from old carry
    ctx->registers->P.C = newCarry; // save new carry

    ctx->registers->setSignBit(mem);
    ctx->registers->setZeroBit(mem);

    MemoryOperation<mode>::writeByte(ctx, mem);
}

DEFINE_OPCODE(BRK) {
    // PC has already been incremented by 1 (brk = 1 byte opcode)
    // we want PC+2
    ctx->registers->PC ++;
    ctx->stack->pushStackWord(ctx->registers->PC);
    // set break flag
    ctx->registers->P.B = 1;
    // push status on stack
    ctx->stack->pushStackByte(ctx->registers->P.asByte());
    // disable irq
    ctx->registers->P.I = 1;

    tCPU::word breakAddress = ctx->mem->readWord(0xFFFE);

    PrintCpu("Break Vector; saved next PC=$%04X in stack; setting PC=$%04X")
            % ctx->registers->PC % breakAddress;

    ctx->registers->PC = breakAddress;
}

/*
 * Arithmetic
 */

// addition
DEFINE_OPCODE(ADC) {
    tCPU::byte value = MemoryOperation<mode>::readByte(ctx);
    tCPU::byte accumulator = RegisterOperation<ACCUMULATOR>::read(ctx);

    tCPU::word result = accumulator + value + (ctx->registers->P.C ? 1 : 0);

    tCPU::byte resultAsByte = static_cast<tCPU::byte>(result);

    ctx->registers->setSignBit(resultAsByte);
    ctx->registers->setZeroBit(resultAsByte);
    ctx->registers->setOverflowFlag(~(accumulator ^ value) & (accumulator ^ resultAsByte) & 0x80);
    ctx->registers->P.C = result > 256;

    RegisterOperation<ACCUMULATOR>::write(ctx, resultAsByte);
}

// subtraction
DEFINE_OPCODE(SBC) {
    tCPU::byte value = MemoryOperation<mode>::readByte(ctx);
    tCPU::byte accumulator = RegisterOperation<ACCUMULATOR>::read(ctx);

    tCPU::word result = accumulator - value - (ctx->registers->P.C ? 0 : 1);

    tCPU::byte resultAsByte = static_cast<tCPU::byte>(result);

    ctx->registers->setSignBit(resultAsByte);
    ctx->registers->setZeroBit(resultAsByte);
    ctx->registers->setOverflowFlag(((accumulator ^ value) & 0x80) && (accumulator ^ resultAsByte) & 0x80);
    ctx->registers->P.C = result <= 256;

    RegisterOperation<ACCUMULATOR>::write(ctx, resultAsByte);
}

// nop
DEFINE_OPCODE(NOP) {

}
