#pragma once

#include "Platform.h"
#include "Cartridge.h"
#include "PPU.h"

static const int PRG_ROM_OFFSET = 0x10000;

enum MemoryMappers {
    MEMORY_MAPPER_NROM = 0,
    MEMORY_MAPPER_SXROM = 1, // MMC1
    MEMORY_MAPPER_UNROM = 2,
    MEMORY_MAPPER_CNROM = 3,
};

class MemoryMapper {
public:
    MemoryMapper(unsigned char *ppuRam, unsigned char *cpuRam);

    void loadRom(Cartridge &rom);

    unsigned short getEffectivePPUAddress(unsigned short address);

    void writeByteCPUMemory(unsigned short address, unsigned char value);

    unsigned char readByteCPUMemory(unsigned short address);

    bool supportsMirroringMode() {
        return memoryMapperId == MEMORY_MAPPER_SXROM;
    }

    eMirroringType getMirroringMode() {
        if (mirroring == 2) {
            return VERTICAL_MIRRORING;
        }
        if (mirroring == 3) {
            return HORIZONTAL_MIRRORING;
        }

        return NONE;
    }

private:
    unsigned char *PPU_RAM;
    unsigned char *CPU_RAM;
//    unsigned char *PRG_BANKS;
    int memoryMapperId;
    int chrBank;
    uint16_t prgBank;
    int prgBankMask;

    int numPrgBanks;

    // MMC1
    uint16_t control, chrBank0, chrBank1, shiftRegister;
    int useLargeChrBankMode, mirroring, prgBankMode;
    enum { SWAP_UPPER, SWAP_LOWER } prgBankSwapMode;
    enum { SWAP_16K, SWAP_32K } prgBankSwapSize;
};
