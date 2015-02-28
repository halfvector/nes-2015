#pragma once

const unsigned int PRG_ROM_PAGE_SIZE = 16384;
const unsigned int CHR_ROM_PAGE_SIZE = 8192;

struct RomHeader {
    char signature[4];
    unsigned char programData;
    unsigned char characterData;
    char CB1;
    char CB2;

    char reserved[8];
};

struct RomInfo {
    enum eMirroringType {
        VERTICAL_MIRRORING, HORIZONTAL_MIRRORING
    };
    eMirroringType mirroring;
    bool sramEnabled;
    bool trainerPresent;
    bool fourScreenVRAM;

    unsigned char numPrgPages;
    unsigned char numChrPages;
};

struct PrgRomPage {
    unsigned char buffer[16384];
};

struct ChrRomPage {
    unsigned char buffer[8192];
};

struct Cartridge {
    RomHeader header;
    RomInfo info;

    PrgRomPage programDataPages[20];
    ChrRomPage characterDataPages[10];
};
