#pragma once

#include "MemoryLookup.h"

/**
 * perform basic memory ops on effective memory address
 */
template<int MemoryMode>
struct MemoryOperation : MemoryAddressResolve<MemoryMode> {
    static tCPU::byte readByte(InstructionContext *ctx) {
        return ctx->mem->readByte(MemoryAddressResolve<MemoryMode>::GetEffectiveAddress(ctx));
    }

    static tCPU::word readWord(InstructionContext *ctx) {
        return ctx->mem->readWord(MemoryAddressResolve<MemoryMode>::GetEffectiveAddress(ctx));
    }

    static bool writeByte(InstructionContext *ctx, tCPU::byte value) {
        return ctx->mem->writeByte(MemoryAddressResolve<MemoryMode>::GetEffectiveAddress(ctx), value);
    }

    static bool writeWord(InstructionContext *ctx, tCPU::word value) {
        return ctx->mem->writeByte(MemoryAddressResolve<MemoryMode>::GetEffectiveAddress(ctx), value);
    }
};

// derives from MemoryOperation< ADDR_MODE_NONE > in order to have an error printing
// GetEffectiveAddress call, since it is an error to use that call for these specific memory modes

// no writing to immediate operands :D
template<>
struct MemoryOperation<ADDR_MODE_IMMEDIATE> : MemoryOperation<ADDR_MODE_NONE> {
    static void writeByte(InstructionContext *ctx, tCPU::byte value) {
        PrintError("MemoryOperation<ADDR_MODE_IMMEDIATE>::writeByte(); Impossible Action in Immediate Mode");
    }

    static void writeWord(InstructionContext *ctx, tCPU::word value) {
        PrintError("MemoryOperation<ADDR_MODE_IMMEDIATE>::writeWord(); Impossible Action in Immediate Mode");
    }

    static tCPU::byte readByte(InstructionContext *ctx) {
        tCPU::byte value = ctx->mem->readByte(ctx->registers->LastPC + 1);
        PrintMemory("MemoryOperation<ADDR_MODE_IMMEDIATE>::readByte(); value = 0x%X") % (int)value;
        return value;
    }

    // this is not the same as absolute address
    // this CAN be a 16bit address, which is absolute, but the value is not a memory location of data
    // its an address to work on ITSELF.. eg jump to there
    static tCPU::word readWord(InstructionContext *ctx) {
        tCPU::word value = ctx->mem->readWord(ctx->registers->LastPC + 1);
        PrintMemory("MemoryOperation<ADDR_MODE_IMMEDIATE>::readWord(); value = 0x%X") % (int)value;
        return value;
    }
};

// working on the accumulator register -- not really memory :D
template<>
struct MemoryOperation<ADDR_MODE_ACCUMULATOR> : MemoryOperation<ADDR_MODE_NONE> {
    static void writeByte(InstructionContext *ctx, tCPU::byte value) {
        ctx->registers->A = value;
        PrintMemory("MemoryOperation<ACCUMULATOR>::writeByte(); value = 0x%X") % value;
    }

    static void writeWord(tCPU::word value) {
        PrintError("MemoryOperation::writeByte(); Impossible Action in Accumulator Mode");
    }

    static tCPU::byte readByte(InstructionContext *ctx) {
        tCPU::byte value = ctx->registers->A;
        PrintMemory("MemoryOperation<ACCUMULATOR>::readByte(); value = 0x%X") % value;
        return value;
    }

    static tCPU::word readWord(InstructionContext *ctx) {
        PrintError("MemoryOperation::readWord(); Impossible Action in Accumulator Mode");
        return 0;
    }
};


// this is the same as normal immediate, except its to registers x/y (instead of accumulator?)
// and so the opcode generated is diff, +4h instead of -4h or something like that
template<>
struct MemoryOperation<ADDR_MODE_IMMEDIATE_TO_XY> : MemoryOperation< ADDR_MODE_IMMEDIATE > {
};
