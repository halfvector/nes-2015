#include "CartridgeLoader.h"
#include "Logging.h"

/**
 * Load ROM from file path
 */
Cartridge
CartridgeLoader::loadCartridge(char const *filePath) {
    std::fstream fh;
    fh.open(filePath, std::fstream::in | std::fstream::binary);

    if (!fh.is_open())
        ThrowException("failed to open rom: %s", filePath);

    Cartridge rom;

    readHeader(fh, rom);
    readData(fh, rom);

    fh.close();

    return rom;
}

/**
 * read header and validate signature
 */
void
CartridgeLoader::readHeader(std::fstream &fh, Cartridge &rom) {
    fh.read((char *) &rom.header, sizeof(rom.header));

    PrintInfo("ROM Header size %d", sizeof(rom.header));

    // signature should say 'NES'
    PrintInfo("signature: %c%c%c", rom.header.signature[0], rom.header.signature[1], rom.header.signature[2]);
    PrintInfo("number of PRG pages: %d", (int) rom.header.numPrgPages);
    PrintInfo("number of CHR pages: %d", (int) rom.header.numChrPages);
    PrintInfo("control bytes: 0x%02X 0x%02X", rom.header.CB1, rom.header.CB2);
    PrintInfo("iNES 2.0 header? %d", ((rom.header.CB2 >> 2) & 0x3) == 0x3);

    bool headerContainsReservedBits = false;
    for (signed char i : rom.header.reserved) {
        headerContainsReservedBits |= i;
    }

    // warn if header contains reserved bits (not supported)
    if (headerContainsReservedBits) {
        PrintWarning("header is using reserved bits");
    }

    analyzeHeader(rom);
}

/**
 * Parse and test header
 */
void
CartridgeLoader::analyzeHeader(Cartridge &rom) {
    char CB1 = rom.header.CB1;

    PrintFlagState(CB1, 0x01, "Mirroring", "Vertical", "Horizontal");
    PrintBitState(CB1, 0x02, "SRAM Enabled");
    PrintBitState(CB1, 0x04, "512-byte trainer present");
    PrintBitState(CB1, 0x08, "Four-screen VRAM layout");

    rom.info.mirroring = CB1 & 0x01 ? VERTICAL_MIRRORING : HORIZONTAL_MIRRORING;
    rom.info.sramEnabled = (CB1 & 0x02) != 0;
    rom.info.trainerPresent = (CB1 & 0x04) != 0;
    rom.info.fourScreenVRAM = (CB1 & 0x08) != 0;

    // sanity check
    assert(rom.header.numPrgPages < 10);
    assert(rom.header.numChrPages < 10);

    // determine memory mapper type (256 possible variants)
    int mapperId = ((rom.header.CB1 & 0xF0) >> 4) | (rom.header.CB2 & 0xF0);
    PrintInfo("ROM requires Memory Mapper #%d", mapperId);
    PrintInfo("CB1: 0x%X", rom.header.CB1);
    PrintInfo("CB2: 0x%X", rom.header.CB2);

    rom.info.memoryMapperId = mapperId;

    if (rom.info.trainerPresent) {
        PrintInfo("512 Byte Trainer Present");
    } else {
        PrintInfo("No Trainer Present");
    }
}

/**
* Load actual ROM contents
*/
void
CartridgeLoader::readData(std::fstream &fh, Cartridge &rom) {
    // read prg pages
    PrintInfo("Reading %d PRG-ROM Pages (%d bytes)", (int) rom.header.numPrgPages,
              (rom.header.numPrgPages * PRG_ROM_PAGE_SIZE));

    for (uint8_t i = 0; i < rom.header.numPrgPages; i++) {
        fh.read((char *) rom.programDataPages[i].buffer, PRG_ROM_PAGE_SIZE);
    }

    PrintInfo("Reading %d CHR-ROM Pages (%d bytes)", (int) rom.header.numChrPages,
              (rom.header.numChrPages * CHR_ROM_PAGE_SIZE));

    for (uint8_t i = 0; i < rom.header.numChrPages; i++) {
        fh.read((char *) rom.characterDataPages[i].buffer, CHR_ROM_PAGE_SIZE);
    }

    // bytes read so far
    int64_t totalRead = fh.tellg();

    // total file length
    fh.seekg(0, fh.end);
    int64_t fileLength = fh.tellg();

    // assert no content left unread in rom file
    if (totalRead != fileLength) {
        PrintWarning("ROM contains unprocessed data (read %d of %d bytes)", totalRead, fileLength);
    }
}
