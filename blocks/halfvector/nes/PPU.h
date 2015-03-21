#pragma once

#include "Platform.h"

class PPU {
public:
    PPU();
    void execute(int numCycles);
    tCPU::byte getStatusRegister();

protected:
    tCPU::byte statusRegister;
    tCPU::word cycles;
    tCPU::word vramAddress14bit;
    tCPU::word tempVRAMAddress;
    tCPU::byte tileXOffset;
    tCPU::byte latchedVRAMByte;

    // states
    bool sprite0HitInThisScanline;
    bool sprite0HitInThisFrame;
    bool inHBlank, inVBlank;
    int currentScanline;
    int scanlinePixel;

    // memory
    tCPU::byte WRAM[2000];
    tCPU::byte VRAM[2000];
    tCPU::byte PPU_RAM[0x4000];
    tCPU::byte* SPR_RAM;

    // shared flipflop by port 2005 and 2006 to maintain first-write bit
    // reset by port 2002 reads
    bool firstWriteToSFF;
    tCPU::byte horizontalScrollOrigin;
    tCPU::byte verticalScrollOrigin;
    tCPU::word reloadBits;
};
