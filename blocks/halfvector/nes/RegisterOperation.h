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
    }

    static tCPU::byte read(InstructionContext *ctx) {
        return ctx->registers->A;
    }
};

template<>
struct RegisterOperation<REGISTER_X> {
    static void write(InstructionContext *ctx, tCPU::byte value) {
        ctx->registers->X = value;
    }

    static tCPU::byte read(InstructionContext *ctx) {
        return ctx->registers->X;
    }
};

template<>
struct RegisterOperation<REGISTER_Y> {
    static void write(InstructionContext *ctx, tCPU::byte value) {
        ctx->registers->Y = value;
    }

    static tCPU::byte read(InstructionContext *ctx) {
        return ctx->registers->Y;
    }
};