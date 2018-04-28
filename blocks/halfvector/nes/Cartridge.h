#pragma once

#include <stdint.h>

const unsigned int PRG_ROM_PAGE_SIZE = 0x4000;
const unsigned int CHR_ROM_PAGE_SIZE = 0x2000;

struct RomHeader {
    uint8_t signature[4];
    uint8_t numPrgPages;
    uint8_t numChrPages;
    int8_t CB1;
    int8_t CB2;

    int8_t reserved[8];
};

enum eMirroringType {
    VERTICAL_MIRRORING, HORIZONTAL_MIRRORING
};

struct RomInfo {
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
        programDataPages = new PrgRomPage[20];
        characterDataPages = new ChrRomPage[10];
    }

    RomHeader header;
    RomInfo info;

    PrgRomPage* programDataPages;
    ChrRomPage* characterDataPages;
};
