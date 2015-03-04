#pragma once

/**
 * Per-memory-address opcode-variant generator
 */
template<InstructionMnemonic opcode, enum AddressMode mode>
struct UnrollInstructions {
    static void unroll(Opcode *opcodes, AddressModeProperties *props) {
        // Calculate opcode variant given a memory address mode
        uint8_t opcodeVariant = opcode + props[mode].offset;

        // There are some exceptions to the formula above.
        // * LDX with absolute indexed Y register address mode uses +0x10 offset instead of +0xC
        if (opcode == LDX && mode == ADDR_MODE_ABSOLUTE_INDEXED_Y) {
            opcodeVariant = uint8_t(opcode + 0x10); // 0xBE
        }

        PrintInfo("  opcode=%i variant=%i offset=%i") % opcode % (int) opcodeVariant % (int) props[mode].offset;

        opcodes[opcodeVariant].execute = &InstructionImplementation<opcode, mode>::execute;

        UnrollInstructions<opcode, static_cast<AddressMode>(mode - 1)>::unroll(opcodes, props);
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

        PrintInfo("assigning mask=%d") % std::bitset<8>(mask).to_string('0', '1');

        opcodes[opcode].AddressModeMask = mask;

        UnrollInstructions<opcode, static_cast<AddressMode>(ADDR_MODE_LAST - 1)>::unroll(opcodes, props);
    }
};

/**
* Exit condition for template unroll
*/
template<InstructionMnemonic opcode>
struct UnrollInstructions<opcode, ADDR_MODE_NONE> {
    static void unroll(Opcode *opcodes, AddressModeProperties *props) {
        PrintInfo("Unroll completed for opcode=%i") % opcode;
    }
};