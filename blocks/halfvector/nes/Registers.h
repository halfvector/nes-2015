#pragma once

// bit numbering starts with 0, so last bit in a byte is #7
template<unsigned char BitPlace>
struct Bit {
    // get a bool that indicates if that bit is set
    static inline bool IsSet(tCPU::byte Byte) {
        return (Byte & (1 << BitPlace)) != 0;
    }

    static inline tCPU::byte QuickIsSet(tCPU::byte Byte) {
        return (Byte >> BitPlace) & 1;
    }

    static inline tCPU::byte IsSet(int Integer) {
        return (Integer >> BitPlace) & 1;
    }

    // get just that bit
    static inline tCPU::byte Get(tCPU::byte Byte) {
        return (tCPU::byte) (Byte & (1 << BitPlace));
    }

    static inline char ToAscii(tCPU::byte Byte) {
        return (Byte & (1 << BitPlace)) ? '1' : '0';
    }

    static inline char ToAscii(tCPU::word Value) {
        return (Value & (1 << BitPlace)) ? '1' : '0';
    }

    static inline tCPU::byte Set(bool On) {
        return (1 << BitPlace) * (char) On;
    }
};

// returns bit 7,6,5,4,3,2 as zeroes, and 0,1 as gotten values
// returned in bit 0 is bitplace1, returned in bit1 is bitplace2
template<unsigned char BitPlace1, unsigned char BitPlace2>
struct Bits {
    // get just that bit
    static inline tCPU::byte Get(tCPU::byte Value) {
        tCPU::byte Bit1 = (Value & tCPU::byte(1 << BitPlace1)) >> BitPlace1;
        tCPU::byte Bit2 = (Value & tCPU::byte(1 << BitPlace2)) >> BitPlace2;

        return Bit1 | (Bit2 << 1);
    }
};

enum ProcessorStatusFlags {
    CARRY_BIT, ZERO_BIT, IRQ_DISABLE, BCD_MODE,
    BREAK_FLAG, ALWAYS_1, OVERFLOW_BIT, NEGATIVE_BIT
};
static const char *ProcessorStatusFlagNames[] = {
    "CARRY_BIT", "ZERO_BIT", "IRQ_DISABLE", "BCD MODE",
    "BREAK FLAG", "ALWAYS 1", "OVERFLOW_BIT", "NEGATIVE_BIT"
};

struct ProcessorStatusRegister {
    tCPU::byte C : 1;    // carry bit
    tCPU::byte Z : 1;    // zero bit
    tCPU::byte I : 1;    // irq disabled
    tCPU::byte D : 1;    // bcd mode for adc/sbc (n/a on nes)
    tCPU::byte B : 1;    // break flag: 0 = irq/nmi, 1 = brk/php opcode
    tCPU::byte X : 1;    // always 1
    tCPU::byte V : 1;    // overflow bit
    tCPU::byte N : 1;    // negative bit: 0 = positive, 1 = negative

    ProcessorStatusRegister() {
        Reset();
    }

    void Reset() {
        C = Z = I = D = B = V = N = 0;
        X = 1; // always 1 :D
    }

    tCPU::byte asByte() {
        return Bit<7>::Set(N) + Bit<6>::Set(V) + Bit<5>::Set(X) + Bit<4>::Set(B) + Bit<3>::Set(D) + Bit<2>::Set(I) + Bit<1>::Set(Z) + Bit<0>::Set(C);
    }

    void fromByte(tCPU::byte Value) {
        N = Bit<7>::IsSet(Value);
        V = Bit<6>::IsSet(Value);
        X = Bit<5>::IsSet(Value);
        B = Bit<4>::IsSet(Value);
        D = Bit<3>::IsSet(Value);
        I = Bit<2>::IsSet(Value);
        Z = Bit<1>::IsSet(Value);
        C = Bit<0>::IsSet(Value);
    }
};

struct Registers {
    Registers() {
        A = X = Y = 0;
        LastPC = PC = 0;

        // stack pointer starts with 0, but points to memory address 0x1FF
        // max value is 255 and then its pointin to 0x100
        S = 0;
    }

    // accumulator, index register x, index register y, stack pointer, and processor status register
    tCPU::byte A, X, Y, S;

    ProcessorStatusRegister P;

    // "program counter" - next instruction to be executed
    tCPU::word PC;

    // current instruction being executed
    tCPU::word LastPC;

    void setSignBit(uint16_t value) {
        P.N = static_cast<uint8_t>((value & 0x80) != 0);
    }

    void setZeroBit(uint16_t value) {
        P.Z = static_cast<uint8_t>(value == 0);
    }

    void setOverflowFlag(uint16_t value) {
        P.V = static_cast<uint8_t>(value != 0);
    }
};

enum Register {
    ACCUMULATOR, REGISTER_X, REGISTER_Y
};

static const char *REGISTER_NAME[] = {
        "Accumulator", "Register X", "Register Y"
};
