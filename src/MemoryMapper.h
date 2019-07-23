#pragma once

#include "Platform.h"
#include "Cartridge.h"

static const int PRG_ROM_OFFSET = 0x10000;

class MemoryMapper {
public:
    MemoryMapper(unsigned char *ppuRam, unsigned char *cpuRam);

    void loadRom(Cartridge &rom);

    unsigned short getEffectivePPUAddress(unsigned short address);

    void writeByteCPUMemory(unsigned short address, unsigned char value);

    unsigned char readByteCPUMemory(unsigned short address);

private:
    unsigned char *PPU_RAM;
    unsigned char *CPU_RAM;
    unsigned char *PRG_BANKS;
    int memoryMapperId;
    int chrBank;
    int prgBank;
    int prgBankMask;

    // MMC1
    int control, chrBank0, chrBank1, shiftRegister;
    int prgBankMode, chrBankMode, mirroring;
};
