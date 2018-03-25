#include "Platform.h"
#include "Registers.h"
#include "Logging.h"
#include "MemoryLookup.h"
#include "RegisterOperation.h"

void
RegisterOperation<ACCUMULATOR>::write(InstructionContext *ctx, tCPU::byte value) {
    ctx->registers->A = value;
//    PrintDbg("Wrote A = 0x%02X") % (int) value;
}

tCPU::byte
RegisterOperation<ACCUMULATOR>::read(InstructionContext *ctx) {
//    PrintDbg("Read A = 0x%02X") % (int) ctx->registers->A;
    return ctx->registers->A;
}

void
RegisterOperation<REGISTER_X>::write(InstructionContext *ctx, tCPU::byte value) {
    ctx->registers->X = value;
//    PrintDbg("Wrote X = 0x%02X") % (int) value;
}

tCPU::byte
RegisterOperation<REGISTER_X>::read(InstructionContext *ctx) {
//    PrintDbg("Read X = 0x%02X") % (int) ctx->registers->X;
    return ctx->registers->X;
}

void
RegisterOperation<REGISTER_Y>::write(InstructionContext *ctx, tCPU::byte value) {
    ctx->registers->Y = value;
//    PrintDbg("Wrote Y = 0x%02X") % (int) value;
}

tCPU::byte
RegisterOperation<REGISTER_Y>::read(InstructionContext *ctx) {
//    PrintDbg("Read Y = 0x%02X") % (int) ctx->registers->Y;
    return ctx->registers->Y;
}