#pragma once

#include "Platform.h"
#include "Cartridge.h"

class Memory;

enum enumSpriteSize { SPRITE_SIZE_8x16 = 1, SPRITE_SIZE_8x8 = 0 };

struct PPU_Settings {
    /*
     * control register 1
     */
    bool GenerateInterruptOnSprite = false;
    bool GenerateInterruptOnVBlank = false;

    // Port 2007h VRAM Address Increment (1byte = horizontal, 32bytes = vertical)
    // since nametable is 32 bytes wide, skipping 32 bytes gets you down one row
    bool DoVerticalWrites = false;

    // 8x16 or 8x8 sprites
    enumSpriteSize SpriteSize = SPRITE_SIZE_8x8;

    // base addresses for pattern tables
    tCPU::word SpritePatternTableAddress = 0x1000;
    tCPU::word BackgroundPatternTableAddress = 0x0000;

    // because of mirroring there are only two real name tables, but this can have
    // 4 diff values: 0x2000, 0x2400, 0x2800, 0x2C00
    tCPU::word NameTableAddress = 0x2000;

    /**
     * control register 2
     */
    bool DisplayTypeMonochrome;
    // dont show left 8 pixels
    bool BackgroundClipping;
    // invisible in left 8 pixel column
    bool SpriteClipping;
    bool BackgroundVisible = false;
    bool SpriteVisible = false;
    eMirroringType mirroring;
};

class Raster {
public:
    Raster() {
        screenBuffer = new tCPU::byte[256 * 256 * 4];
        palette = new tCPU::byte[256 * 32 * 4];
        patternTable = new tCPU::byte[128 * 256 * 4];
        attributeTable = new tCPU::byte[256 * 256 * 4];
        backgroundMask = new tCPU::byte[256 * 256];
        spriteMask = new tCPU::byte[256 * 256];
        nametables = new tCPU::byte[512 * 512 * 4];
    }

    tCPU::byte* screenBuffer;
    tCPU::byte* palette;
    tCPU::byte* patternTable;
    tCPU::byte* attributeTable;
    tCPU::byte* backgroundMask;
    tCPU::byte* spriteMask;
    tCPU::byte* nametables;
};

enum MemoryMappers {
    MEMORY_MAPPER_NROM = 0,
    MEMORY_MAPPER_UNROM = 2,
    MEMORY_MAPPER_CNROM = 3,
};

class MemoryMapper {
public:
    MemoryMapper(tCPU::byte *ppuRam, tCPU::byte *cpuRam) {
        this->PPU_RAM = ppuRam;
        this->CPU_RAM = cpuRam;
        this->PRG_BANKS = new tCPU::byte[0x20000]; // 128KiB (ie megaman1)
        chrBank = 0;
        prgBank = 0;
    }

    void loadRom(Cartridge &rom);

    tCPU::word getEffectivePPUAddress(tCPU::word address);

    void writeByteCPUMemory(tCPU::word address, tCPU::byte value);

    tCPU::byte readByteCPUMemory(tCPU::word address);

private:
    tCPU::byte *PPU_RAM;
    tCPU::byte *CPU_RAM;
    tCPU::byte *PRG_BANKS;
    int memoryMapperId;
    int chrBank;
    int prgBank;
    int prgBankMask;
};

class PPU {
public:
    PPU(Raster *);

    void loadRom(Cartridge &rom);
    void writeChrPage(uint16_t page, uint8_t buffer[]);
    void clear();

    void execute(int numCycles);
    tCPU::byte getStatusRegister();

    void renderDebug();

    bool enteredVBlank() {
//        return currentScanline == 243 && scanlinePixel == 0;
        auto wasInVBlank = inVBlank;
        inVBlank = false;
        return wasInVBlank;
    }

    /**
     * CPU pulls NMI line and resets it in the process.
     */
    bool pullNMI() {
        bool nmi = vblankNmiAwaiting;
        vblankNmiAwaiting = false;
        return nmi;
    }

    void setControlRegister1(tCPU::byte value);
    void setControlRegister2(tCPU::byte value);

    tCPU::byte getControlRegister1();

    void setVRamAddressRegister2(tCPU::byte value);
    void writeToVRam(tCPU::byte value);
    tCPU::byte readFromVRam();

    // calculate memory address in PPU ram, taking into account mirroring
    tCPU::word GetEffectiveAddress(tCPU::word address);

    tCPU::byte ReadByteFromPPU(tCPU::word Address);
    bool WriteByteToPPU(tCPU::word Address, tCPU::byte Value);
    void AutoIncrementVRAMAddress();

    void setVRamAddressRegister1(tCPU::byte value);
    void setSprRamAddress(tCPU::byte address);
    void writeSpriteMemory(tCPU::byte value);
    tCPU::byte readSpriteMemory();
    void StartSpriteXferDMA(Memory* memory, tCPU::byte address);

    tCPU::byte* getPpuRam() {
        return PPU_RAM;
    }

    void useMemoryMapper(MemoryMapper *mapper);

protected:
    tCPU::byte statusRegister;
    tCPU::byte controlRegister1;
    tCPU::word cycles;
    tCPU::word vramAddress14bit;
    tCPU::word tempVRAMAddress;
    tCPU::byte latchedVRAMByte;

    // states
    bool sprite0HitInThisScanline;
    bool sprite0HitInThisFrame;
    bool inHBlank, inVBlank;
    int currentScanline;
    int scanlinePixel;

    // memory
    tCPU::byte *WRAM = new tCPU::byte[2000];
    tCPU::byte *VRAM = new tCPU::byte[2000];
    tCPU::byte *PPU_RAM = new tCPU::byte[0x10000]; // 65KiB memory. can hold MMC roms.
    tCPU::byte *SPR_RAM = new tCPU::byte[0x100];

    Raster *raster;

    // separate flipflop bits for ports 2005 and 2006 to maintain first-write state
    // reset by port 2002 reads. should this be shared?
    bool firstWriteToSFF = true;
    bool firstWriteToSFF2 = true;
    tCPU::byte horizontalScrollOrigin;
    tCPU::byte verticalScrollOrigin;

    tCPU::word reloadBits;

    PPU_Settings settings;
    void setVerticalBlank();
    void advanceRenderableScanline();
    void advanceBlankScanline();
    void onEnterHBlank();
    void renderScanline(const tCPU::word scanline);

    tCPU::byte GetColorFromPalette(int paletteType, int upperBits, int lowerBits);
    /*
     * PPU Settings
     */

    // port $2003, $2004
    tCPU::word spriteRamAddress = 0;

    // activated when generateInterruptOnVBlank is true and vblank interval entered
    bool vblankNmiAwaiting = false;

    void RenderDebugNametables();

    void RenderDebugColorPalette();

    void RenderDebugAttributes(tCPU::word NametableAddress);

    void RenderDebugPatternTables();

    void RenderBackgroundTiles();

    void RenderSpriteTiles();

    MemoryMapper *mapper = nullptr;
};
