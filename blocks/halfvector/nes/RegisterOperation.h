#pragma once

/**
 * Register helper
 */
template<Register register>
struct RegisterOperation {
};

template<>
struct RegisterOperation<ACCUMULATOR> {
    static void write(InstructionContext *ctx, tCPU::byte value) {
        ctx->registers->A = value;
        PrintDbg("Wrote A = 0x%02X") % (int) value;
    }

    static tCPU::byte read(InstructionContext *ctx) {
        PrintDbg("Read A = 0x%02X") % (int) ctx->registers->A;
        return ctx->registers->A;
    }
};

template<>
struct RegisterOperation<REGISTER_X> {
    static void write(InstructionContext *ctx, tCPU::byte value) {
        ctx->registers->X = value;
        PrintDbg("Wrote X = 0x%02X") % (int) value;
    }

    static tCPU::byte read(InstructionContext *ctx) {
        PrintDbg("Read X = 0x%02X") % (int) ctx->registers->X;
        return ctx->registers->X;
    }
};

template<>
struct RegisterOperation<REGISTER_Y> {
    static void write(InstructionContext *ctx, tCPU::byte value) {
        ctx->registers->Y = value;
        PrintDbg("Wrote Y = 0x%02X") % (int) value;
    }

    static tCPU::byte read(InstructionContext *ctx) {
        PrintDbg("Read Y = 0x%02X") % (int) ctx->registers->Y;
        return ctx->registers->Y;
    }
};