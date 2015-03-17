#pragma once

#include "Memory.h"
#include "Registers.h"

struct InstructionContext {
    Memory *mem;
    Registers *registers;
};

struct tMemoryAddressLookupBase
{
    static bool PageBoundaryCrossed;
};

/**
* Lookup effective address
*/
template< int MemoryMode >
struct tMemoryAddressLookup : tMemoryAddressLookupBase {
    static tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        PrintWarning("tMemoryAddressLookup<?>::GetEffectiveAddress(); Unimplemented Memory Mode: %s")
                % AddressModeTitle[MemoryMode];
        PageBoundaryCrossed = false;
        return 0;
    }
};

template<>
struct tMemoryAddressLookup< ADDR_MODE_NONE > : tMemoryAddressLookupBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        PrintWarning("tMemoryAddressLookup<ADDR_MODE_NONE>::GetEffectiveAddress(); Unimplemented Memory Mode: %s")
                % AddressModeTitle[ADDR_MODE_NONE];
        PageBoundaryCrossed = false;

        NumOfCalls++;
        return 0;
    }
};

template<>
struct tMemoryAddressLookup< ADDR_MODE_ZEROPAGE > : tMemoryAddressLookupBase {
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        tCPU::word EffectiveAddress = ctx->mem->getRealMemoryAddress(ctx->registers->PC);//g_Memory.GetByteAfterPC();
        PageBoundaryCrossed = false;

        NumOfCalls++;
        return EffectiveAddress;
    }
};


template<>
struct tMemoryAddressLookup< ADDR_MODE_ZEROPAGE_INDEXED_X > : tMemoryAddressLookupBase
{
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        //g_Memory.GetByteAfterPC() + g_Registers.X;
        tCPU::word EffectiveAddress = ctx->mem->readByte(ctx->registers->LastPC+1) + ctx->registers->X;
        PageBoundaryCrossed = false;

        NumOfCalls++;
        return EffectiveAddress;
    }
};

template<>
struct tMemoryAddressLookup< ADDR_MODE_ABSOLUTE > : tMemoryAddressLookupBase
{
    static int NumOfCalls;

    static inline tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        tCPU::word EffectiveAddress = ctx->mem->readWord(ctx->registers->LastPC+1);
        PageBoundaryCrossed = false;

        NumOfCalls++;
        return EffectiveAddress;
    }
};

template<>
struct tMemoryAddressLookup< ADDR_MODE_ABSOLUTE_INDEXED_X > : tMemoryAddressLookupBase
{
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        //g_Memory.GetWordAfterPC();
        tCPU::word AbsoluteAddress = ctx->mem->readWord(ctx->registers->LastPC+1);
        tCPU::word EffectiveAddress = AbsoluteAddress + ctx->registers->X;

        if( (AbsoluteAddress & 0xFF00) != (EffectiveAddress & 0xFF00) )
        {	// page boundary crossed?
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
struct tMemoryAddressLookup< ADDR_MODE_ABSOLUTE_INDEXED_Y > : tMemoryAddressLookupBase
{
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        tCPU::word AbsoluteAddress = ctx->mem->readWord(ctx->registers->LastPC + 1);//g_Memory.GetWordAfterPC();
        tCPU::word EffectiveAddress = AbsoluteAddress + ctx->registers->Y;

        if( (AbsoluteAddress & 0xFF00) != (EffectiveAddress & 0xFF00) )
        {	// page boundary crossed?
            PageBoundaryCrossed = true;
//			PrintNotice( "< ADDR_MODE_ABSOLUTE_INDEXED_Y > Page Boundary Crossed: $%04X + $%02X -> $%04X", AbsoluteAddress, g_Registers.X, EffectiveAddress );
        } else
            PageBoundaryCrossed = false;

        NumOfCalls++;

        return EffectiveAddress;
    }
};

template<>
struct tMemoryAddressLookup< ADDR_MODE_INDIRECT_INDEXED > : tMemoryAddressLookupBase
{
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        // zeropage 8bit address of where the real 16bit address is
        tCPU::word ZeroPageAddress = ctx->mem->readByte(ctx->registers->LastPC + 1); //g_Memory.GetByteAfterPC();
        // fetch the lsb of the 16bit address from zeropage address
        tCPU::word IndirectAddress = ctx->mem->readWord(ZeroPageAddress); //g_Memory.GetWordAt( ZeroPageAddress );
        // add the indexed offset (from register Y) as the msb
        tCPU::word IndexedIndirectAddress = IndirectAddress + ctx->registers->Y;

        if( (IndirectAddress & 0xFF00) != (IndexedIndirectAddress & 0xFF00) )
        {	// page boundary crossed?
            PageBoundaryCrossed = true;
            PrintDbg( "< ADDR_MODE_INDIRECT_INDEXED > Page Boundary Crossed: $%04X + $%02X -> $%04X")
                    % IndirectAddress % ctx->registers->Y % IndexedIndirectAddress;
        } else
            PageBoundaryCrossed = false;

//		PrintDbg( "tMemoryAddressLookup< ADDR_MODE_INDIRECT_INDEXED >::GetEffectiveAddress(); ZPA: $%04X, IA: $%04X, IIA: $%04X", ZeroPageAddress, IndirectAddress, IndexedIndirectAddress );

