#pragma once

#include "Platform.h"
#include "Cartridge.h"

class Memory;

enum enumSpriteSize { SPRITE_SIZE_8x16 = 1, SPRITE_SIZE_8x8 = 0 };

struct PPU_Settings {
    /*
     * control register 1
     */
    bool GenerateInterruptOnSprite;
    bool GenerateInterruptOnVBlank;

    // Port 2007h VRAM Address Increment (1byte = horizontal, 32bytes = vertical)
    // since nametable is 32 bytes wide, skipping 32 bytes gets you down one row
    bool DoVerticalWrites;

    // 8x16 or 8x8 sprites
    enumSpriteSize SpriteSize;

    // base addresses for pattern tables
    tCPU::word SpritePatternTableAddress;
    tCPU::word BackgroundPatternTableAddress;

    // because of mirroring there are only two real name tables, but this can have
    // 4 diff values: 0x2000, 0x2400, 0x2800, 0x2C00
    tCPU::word NameTableAddress;

    /**
     * control register 2
     */
    bool DisplayTypeMonochrome;
    // dont show left 8 pixels
    bool BackgroundClipping;
    // invisible in left 8 pixel column
    bool SpriteClipping;
    bool BackgroundVisible;
    bool SpriteVisible;
};

class Raster {
public:
    Raster() {
        screenBuffer = new tCPU::byte[256 * 256 * 4];
        patternTable = new tCPU::byte[128 * 256 * 4];
        attributeTable = new tCPU::byte[256 * 256 * 4];
        backgroundMask = new tCPU::byte[256 * 256];
        spriteMask = new tCPU::byte[256 * 256];
    }

    tCPU::byte* screenBuffer;
    tCPU::byte* patternTable;
    tCPU::byte* attributeTable;
    tCPU::byte* backgroundMask;
    tCPU::byte* spriteMask;
};

class PPU {
public:
    PPU(Raster *);

    void loadRom(Cartridge &rom);
    void writeChrPage(uint8_t buffer[]);

    void execute(int numCycles);
    tCPU::byte getStatusRegister();

    void renderDebug();

    bool enteredVBlank() {
        return currentScanline == 243 && scanlinePixel == 0;
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

    tCPU::word GetEffectiveAddress(tCPU::word address);
    tCPU::byte ReadInternalMemoryByte(tCPU::word Address);
    bool WriteInternalMemoryByte(tCPU::word Address, tCPU::byte Value);
    void AutoIncrementVRAMAddress();

    void setVRamAddressRegister1(tCPU::byte value);
    void setSprRamAddress(tCPU::byte address);
    void writeSpriteMemory(tCPU::byte value);
    tCPU::byte readSpriteMemory();
    void StartSpriteXferDMA(Memory* memory, tCPU::byte address);

protected:
    tCPU::byte statusRegister;
    tCPU::byte controlRegister1;
    tCPU::word cycles;
    tCPU::word vramAddress14bit;
    tCPU::word tempVRAMAddress;
    tCPU::byte tileXOffset;
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
    tCPU::byte *PPU_RAM = new tCPU::byte[0x4000];
    tCPU::byte *SPR_RAM = new tCPU::byte[0x100];

    Raster *raster;

    // shared flipflop by port 2005 and 2006 to maintain first-write bit
    // reset by port 2002 reads
    bool firstWriteToSFF = true;
    tCPU::byte horizontalScrollOrigin;
    tCPU::byte verticalScrollOrigin;

    tCPU::word reloadBits;

    PPU_Settings settings;
    void setVerticalBlank();
    void advanceRenderableScanline();
    void advanceBlankScanline();
    void onEnterHBlank();
    void renderScanline(int scanline);

    tCPU::byte GetColorFromPalette(int PaletteType, int NameTableId, int ColorId);
    /*
     * PPU Settings
     */

    // port $2003, $2004
    tCPU::word spriteRamAddress = 0;

    // activated when generateInterruptOnVBlank is true and vblank interval entered
    bool vblankNmiAwaiting = false;
};
