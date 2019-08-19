#pragma once

#include <stdint.h>

const unsigned int PRG_ROM_PAGE_SIZE = 0x4000; // 16K
const unsigned int CHR_ROM_PAGE_SIZE = 0x2000; // 8K

#pragma pack(push, 1)
struct RomHeader {
    uint8_t signature[4];
    uint8_t numPrgPages;
    uint8_t numChrPages;
    uint8_t CB1;
    uint8_t CB2;

    uint8_t reserved[8];
};
#pragma pack(pop)

enum eMirroringType {
    VERTICAL_MIRRORING, HORIZONTAL_MIRRORING, NONE
};

struct RomInfo {
    int memoryMapperId;
    eMirroringType mirroring;
    bool sramEnabled;
    bool trainerPresent;
    bool fourScreenVRAM;
};

struct PrgRomPage {
    uint8_t* buffer = new uint8_t[PRG_ROM_PAGE_SIZE];
};

struct ChrRomPage {
    uint8_t* buffer = new uint8_t[CHR_ROM_PAGE_SIZE];
};

struct Cartridge {
    Cartridge() {
        programDataPages = new PrgRomPage[32];
        characterDataPages = new ChrRomPage[32];
    }

    RomHeader header;
    RomInfo info;

    PrgRomPage* programDataPages;
    ChrRomPage* characterDataPages;
};