        NumOfCalls++;

        // whew :D
        return IndexedIndirectAddress;
    }
};

// FIXME: double check this and one above
template<>
struct tMemoryAddressLookup< ADDR_MODE_INDIRECT_ABSOLUTE > : tMemoryAddressLookupBase
{
    static int NumOfCalls;

    static tCPU::word GetEffectiveAddress(InstructionContext* ctx) {
        // address of where the real address is stored
        tCPU::word IndirectAddress = ctx->mem->readByte(ctx->registers->LastPC + 1); //g_Memory.GetWordAfterPC();
        // the real address
        tCPU::word EffectiveAddress = ctx->mem->readWord(IndirectAddress); //g_Memory.GetWordAt( IndirectAddress );

        PageBoundaryCrossed = false;

        NumOfCalls++;

        return EffectiveAddress;
    }
};



/**
 * perform basic memory ops on effective memory address
 */
template<int MemoryMode>
struct tMemoryOperation : tMemoryAddressLookup<MemoryMode> {
    static tCPU::byte GetByte(InstructionContext *ctx) {
        return ctx->mem->readByte(tMemoryAddressLookup<MemoryMode>::GetEffectiveAddress(ctx));
    }

    static tCPU::word GetWord(InstructionContext *ctx) {
        return ctx->mem->readWord(tMemoryAddressLookup<MemoryMode>::GetEffectiveAddress(ctx));
    }

    static bool WriteByte(InstructionContext *ctx, tCPU::byte Value) {
        return ctx->mem->writeByte(tMemoryAddressLookup<MemoryMode>::GetEffectiveAddress(ctx), Value);
    }

    static bool WriteWord(InstructionContext *ctx, tCPU::word Value) {
        return ctx->mem->writeByte(tMemoryAddressLookup<MemoryMode>::GetEffectiveAddress(ctx), Value);
    }
};

// derives from tMemoryOperation< ADDR_MODE_NONE > in order to have an error printing
// GetEffectiveAddress call, since it is an error to use that call for these specific memory modes

// no writing to immediate operands :D
template<>
struct tMemoryOperation<ADDR_MODE_IMMEDIATE> : tMemoryOperation<ADDR_MODE_NONE> {
    static void WriteByte(InstructionContext *ctx, tCPU::byte Value) {
        PrintError("tMemoryOperation<ADDR_MODE_IMMEDIATE>::WriteByte(); Impossible Action in Immediate Mode");
    }

    static void WriteWord(InstructionContext *ctx, tCPU::word Value) {
        PrintError("tMemoryOperation<ADDR_MODE_IMMEDIATE>::WriteWord(); Impossible Action in Immediate Mode");
    }

    static tCPU::byte GetByte(InstructionContext *ctx) {
        tCPU::byte value = ctx->mem->readByte(ctx->registers->LastPC + 1);
        PrintDbg("tMemoryOperation<ADDR_MODE_IMMEDIATE>::GetByte(); Value = 0x%X") % (int)value;
        return value;
    }

    // this is not the same as absolute address
    // this CAN be a 16bit address, which is absolute, but the value is not a memory location of data
    // its an address to work on ITSELF.. eg jump to there
    static tCPU::word GetWord(InstructionContext *ctx) {
        tCPU::word value = ctx->mem->readWord(ctx->registers->LastPC + 1);
        PrintDbg("tMemoryOperation<ADDR_MODE_IMMEDIATE>::GetWord(); Value = 0x%X") % (int)value;
        return value;
    }
};

// working on the accumulator register -- not really memory :D
template<>
struct tMemoryOperation<ADDR_MODE_ACCUMULATOR> : tMemoryOperation<ADDR_MODE_NONE> {
    static void WriteByte(InstructionContext *ctx, tCPU::byte value) {
        ctx->registers->A = value;
        PrintDbg("tMemoryOperation<ACCUMULATOR>::WriteByte(); Value = 0x%X") % value;
    }

    static void WriteWord(tCPU::word Value) {
        PrintError("tMemoryOperation::WriteByte(); Impossible Action in Accumulator Mode");
    }

    static tCPU::byte GetByte(InstructionContext *ctx) {
        tCPU::byte value = ctx->registers->A;
        PrintDbg("tMemoryOperation<ACCUMULATOR>::GetByte(); Value = 0x%X") % value;
        return value;
    }

    static tCPU::word GetWord(InstructionContext *ctx) {
        PrintError("tMemoryOperation::GetWord(); Impossible Action in Accumulator Mode");
        return 0;
    }
};


// this is the same as normal immediate, except its to registers x/y (instead of accumulator?)
// and so the opcode generated is diff, +4h instead of -4h or something like that
template<> struct tMemoryOperation<ADDR_MODE_IMMEDIATE_TO_XY> : tMemoryOperation< ADDR_MODE_IMMEDIATE > { };

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
