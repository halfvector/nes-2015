#include "MemoryLookup.h"

bool MemoryAddressResolveBase::PageBoundaryCrossed = false;

int MemoryAddressResolve<ADDR_MODE_NONE>::NumOfCalls = 0;
int MemoryAddressResolve<ADDR_MODE_ZEROPAGE>::NumOfCalls = 0;
int MemoryAddressResolve<ADDR_MODE_ZEROPAGE_INDEXED_X>::NumOfCalls = 0;
int MemoryAddressResolve<ADDR_MODE_ABSOLUTE>::NumOfCalls = 0;
int MemoryAddressResolve<ADDR_MODE_ABSOLUTE_INDEXED_X>::NumOfCalls = 0;
int MemoryAddressResolve<ADDR_MODE_ABSOLUTE_INDEXED_Y>::NumOfCalls = 0;
int MemoryAddressResolve<ADDR_MODE_INDIRECT_INDEXED>::NumOfCalls = 0;
int MemoryAddressResolve<ADDR_MODE_INDIRECT_ABSOLUTE>::NumOfCalls = 0;

template<>
bool ProcessorStatusFlag<CARRY_BIT>::getState(InstructionContext *ctx) {
    return ctx->registers->P.C;
}

template<>
bool ProcessorStatusFlag<ZERO_BIT>::getState(InstructionContext *ctx) {
    return ctx->registers->P.Z;
}

template<>
bool ProcessorStatusFlag<NEGATIVE_BIT>::getState(InstructionContext *ctx) {
    return ctx->registers->P.N;
}

template<>
bool ProcessorStatusFlag<OVERFLOW_BIT>::getState(InstructionContext *ctx) {
    return ctx->registers->P.V;
}

template<>
bool ProcessorStatusFlag<BCD_MODE>::getState(InstructionContext *ctx) {
    return ctx->registers->P.D;
}

template<>
bool ProcessorStatusFlag<ALWAYS_1>::getState(InstructionContext *ctx) {
    return 1;
}

template<>
bool ProcessorStatusFlag<BREAK_FLAG>::getState(InstructionContext *ctx) {
    return ctx->registers->P.B;
}