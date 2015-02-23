#ifndef HALFVECTOR_NES_ROM_H
#define HALFVECTOR_NES_ROM_H

struct RomHeader {
    char signature[4];
    char programData;
    char characterData;
    char CB1;
    char CB2;

    char reserved[8];
};

struct RomInfo {
    enum eMirroringType {
        VERTICAL_MIRRORING, HORIZONTAL_MIRRORING
    };
    eMirroringType Mirroring;
    bool SRAM_Enabled;
    bool TrainPresent;
    bool FourScreenVRAM;

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

#endif