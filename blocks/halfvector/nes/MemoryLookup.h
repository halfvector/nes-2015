#pragma once

#include "Memory.h"
#include "Registers.h"
#include "MemoryStack.h"

struct InstructionContext {
    Memory *mem;
    Registers *registers;
    Stack* stack;
};

struct MemoryAddressResolveBase {
    static bool PageBoundaryCrossed;
};

/**
 * Lookup effective address
 */
template<int MemoryMode>
struct MemoryAddressResolve : MemoryAddressResolveBase {
    static tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        PrintWarning("MemoryAddressResolve<?>::GetEffectiveAddress(); Unimplemented Memory Mode: %s")
                % AddressModeTitle[MemoryMode];
        PageBoundaryCrossed = false;
        return 0;
    }
};

template<>
struct MemoryAddressResolve<ADDR_MODE_NONE> : MemoryAddressResolveBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        PrintWarning("MemoryAddressResolve<ADDR_MODE_NONE>::GetEffectiveAddress(); Unimplemented Memory Mode: %s")
                % AddressModeTitle[ADDR_MODE_NONE];
        PageBoundaryCrossed = false;

        NumOfCalls++;
        return 0;
    }
};

template<>
struct MemoryAddressResolve<ADDR_MODE_ZEROPAGE> : MemoryAddressResolveBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        tCPU::word effectiveAddress = ctx->mem->readByte(ctx->registers->LastPC + 1);
        PageBoundaryCrossed = false;

        NumOfCalls++;
        return effectiveAddress;
    }
};

template<>
struct MemoryAddressResolve<ADDR_MODE_ZEROPAGE_INDEXED_X> : MemoryAddressResolveBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        tCPU::word effectiveAddress = ctx->mem->readByte(ctx->registers->LastPC + 1) + ctx->registers->X;
        PageBoundaryCrossed = false;

        // zero-page address is only 1 byte, wrap around after additing value from X register
        effectiveAddress = (tCPU::word) 0xFF & effectiveAddress;

        NumOfCalls++;
        return effectiveAddress;
    }
};

template<>
struct MemoryAddressResolve<ADDR_MODE_ABSOLUTE> : MemoryAddressResolveBase {
    static int NumOfCalls;

    static inline tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        tCPU::word EffectiveAddress = ctx->mem->readWord(ctx->registers->LastPC + 1);
        PageBoundaryCrossed = false;

        NumOfCalls++;
        return EffectiveAddress;
    }
};

template<>
struct MemoryAddressResolve<ADDR_MODE_ABSOLUTE_INDEXED_X> : MemoryAddressResolveBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        //g_Memory.GetWordAfterPC();
        tCPU::word AbsoluteAddress = ctx->mem->readWord(ctx->registers->LastPC + 1);
        tCPU::word EffectiveAddress = AbsoluteAddress + ctx->registers->X;

        if ((AbsoluteAddress & 0xFF00) != (EffectiveAddress & 0xFF00)) {    // page boundary crossed?
            PageBoundaryCrossed = true;
            PrintDbg("< ADDR_MODE_ABSOLUTE_INDEXED_X > Page Boundary Crossed: $%04X + $%02X -> $%04X")
                    % AbsoluteAddress % ctx->registers->X % EffectiveAddress;
        } else
            PageBoundaryCrossed = false;

        NumOfCalls++;

        return EffectiveAddress;
    }
};

template<>
struct MemoryAddressResolve<ADDR_MODE_ABSOLUTE_INDEXED_Y> : MemoryAddressResolveBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        tCPU::word AbsoluteAddress = ctx->mem->readWord(ctx->registers->LastPC + 1);//g_Memory.GetWordAfterPC();
        tCPU::word EffectiveAddress = AbsoluteAddress + ctx->registers->Y;

        if ((AbsoluteAddress & 0xFF00) != (EffectiveAddress & 0xFF00)) {    // page boundary crossed?
            PageBoundaryCrossed = true;
//			PrintNotice( "< ADDR_MODE_ABSOLUTE_INDEXED_Y > Page Boundary Crossed: $%04X + $%02X -> $%04X", AbsoluteAddress, g_Registers.X, EffectiveAddress );
        } else
            PageBoundaryCrossed = false;

        NumOfCalls++;

        return EffectiveAddress;
    }
};

template<>
struct MemoryAddressResolve<ADDR_MODE_INDIRECT_INDEXED> : MemoryAddressResolveBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        // read byte after opcode as an address
        // the zeropage 8bit address contains the full 16bit address
        tCPU::word ZeroPageAddress = ctx->mem->readByte(ctx->registers->LastPC + 1);

        // fetch the 16bit address from zeropage address stored in the operand
        tCPU::word IndirectAddress = ctx->mem->readWord(ZeroPageAddress);

        // add the indexed offset (from register Y) to calculate the offset address
        tCPU::word IndexedIndirectAddress = IndirectAddress + ctx->registers->Y;

        if ((IndirectAddress & 0xFF00) != (IndexedIndirectAddress & 0xFF00)) {
            // page boundary crossed
            PageBoundaryCrossed = true;
            PrintDbg("< ADDR_MODE_INDIRECT_INDEXED > Page Boundary Crossed: $%04X + $%02X -> $%04X")
                    % IndirectAddress % ctx->registers->Y % IndexedIndirectAddress;
        } else {
            PageBoundaryCrossed = false;
        }

		PrintDbg("MemoryAddressResolve< ADDR_MODE_INDIRECT_INDEXED >::GetEffectiveAddress(); ZPA: $%04X, IA: $%04X, IIA: $%04X")
            % ZeroPageAddress % IndirectAddress % IndexedIndirectAddress;

        NumOfCalls++;

        // whew :D
        return IndexedIndirectAddress;
    }
};

// FIXME: double check this and one above
template<>
struct MemoryAddressResolve<ADDR_MODE_INDIRECT_ABSOLUTE> : MemoryAddressResolveBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext *ctx) {
        // address of where the real address is stored
        tCPU::word IndirectAddress = ctx->mem->readByte(ctx->registers->LastPC + 1);
        // the real address
        tCPU::word EffectiveAddress = ctx->mem->readWord(IndirectAddress);

        PageBoundaryCrossed = false;

        NumOfCalls++;

        return EffectiveAddress;
    }
};

/**
 * Templetized registers lookup
 */
template<ProcessorStatusFlags T>
struct ProcessorStatusFlag {
    static bool getState(InstructionContext *ctx) {
        PrintError("Unknown ProcessorStatusFlag Requested");
        return false;
    }
};
