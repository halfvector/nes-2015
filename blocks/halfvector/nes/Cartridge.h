#pragma once

#include <stdint.h>

const unsigned int PRG_ROM_PAGE_SIZE = 16384;
const unsigned int CHR_ROM_PAGE_SIZE = 8192;

struct RomHeader {
    int8_t signature[4];
    uint8_t programData;
    uint8_t characterData;
    int8_t CB1;
    int8_t CB2;

    int8_t reserved[8];
};

struct RomInfo {
    enum eMirroringType {
        VERTICAL_MIRRORING, HORIZONTAL_MIRRORING
    };
    eMirroringType mirroring;
    bool sramEnabled;
    bool trainerPresent;
    bool fourScreenVRAM;

    uint8_t numPrgPages;
    uint8_t numChrPages;
};

struct PrgRomPage {
    uint8_t buffer[16384];
};

struct ChrRomPage {
    uint8_t buffer[8192];
};

struct Cartridge {
    RomHeader header;
    RomInfo info;

    PrgRomPage programDataPages[20];
    ChrRomPage characterDataPages[10];
};
