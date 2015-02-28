#include <iostream>
#include "boost/format.hpp"
#include "boost/filesystem.hpp"
#include "CartridgeLoader.h"
#include "Logging.h"

/**
* Load ROM from filepath
*/
Cartridge
CartridgeLoader::readCartridge(char const *string) {
    std::fstream fh;
    fh.open(string, std::fstream::in | std::fstream::binary);

    if (!fh.is_open())
        throw std::runtime_error((boost::format("failed to open rom: %s") % string).str());

    Cartridge rom;

    readHeader(fh, rom);
    analyzeHeader(rom);
    readData(fh, rom);

    fh.close();

    return rom;
}

/**
* Load ROM header and validate signature
*/
void
CartridgeLoader::readHeader(std::fstream &fh, Cartridge &rom) {
    fh.read((char *) &rom.header, sizeof(rom.header));

    PrintInfo("signature: %d") % rom.header.signature;
    PrintInfo("programData: 0x%02x") % rom.header.programData;
    PrintInfo("characterData: 0x%02x") % rom.header.characterData;

    bool headerIsClean = true;
    for (unsigned int i = 0; i < 8; i++) {
        headerIsClean |= rom.header.reserved[i];
    }

    // warn if header contains reserved bits (not supported)
    if (!headerIsClean) {
        PrintWarning("header is using reserved bits");
    }
}

/**
* Parse meaning info out of header
*/
void
CartridgeLoader::analyzeHeader(Cartridge &rom) {
    char CB1 = rom.header.CB1;

    PrintFlagState(CB1, 0x01, "Mirroring", "Vertical", "Horizontal");
    PrintBitState(CB1, 0x02, "SRAM Enabled");
    PrintBitState(CB1, 0x04, "512-byte trainer present");
    PrintBitState(CB1, 0x08, "Four-screen VRAM layout");

    rom.info.mirroring = CB1 & 0x01 ? RomInfo::VERTICAL_MIRRORING : RomInfo::HORIZONTAL_MIRRORING;
    rom.info.sramEnabled = (CB1 & 0x02) != 0;
    rom.info.trainerPresent = (CB1 & 0x04) != 0;
    rom.info.fourScreenVRAM = (CB1 & 0x08) != 0;

    rom.info.numPrgPages = rom.header.programData;
    rom.info.numChrPages = rom.header.characterData;

    int MapperId = ((rom.header.CB1 & 0xF0) >> 4) + ((rom.header.CB2 & 0xF0) >> 4);
    LOG(INFO) << "Rom requires Memory Mapper #" << MapperId;

    if (rom.info.trainerPresent) {    // read the 512 bytes trainer
        LOG(INFO) << "512 Byte Trainer Found";
    } else
        LOG(INFO) << "No 512 Byte Trainer Present";
}

/**
* Load actual ROM contents
*/
void
CartridgeLoader::readData(std::fstream &fh, Cartridge &rom) {
    // sanity check
    assert(rom.info.numPrgPages < 10);
    assert(rom.info.numChrPages < 10);

    // read prg pages
    PrintInfo("Reading %d PRG-ROM Pages (%d bytes)") % (int) rom.info.numPrgPages % (rom.info.numPrgPages * PRG_ROM_PAGE_SIZE);
    for (unsigned int i = 0; i < rom.info.numPrgPages; i++) {
        fh.read((char *) rom.programDataPages[i].buffer, PRG_ROM_PAGE_SIZE);
    }

    PrintInfo("Reading %d CHR-ROM Pages (%d bytes)") % (int) rom.info.numChrPages % (rom.info.numChrPages * CHR_ROM_PAGE_SIZE);
    for (unsigned int i = 0; i < rom.info.numChrPages; i++) {
        fh.read((char *) rom.characterDataPages[i].buffer, CHR_ROM_PAGE_SIZE);
    }

    // bytes read so far
    int totalRead = fh.tellg();

    // total file length
    fh.seekg(0, fh.end);
    int fileLength = fh.tellg();

    // assert no content left unread in rom file
    if (totalRead != fileLength) {
        throw std::runtime_error((boost::format("rom file contains unread data")).str());
    }
}
