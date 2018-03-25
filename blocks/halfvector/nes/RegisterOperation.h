#pragma once

/**
 * Register helper
 */
template<Register register>
struct RegisterOperation {
};

template<>
struct RegisterOperation<ACCUMULATOR> {
    static void write(InstructionContext *ctx, tCPU::byte value);
    static tCPU::byte read(InstructionContext *ctx);
};

template<>
struct RegisterOperation<REGISTER_X> {
    static void write(InstructionContext *ctx, tCPU::byte value);
    static tCPU::byte read(InstructionContext *ctx);
};

template<>
struct RegisterOperation<REGISTER_Y> {
    static void write(InstructionContext *ctx, tCPU::byte value);
    static tCPU::byte read(InstructionContext *ctx);
};