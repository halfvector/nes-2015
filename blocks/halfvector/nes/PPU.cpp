#include "PPU.h"
#include "Registers.h"
#include "Logging.h"
#include "Memory.h"
#include "Cartridge.h"
#include <math.h>
#include <bitset>

struct tPaletteEntry {
    union {
        struct {
            uint8_t R, G, B, A;
        };

        uint32_t ColorValue;
    };
};

tPaletteEntry colorPalette[64] = {
        {0x80, 0x80, 0x80, 0xFF},
        {0x00, 0x00, 0xBB, 0xFF},
        {0x37, 0x00, 0xBF, 0xFF},
        {0x84, 0x00, 0xA6, 0xFF},
        {0xBB, 0x00, 0x6A, 0xFF},
        {0xB7, 0x00, 0x1E, 0xFF},
        {0xB3, 0x00, 0x00, 0xFF},
        {0x91, 0x26, 0x00, 0xFF},
        {0x7B, 0x2B, 0x00, 0xFF},
        {0x00, 0x3E, 0x00, 0xFF},
        {0x00, 0x48, 0x0D, 0xFF},
        {0x00, 0x3C, 0x22, 0xFF},
        {0x00, 0x2F, 0x66, 0xFF},
        {0x00, 0x00, 0x00, 0xFF},
        {0x05, 0x05, 0x05, 0xFF},
        {0x05, 0x05, 0x05, 0xFF},

        {0xC8, 0xC8, 0xC8, 0xFF},
        {0x00, 0x59, 0xFF, 0xFF},
        {0x44, 0x3C, 0xFF, 0xFF},
        {0xB7, 0x33, 0xCC, 0xFF},
        {0xFF, 0x33, 0xAA, 0xFF},
        {0xFF, 0x37, 0x5E, 0xFF},
        {0xFF, 0x37, 0x1A, 0xFF},
        {0xD5, 0x4B, 0x00, 0xFF},
        {0xC4, 0x62, 0x00, 0xFF},
        {0x3C, 0x7B, 0x00, 0xFF},
        {0x1E, 0x84, 0x15, 0xFF},
        {0x00, 0x95, 0x66, 0xFF},
        {0x00, 0x84, 0xC4, 0xFF},
        {0x11, 0x11, 0x11, 0xFF},
        {0x09, 0x09, 0x09, 0xFF},
        {0x09, 0x09, 0x09, 0xFF},

        {0xFF, 0xFF, 0xFF, 0xFF},
        {0x00, 0x95, 0xFF, 0xFF},
        {0x6F, 0x84, 0xFF, 0xFF},
        {0xD5, 0x6F, 0xFF, 0xFF},
        {0xFF, 0x77, 0xCC, 0xFF},
        {0xFF, 0x6F, 0x99, 0xFF},
        {0xFF, 0x7B, 0x59, 0xFF},
        {0xFF, 0x91, 0x5F, 0xFF},
        {0xFF, 0xA2, 0x33, 0xFF},
        {0xA6, 0xBF, 0x00, 0xFF},
        {0x51, 0xD9, 0x6A, 0xFF},
        {0x4D, 0xD5, 0xAE, 0xFF},
        {0x00, 0xD9, 0xFF, 0xFF},
        {0x66, 0x66, 0x66, 0xFF},
        {0x0D, 0x0D, 0x0D, 0xFF},
        {0x0D, 0x0D, 0x0D, 0xFF},

        {0xFF, 0xFF, 0xFF, 0xFF},
        {0x84, 0xBF, 0xFF, 0xFF},
        {0xBB, 0xBB, 0xFF, 0xFF},
        {0xD0, 0xBB, 0xFF, 0xFF},
        {0xFF, 0xBF, 0xEA, 0xFF},
        {0xFF, 0xBF, 0xCC, 0xFF},
        {0xFF, 0xC4, 0xB7, 0xFF},
        {0xFF, 0xCC, 0xAE, 0xFF},
        {0xFF, 0xD9, 0xA2, 0xFF},
        {0xCC, 0xE1, 0x99, 0xFF},
        {0xAE, 0xEE, 0xB7, 0xFF},
        {0xAA, 0xF7, 0xEE, 0xFF},
        {0xB3, 0xEE, 0xFF, 0xFF},
        {0xDD, 0xDD, 0xDD, 0xFF},
        {0x11, 0x11, 0x11, 0xFF},
        {0x11, 0x11, 0x11, 0xFF}
};

tCPU::byte screenAttributes[16][16];

PPU::PPU(Raster *raster) : raster(raster) {
    firstWriteToSFF = true, sprite0HitInThisFrame = false, sprite0HitInThisScanline = false,
    inHBlank = false, inVBlank = false;

    currentScanline = 0, scanlinePixel = 0, cycles = 0, vramAddress14bit = 0,
    tempVRAMAddress = 0, tileXOffset = 0, latchedVRAMByte = 0, statusRegister = 0,
    horizontalScrollOrigin = 0, verticalScrollOrigin = 0, reloadBits = 0;

    // clear memory
    memset(SPR_RAM, 248, 256);
    memset(PPU_RAM, 0, 0x4000);
}

/**
 * Read Status register ($2002)
 */
tCPU::byte
PPU::getStatusRegister() {
    tCPU::byte returnValue = statusRegister;

    PrintPpu("PPU; Status register: %s", std::bitset<8>(statusRegister).to_string().c_str());
    PrintPpu("PPU; -> Scanline: %d, HBlank: %d, Pixel: %d", currentScanline, inHBlank, scanlinePixel);

    // reset vblank on register read
    statusRegister &= ~(1 << 7);

    // reset $2005 and $2006 write modes
    firstWriteToSFF = true;

//    // detect sprite 0 collision with current scanline
//    for(int i = 0; i < 64; i++ ){
//        SPR_RAM[i * 4];
//    }

    return returnValue;
}

void
PPU::execute(int numCycles) {

    /**
     *   0-239 = rendering scanlines
     *     240 = idle scanline
     * 241-260 = vertical blanking
     */

    for (int i = 0; i < numCycles; i++) {
        if (currentScanline < 240) {
            // process scanline
            advanceRenderableScanline();
        } else if (currentScanline < 243) {
            advanceBlankScanline();
        } else if (currentScanline < 262) {
            if (currentScanline == 243 && scanlinePixel == 0) {
                setVerticalBlank();
            }
            advanceBlankScanline();
        } else {
            // end of vblank
            currentScanline = 0;
            scanlinePixel = 0;
            inHBlank = false;
            inVBlank = false;

            // reset sprite-0 hit
            statusRegister &= ~(1 << 6);

            // reset vblank and overflow flags
            statusRegister &= ~(1 << 7);
            statusRegister &= ~(1 << 5);

            sprite0HitInThisFrame = false;
            sprite0HitInThisScanline = false;
        }
    }
}

/**
 * Vertical Blanking Interval started
 */
void
PPU::setVerticalBlank() {
    statusRegister |= 1 << 7; // set vblank bit

    // generate nmi trigger if we have one pending
    if (settings.GenerateInterruptOnVBlank) {
        vblankNmiAwaiting = true;
        settings.GenerateInterruptOnVBlank = false;
    }
}

/**
 * Advance VBlank/HBlank scanline
 */
void
PPU::advanceBlankScanline() {
    // process hblank until we reset to the start of the next scanline
    if (++scanlinePixel == 341) {
        currentScanline++;
        scanlinePixel = 0;
    }
}

/**
 * Advance renderable scanline
 * a pixel at a time on the scanline of life
 */
void
PPU::advanceRenderableScanline() {
    if (scanlinePixel < 255) {
        // drawing pixels
        inHBlank = false;
    } else if (scanlinePixel == 255) {
        // last pixel of the scanline. enter hblank.
        // TODO: perform sprite-0 hit detection
        inHBlank = true;
        onEnterHBlank();
    } else if (scanlinePixel < 340) {
        // in hblank
        inHBlank = true;
    } else {
        // leave hblank and increment scanline
        inHBlank = false;
        currentScanline++;
        scanlinePixel = -1;

        // copy all horizontal scrolling information from temp to vram addy
        if (settings.BackgroundVisible && settings.SpriteVisible) {
            vramAddress14bit &= ~0x041F; // zero out position bits
            vramAddress14bit |= (tempVRAMAddress & 0x041F); // copy over just the position bits
            vramAddress14bit &= 0x7FFF; // ensure 15th bit is zero

            PrintInfo("Start of Scanline; vramAddress14bit = 0x%X / tempVRAMAddress = %s",
                      vramAddress14bit, std::bitset<16>(vramAddress14bit).to_string().c_str());
        }

    }

    scanlinePixel++;
}

void PPU::onEnterHBlank() {
    renderScanline(currentScanline);

    // scanline somewhere within sprite 0
    if (sprite0HitInThisFrame) {
        // if we already matched a sprite0 hit this frame
        // dont bother with any hit-detection this frame
        return;
    }

    // check if this scanline hit sprite0
    tCPU::byte Y = SPR_RAM[0] + 1;
    tCPU::byte Tile = SPR_RAM[1];
    tCPU::byte attributes = SPR_RAM[2];
    tCPU::byte X = SPR_RAM[3];

    // ignore sprite if Y is set to 249
    if (Y == 249) {
        return;
    }

    if (Y >= 247) {
        return;
    }

    if (currentScanline >= Y && currentScanline <= (Y + 8)) {
        if (!sprite0HitInThisFrame && sprite0HitInThisScanline) {
            statusRegister |= Bit<6>::Set(true);
            sprite0HitInThisFrame = true;
            sprite0HitInThisScanline = false;
        }
    }
}

void PPU::renderScanline(int Y) {
    int X = 0;
    int i, j;

    int reverseY = Y;

    tCPU::byte UpperBits = 0;

    //return (Y * Width + X) * 3;
    unsigned char *backgroundBufferPtr = raster->backgroundMask + (Y * 256);
    unsigned char *screenBufferPtr = raster->screenBuffer + (Y * 256) * 4;

    // moving these outside the 32x loop shaves off 10kcs
    j = (int) floor(Y / 8.0);
    int yOffset = (Y % 8);

    int attributeY = (int) floor(Y / 32.0);


    //int scrollingOffset = (int) floor(horizontalScrollOrigin / 8);
    //unsigned char* ppuRamTileBasePtr = PPU_RAM + NametableAddress + j * 32 + scrollingOffset;

    // decode scanline
    // 256 pixels in 32 bytes, each byte a tile consisting of 8 pixels

    int lastAttributeNumber = -1, lastTileNumber = -1;
    tCPU::byte AttributeByte;
    tCPU::byte *ppuRamBasePtr = PPU_RAM + settings.BackgroundPatternTableAddress + yOffset;

    tCPU::word NametableAddress = settings.NameTableAddress;
    int NametableId = (NametableAddress - 0x2000) / 0x400;

    if (horizontalScrollOrigin >= 255) {
        if (NametableId == 0)
            NametableAddress += 0x400;
        else
            NametableAddress -= 0x400;

        NametableId = (NametableAddress - 0x2000) / 0x400;
    }

    int TileAddress = NametableAddress + j * 32 + horizontalScrollOrigin / 8;

    tCPU::byte *tileRamBasePtr = PPU_RAM + TileAddress;

    // only need to warp around once

    static tCPU::byte TileCache[33];
    static tCPU::byte AttributeByteCache[33];

    // tiles are 8x8, so we can reuse a row of tiles for 8 scanlines
    // and i think attributes for 16?
    if ((Y % 8) == 0) {
        // cache the next 8 scanlines worth of tiles!
        for (i = 0; i < 33; i++) {
            int ScrolledX = i * 8 + horizontalScrollOrigin;

            // horizontal scrolled tile position
            int ScrolledI = ScrolledX / 8;

            // only need to wrap around once per scanline (i hope..)
            if (ScrolledX >= 256) {
                NametableAddress = settings.NameTableAddress;
                NametableId = (NametableAddress - 0x2000) / 0x400;

                if (NametableId == 0)
                    NametableAddress += 0x400;
                else
                    NametableAddress -= 0x400;

                ScrolledX %= 256;
                ScrolledI = ScrolledX / 8;
            }


            tCPU::byte Tile = PPU_RAM[NametableAddress + j * 32 + ScrolledI];

            TileCache[i] = Tile;

            int attributeX = ScrolledI / 4;

            //if( lastAttributeNumber != attributeX)
            {
                lastAttributeNumber = attributeX;
                AttributeByte = *(PPU_RAM + NametableAddress + 0x3C0 + attributeY * 8 + attributeX);
                // upper bites will change every 8 pixels
                UpperBits = (AttributeByte >> ((ScrolledI & 2) | ((j & 2) << 1))) & 3;
            }

            AttributeByteCache[i] = UpperBits * 4; // prescale them
        }
    }

    tCPU::byte defaultPaletteId = PPU_RAM[0x3F00];

    static tCPU::byte pixelPaletteStream[256];
    int currentPixel = 0;

    // only sequential access here :D
    for (i = 0; i <= 32; i++) {
        if (i == 32 && (horizontalScrollOrigin % 8) == 0)
            break;

        tCPU::byte Tile = TileCache[i];
        tCPU::byte preScaledUpperBits = AttributeByteCache[i];

        // 8-pixel row within the tile (Y%8)
        tCPU::byte PatternByte0 = PPU_RAM[settings.BackgroundPatternTableAddress + Tile * 16 + 8 * 0 + yOffset];
        tCPU::byte PatternByte1 = PPU_RAM[settings.BackgroundPatternTableAddress + Tile * 16 + 8 * 1 + yOffset];

        int startX = 0;
        int endX = 8;

        if (i == 0)
            startX = horizontalScrollOrigin % 8;
        if (i == 32)
            endX = horizontalScrollOrigin % 8;

        tCPU::byte shiftedPattern0 = PatternByte0;
        tCPU::byte shiftedPattern1 = PatternByte1;

        // no more inner loop branches! -20k cycles per scanline
        for (X = startX; X < endX; X++) {
            int xOffset = 7 - X;

            // this pixel has two color bits from the pattern table available
            tCPU::byte PixelBit0 = (PatternByte0 >> xOffset) & 1;
            tCPU::byte PixelBit1 = (PatternByte1 >> xOffset) & 1;

            tCPU::byte ColorId = PixelBit0 | (PixelBit1 << 1);

            pixelPaletteStream[currentPixel++] = ColorId == 0 ? 0 : ColorId + preScaledUpperBits;

            *(backgroundBufferPtr++) = ColorId;
//            *screenBufferPtr = ColorId;
//            screenBufferPtr += 4;
        }
    }

    // now we have to jump around memory to fetch the palette ids
    // its 1.5x faster doing it here than in the loop above :>

    // do it backwards to make bitblts more native

    int *screenBufferPtr32bit = (int *) (raster->screenBuffer + (reverseY * 256) * 4);

    for (i = 0; i < 256; i++) {
        tCPU::byte PaletteId = PPU_RAM[0x3F00 + pixelPaletteStream[i]];

        tPaletteEntry &RGBColor = colorPalette[PaletteId];
        *(screenBufferPtr32bit++) = 0xFF << 24 | RGBColor.R << 16 | RGBColor.G << 8 | RGBColor.B;
    }

    //tCPU::byte* spriteRamStream = SPR_RAM;
    tCPU::dword *spriteRamStream32bit = (tCPU::dword *) SPR_RAM;
    tCPU::word *ppuRamPatternBase = (tCPU::word *) PPU_RAM;

    // 64 possible sprites, can only draw 8 of them per scanline
    int numSpritesDrawn = 0;

    for (i = 0; i < 256; i += 4) {
        tCPU::dword spriteEntry = *(spriteRamStream32bit++);

        tCPU::byte SpriteY = (spriteEntry & 0xff) + 1; // *(spriteRamStream++) + 1;

        if (SpriteY == 249) {    // sprite wants to be ignored
            continue;
        }

        if (SpriteY > Y || (SpriteY + 8) <= Y) {    // current scanline does not overlap the sprite
            continue;
        }

        tCPU::byte Tile = (spriteEntry >> 8) & 0xff;
        tCPU::byte Attr = (spriteEntry >> 16) & 0xff;
        tCPU::byte SpriteX = (spriteEntry >> 24) & 0xff;

        if (SpriteY > 238 || SpriteX > 248) {
            // not so properly ignored
            continue;
        }

        numSpritesDrawn++;

        bool VerticalFlip = Bit<7>::IsSet(Attr);
        bool HorizontalFlip = Bit<6>::IsSet(Attr);
        bool SpriteBehindBG = Bit<5>::IsSet(Attr);
        tCPU::byte SpritePalette = Bits<0, 1>::Get(Attr);


        //if( SpriteY <= Y && (SpriteY + 8) > Y && SpriteX <= X && (SpriteX + 8) > X )
        //if( SpriteY <= Y && (SpriteY + 8) > Y )
        {
            j = Y - SpriteY;
            int ProperY = VerticalFlip ? (7 - j) : j;

            tCPU::byte PatternBit0 = PPU_RAM[settings.SpritePatternTableAddress + Tile * 16 + ProperY + 8 * 0];
            tCPU::byte PatternBit1 = PPU_RAM[settings.SpritePatternTableAddress + Tile * 16 + ProperY + 8 * 1];

            for (X = SpriteX; X < SpriteX + 8; X++) {
                int l = X - SpriteX;
                int ProperX = HorizontalFlip ? (l) : (7 - l);

                // the result of these logical operations is not 0/1, so we need to clamp them to a bool
                bool PixelBit0 = PatternBit0 & (1 << ProperX) ? 1 : 0;
                bool PixelBit1 = PatternBit1 & (1 << ProperX) ? 1 : 0;

                // lower two bits
                tCPU::byte Color = (PixelBit0 ? 1 : 0) + (PixelBit1 ? 2 : 0);

                //PrintPpu( "(%d & (1 << %d) = %d", PatternBit0, ProperX, (PatternBit0 & (1 << ProperX)));
                //PrintPpu( "  (%d & (1 << %d) = %d", PatternBit1, ProperX, (PatternBit1 & (1 << ProperX)));

                tCPU::byte PaletteId = GetColorFromPalette(1, SpritePalette, Color);
                tCPU::byte Luminance = (PaletteId & 0xF0) >> 4;
                tCPU::byte Chrominance = (PaletteId & 0x0F);

                //tPaletteEntry& RGBColor = g_ColorPalette[Chrominance + Luminance * 16];

                raster->spriteMask[Y * 256 + X] = Color;

                // FIXME: add support for sprite priorities

                if (!SpriteBehindBG && Color || true) {
                    int *screenBufferPtr32bit = (int *) (raster->screenBuffer + (reverseY * 256) * 4 + X * 4);

                    tPaletteEntry &RGBColor = colorPalette[Chrominance + Luminance * 16];

                    *(screenBufferPtr32bit) = 0xffffffff;// RGBColor.B | RGBColor.G << 8 | RGBColor.R << 16 | 0xff000000;
//                    PrintInfo("Wrote %X into %0X", *(screenBufferPtr32bit), (reverseY * 256) * 4 + X * 4);
                    //*(screenBufferPtr32bit) = g_ColorPalette[Chrominance + Luminance * 16].ColorValue;
                    //screenBufferPtr32bit++;
                }

                if (!sprite0HitInThisFrame && !sprite0HitInThisScanline && i == 0) {
                    // if we got a sprite0 color pixel over a non-transparent background pixel
                    if (raster->backgroundMask[Y * 256 + X] == Color) {
                        sprite0HitInThisScanline = true;
                    }
                }
            }
        }
    }

    if (numSpritesDrawn >= 8) {
        //PrintPpu("sprite overflow!" );
        statusRegister |= Bit<5>::Set(true);
    }

//    for(int x = 0; x < 256; x ++) {
//        raster->finalBuffer[Y * 256 * 4 + x * 4 + 0] = 0x33; // b
//        raster->finalBuffer[Y * 256 * 4 + x * 4 + 1] = 0x66; // g
//        raster->finalBuffer[Y * 256 * 4 + x * 4 + 2] = Y; // r
//        raster->finalBuffer[Y * 256 * 4 + x * 4 + 3] = 0x33; // alpha
//    }

    PrintPpu("Rendered scanline Y=%d", Y);
}

// palettetype: 0 = background, 1 = sprites
tCPU::byte
PPU::GetColorFromPalette(int PaletteType, int NameTableId, int ColorId) {
    tCPU::byte Color = 0;

    if (ColorId == 0) {
        Color = PPU_RAM[0x3F00];
    } else if (ColorId <= 3) {
        // i guess the landscape of mario is sprites and not background
        // which makes sense cause u want collision detection with landscape..
        Color = PPU_RAM[0x3F00 + PaletteType * 0x10 + ColorId + NameTableId * 4];
    } else {
        PrintError("PPU::GetColorFromPalette(%d, %d); Invalid Color Id", NameTableId, ColorId);
        throw std::runtime_error("Unexpected error");
    }

    return Color;
}

tCPU::byte
PPU::getControlRegister1() {
    return controlRegister1;
}

void
PPU::setControlRegister1(tCPU::byte value) {
    std::bitset<8> bits(value);
    controlRegister1 = value;

    tCPU::byte nameTableIdx = bits.test(0) + bits.test(1);
    settings.NameTableAddress = 0x2000 + nameTableIdx * 0x400;

    // increment vram address (on port $2007 activity) by 1 (horizontal) or 32 (vertical) bytes
    settings.DoVerticalWrites = bits.test(2);
    settings.SpritePatternTableAddress = bits.test(3) ? 0x1000 : 0x0000;
    settings.BackgroundPatternTableAddress = bits.test(4) ? 0x1000 : 0x0000;

    settings.SpriteSize = bits.test(5) ? SPRITE_SIZE_8x16 : SPRITE_SIZE_8x8;

    settings.GenerateInterruptOnSprite = bits.test(6);
    settings.GenerateInterruptOnVBlank = bits.test(7);

    tempVRAMAddress &= 0xF3FF;
    tempVRAMAddress |= (value & 0x03) << 10;

//    PrintInfo("Set base nametable address to %X", settings.NameTableAddress);
//    PrintInfo("Set base background pattern table address to %X", settings.BackgroundPatternTableAddress);
//    PrintInfo("Set base sprite pattern table address to %X", settings.SpritePatternTableAddress);
    PrintPpu("Generate an NMI at start of vertical blanking interval: %d", settings.GenerateInterruptOnVBlank);
}

void
PPU::setControlRegister2(tCPU::byte value) {
    std::bitset<8> bits(value);

    settings.DisplayTypeMonochrome = bits.test(0);
    settings.BackgroundClipping = bits.test(1);
    settings.SpriteClipping = bits.test(2);
    settings.BackgroundVisible = bits.test(3);
    settings.SpriteVisible = bits.test(4);
}

/**
 * This function gets called twice to fill out a 16-bit address
 * once for each byte. so we have to track whether we are writing byte 1 or 2.
 */
void
PPU::setVRamAddressRegister2(tCPU::byte value) {
    if (firstWriteToSFF) {
        // first write -- high byte
        latchedVRAMByte = value;
        tempVRAMAddress &= 0x80FF;
        tempVRAMAddress |= (value & 0x3F) << 8; // 6 bits to high byte

        PrintInfo("tempVRAMAddress first part = 0x%X (0x%X)", tempVRAMAddress, value);
    } else {
        // second write -- low byte
//        vramAddress14bit = ((tCPU::word) latchedVRAMByte) << 8;
//        vramAddress14bit |= value;

        tempVRAMAddress &= 0xFF00; // clear low byte
        tempVRAMAddress |= value; // set low byte
        vramAddress14bit = tempVRAMAddress;
//        vramAddress14bit &= 0x7FFF; // clear highest bit

        PrintInfo("vramAddress14bit = 0x%X (0x%X)", vramAddress14bit, value);
    }

    // flip
    firstWriteToSFF = !firstWriteToSFF;
}

tCPU::byte
PPU::readFromVRam() {
    tCPU::byte value;

    if (vramAddress14bit % 0x4000 <= 0x3EFF) {    // latch value, return old
        value = latchedVRAMByte;
        latchedVRAMByte = ReadInternalMemoryByte(vramAddress14bit);
        PrintDbg("New Latch = 0x%02X; Returning Old Latch value 0x%02X", (int) latchedVRAMByte, (int) value);
    } else {
        value = ReadInternalMemoryByte(vramAddress14bit);
        PrintDbg("Returning Direct (Non-Latched) VRAM value: 0x%02X", (int) value);
    }

    AutoIncrementVRAMAddress();

    return value;
}

void
PPU::writeToVRam(tCPU::byte value) {
    WriteInternalMemoryByte(vramAddress14bit, value);
    AutoIncrementVRAMAddress();
}

tCPU::word
PPU::GetEffectiveAddress(tCPU::word address) {
    if (address < 0x2000) {
        // pattern table chr-rom page
        PrintDbg("Address is < 0x2000; Referencing CHR-ROM @ $%04X", (int) (address));
    }

    if (address >= 0x3000 && address <= 0x3EFF) {
        // mirror of 2000h-2EFFh
        PrintDbg("Address in range: (0x3000-0x3EFF); Mirror of $%04X", (int) (address - 0x1000));
        address -= 0x1000;
    }
    if (address >= 0x3F20 && address <= 0x3FFF) {
        // 7 mirrors of 3F00h-3F1Fh :(
        PrintDbg("Hit 7 Mirror Address: 0x%X", (int) address);
        address -= ((address - 0x3F20) % 0x1F) * 0x1F;
        PrintDbg("-> Resolved it to: 0x%X", (int) address);
    }

    // mirrors 3F10h,3F14h,3F18h,3F1Ch -> 3F00h,3F04h,3F08h,3F0Ch
    // FIXME: im not sure if these mirrors are ranges or just single byte entries
    if (address == 0x3F10 || address == 0x3F14 || address == 0x3F18 || address == 0x3F1C)
        address -= 0x10;


    if (address >= 0x3F00 && address <= 0x3F1F) {
        PrintDbg("address $%04X is a BG/Sprite Palette!", address);
    }

    if (address >= 0x4000) {
        PrintDbg("Address in range: (0x4000-0x10000); Mirror of $%04X", address % 0x4000);
        address = address % 0x4000;
    }

    return address;
}

tCPU::byte
PPU::ReadInternalMemoryByte(tCPU::word Address) {
    tCPU::word EffectiveAddress = GetEffectiveAddress(Address);
    tCPU::byte Value = PPU_RAM[EffectiveAddress];
    PrintPpu("Read 0x%02X from PPU RAM @ 0x%04X", (int) Value, (int) EffectiveAddress);
    return Value;
}

bool
PPU::WriteInternalMemoryByte(tCPU::word Address, tCPU::byte Value) {
    tCPU::word EffectiveAddress = GetEffectiveAddress(Address);
    PPU_RAM[EffectiveAddress] = Value;
    if (EffectiveAddress >= 0x3f00 && EffectiveAddress < 0x4000) {
        PrintDbg("Wrote 0x%02X to PPU RAM @ 0x%04X (0x%04X)", (int) Value, (int) EffectiveAddress, Address);
    }
    return true;
}

void
PPU::AutoIncrementVRAMAddress() {
    tCPU::byte incAmount = settings.DoVerticalWrites ? 32 : 1;
    vramAddress14bit += incAmount;
    PrintPpu("Incremented by %d bytes", (int) incAmount);
}

// https://wiki.nesdev.com/w/index.php/PPU_scrolling
void
PPU::setVRamAddressRegister1(tCPU::byte value) {
//    PrintInfo("value = %d and scanline = %d hblank = %d", value, currentScanline, inHBlank);
//    if (currentScanline < 240) {
//        if (!inHBlank) {
//            return;
//        }
//    }

    if (firstWriteToSFF) {
        // first write
        horizontalScrollOrigin = value;

        tempVRAMAddress &= 0xFFE0;
        tempVRAMAddress |= value >> 3;

        tileXOffset = value & 0x07;

        if (horizontalScrollOrigin) {
            auto x = vramAddress14bit & 0x001F;
            PrintInfo("course x = %d / fine x = %d / tileXOffset = %d", x, horizontalScrollOrigin, tileXOffset);
        }
//        PrintInfo("tempVRAMAddress = 0x%04X first write", (int) tempVRAMAddress);
    } else {
        // second write
        verticalScrollOrigin = value;

        tempVRAMAddress &= 0x8FFF;
        tempVRAMAddress |= (value & 0x07) << 12;
        tempVRAMAddress &= 0xFC1F;
        tempVRAMAddress |= (value & 0xF8) << 2;


//        PrintInfo("tempVRAMAddress = 0x%04X second", (int) tempVRAMAddress);
        PrintPpu("verticalScrollOrigin = %d", verticalScrollOrigin);
    }


//    PrintDbg("-> Background/Sprite Visibility: %d/%d", (int) settings.BackgroundVisible, (int) settings.SpriteVisible);

    // flip
    firstWriteToSFF = !firstWriteToSFF;
}

void
PPU::setSprRamAddress(tCPU::byte address) {
    spriteRamAddress = address;
}

void
PPU::writeSpriteMemory(tCPU::byte value) {
    if (spriteRamAddress >= 256) {
        PrintError("spriteRamAddress is out of range; Expected < 256, Actual = %d", (int) spriteRamAddress);
    } else {
        PrintDbg("Write byte $%02X to Sprite RAM @ $%02X", (int) value, (int) spriteRamAddress);
        SPR_RAM[spriteRamAddress] = value;
    }

    // address incremented after every write
    spriteRamAddress++;
}

tCPU::byte PPU::readSpriteMemory() {
    tCPU::byte value = SPR_RAM[spriteRamAddress];
    PrintDbg("Read byte $%02X from SPR-RAM @ $%02X", (int) value, (int) spriteRamAddress);
    return value;
}

void
PPU::StartSpriteXferDMA(Memory *memory, tCPU::byte address) {
    tCPU::word startAddress = address * 0x100;
    PrintDbg("DMA Transfer from RAM @ $%04X to SPR-RAM", (int) startAddress);

    for (tCPU::word i = 0; i < 256; i++) {
        SPR_RAM[i] = memory->readByte(startAddress + i);
    }

//    vramAddress14bit = 0;
}

void PPU::clear() {
    // clear final output (256x256@32bit)
    for (int x = 0; x < 256; x++) {
        for (int y = 0; y < 256; y++) {
            raster->screenBuffer[y * 256 * 4 + x * 4 + 0] = 0x33; // b
            raster->screenBuffer[y * 256 * 4 + x * 4 + 1] = 0x33; // g
            raster->screenBuffer[y * 256 * 4 + x * 4 + 2] = 0x33; // r
            raster->screenBuffer[y * 256 * 4 + x * 4 + 3] = 0xff; // alpha
        }
    }

    // clear pattern table debug view (128x256@32bit)
    for (int x = 0; x < 128; x++) {
        for (int y = 0; y < 256; y++) {
            raster->patternTable[y * 128 * 4 + x * 4 + 0] = 0x33; // b
            raster->patternTable[y * 128 * 4 + x * 4 + 1] = 0x33; // g
            raster->patternTable[y * 128 * 4 + x * 4 + 2] = 0x33; // r
            raster->patternTable[y * 128 * 4 + x * 4 + 3] = 0xff; // alpha
        }
    }

    // clear palette table debug view (256x32@32bit)
    for (int x = 0; x < 256; x++) {
        for (int y = 0; y < 32; y++) {
            raster->palette[y * 256 * 4 + x * 4 + 0] = 0x33; // b
            raster->palette[y * 256 * 4 + x * 4 + 1] = 0x33; // g
            raster->palette[y * 256 * 4 + x * 4 + 2] = 0x33; // r
            raster->palette[y * 256 * 4 + x * 4 + 3] = 0xff; // alpha
        }
    }

    // clear sprite mask debug view (256x256@32bit)
    for (int x = 0; x < 256; x++) {
        for (int y = 0; y < 256; y++) {
            raster->spriteMask[y * 256 + x + 0] = 0; // single byte packed format
        }
    }
}

void PPU::renderDebug() {
//    RenderBackgroundTiles();
//    RenderSpriteTiles();

    RenderDebugNametables();
//    RenderDebugAttributes(NametableAddress);
//    RenderDebugPatternTables();
//    RenderDebugColorPalette();
}

/**
 * Render sprites onto final output
 *
 * 64 sprites (4 bytes each)
 */
void PPU::RenderSpriteTiles() {
    for (auto i = 0; i < 64; i++) {
        auto addr = i * 4;
        auto y = SPR_RAM[addr] - 1;
        auto tileNumber = SPR_RAM[addr + 1];
        auto attributes = SPR_RAM[addr + 2];
        auto x = SPR_RAM[addr + 3];

        auto upperBits = Bits<1,2>::Get(attributes);

        // render 8x8 block

        int offsetY = y * 256 * 4;
        int offsetX = x * 4;

        if (y >= 239 || y < 0) {
            continue;
        }

//        PrintInfo("%d: x/y = %d,%d: flip bits horizontal=%x vertical=%x", i, x, y, Bit<6>::QuickIsSet(attributes), Bit<7>::QuickIsSet(attributes));

        auto flipH = Bit<6>::QuickIsSet(attributes);

        // row
        for (auto k = 0; k < 8; k++) {
            tCPU::byte PatternByte0 = PPU_RAM[settings.SpritePatternTableAddress + tileNumber * 16 + k];
            tCPU::byte PatternByte1 = PPU_RAM[settings.SpritePatternTableAddress + tileNumber * 16 + k + 8];

            // column
            for (auto l = 0; l < 8; l++) {
                int pixelBit0 = PatternByte0 & (1 << l) ? 255 : 0;
                int pixelBit1 = PatternByte1 & (1 << l) ? 255 : 0;
                tCPU::byte lowerBits = (pixelBit0 ? 1 : 0) + (pixelBit1 ? 2 : 0);
                tCPU::byte paletteId = GetColorFromPalette(1, upperBits, lowerBits);

                if (lowerBits == 0) {
                    continue;
                }

                tPaletteEntry &color = colorPalette[paletteId];

                int offsetBlockY = offsetY + offsetX + k * 256 * 4;
                int offsetBytes = offsetBlockY + (flipH ? l : (7 - l)) * 4;

                // write 4 bytes BGRA
                raster->screenBuffer[offsetBytes + 0] = color.B; // b
                raster->screenBuffer[offsetBytes + 1] = color.G; // g
                raster->screenBuffer[offsetBytes + 2] = color.R; // r
                raster->screenBuffer[offsetBytes + 3] = color.A; // alpha

//                if (!sprite0HitInThisFrame && !sprite0HitInThisScanline) {
//                    if (raster->backgroundMask[offsetBytes / 4] > 0 && lowerBits > 0) {
//                        // same color?
//                        statusRegister |= Bit<6>::Set(true);
//                        sprite0HitInThisFrame = true;
//                        sprite0HitInThisScanline = true;
//                    }
//                }
            }
        }
    }
}

void PPU::RenderBackgroundTiles() {
    tCPU::word NametableAddress = settings.NameTableAddress;
//    int NametableId = (NametableAddress - 0x2000) / 0x400;
//
//    if (horizontalScrollOrigin >= 255) {
//        if (NametableId == 0)
//            NametableAddress += 0x400;
//        else
//            NametableAddress -= 0x400;
//
//        NametableId = (NametableAddress - 0x2000) / 0x400;
//    }
//

//    for (unsigned int i = 0; i < 32; i++) {
//        PrintInfo("%d = %X", i, PPU_RAM[0x3F00 + i]);
//    }

    /**
     * Render background tiles onto final output
     */

    // rows (240 pixels vertically)
    for (unsigned int i = 0; i < 30; i++) {
        // columns (256 pixels horizontally)
        for (unsigned int j = 0; j < 32; j++) {
            // 8x8 tile number
            // first 960 bytes contain tiles
            tCPU::byte tileNumber = PPU_RAM[NametableAddress + i * 32 + j];
            int offsetY = i * 8 * 256 * 4;
            int offsetX = j * 8 * 4;

            auto AttrI = (int) floor(j / 4.0);
            auto AttrJ = (int) floor(i / 4.0);

            // after tiles, we have two scanlines for attributes
            tCPU::byte attribute = PPU_RAM[NametableAddress + 0x3C0 + AttrJ * 8 + AttrI];

            int offsetI = (j / 2) % 2;
            int offsetJ = (i / 2) % 2;

            tCPU::byte upperLeft = Bits<0, 1>::Get(attribute);
            tCPU::byte upperRight = Bits<2, 3>::Get(attribute);
            tCPU::byte lowerLeft = Bits<4, 5>::Get(attribute);
            tCPU::byte lowerRight = Bits<6, 7>::Get(attribute);

            // grab attributes for the tile
            tCPU::byte upperBits;

            if (!offsetI && !offsetJ)
                upperBits = upperLeft;
            else if (offsetI && !offsetJ)
                upperBits = upperRight;
            else if (!offsetI && offsetJ)
                upperBits = lowerLeft;
            else
                upperBits = lowerRight;

            // render 8x8 block

            // row
            for (unsigned short k = 0; k < 8; k++) {
                tCPU::byte PatternByte0 = PPU_RAM[settings.BackgroundPatternTableAddress + tileNumber * 16 + k];
                tCPU::byte PatternByte1 = PPU_RAM[settings.BackgroundPatternTableAddress + tileNumber * 16 + k + 8];

                // column
                for (short l = 0; l < 8; l++) {
                    int pixelBit0 = PatternByte0 & (1 << l) ? 255 : 0;
                    int pixelBit1 = PatternByte1 & (1 << l) ? 255 : 0;
                    tCPU::byte lowerBits = (pixelBit0 ? 1 : 0) + (pixelBit1 ? 2 : 0);
                    tCPU::byte paletteId = GetColorFromPalette(0, upperBits, lowerBits);

                    tPaletteEntry &color = colorPalette[paletteId];

                    int offsetBlockY = offsetY + offsetX + k * 256 * 4;
                    int offsetBytes = offsetBlockY + (7 - l) * 4;

//                    raster->backgroundMask[offsetBytes / 4] = lowerBits ? 0x90 : 0x00;

                    // write 4 bytes BGRA
                    raster->screenBuffer[offsetBytes + 0] = color.B; // b
                    raster->screenBuffer[offsetBytes + 1] = color.G; // g
                    raster->screenBuffer[offsetBytes + 2] = color.R; // r
                    raster->screenBuffer[offsetBytes + 3] = 0xff; // alpha
                }
            }
        }
    }
}

void PPU::RenderDebugPatternTables() {// render two pattern tables
    // pattern table 0 = 0x0000 - 0x0FFF
    // pattern table 1 = 0x1000 - 0x1FFF

    // pattern table pitch
    unsigned int srcPitch = 16;

    // render target pitch
    unsigned int dstPitch = 128;

    // each pattern table:
    // has 256 tiles, 16x16
    // each tile is 8x8 pixels
    // each pixel has two bits for four colors
    // renders into 128x128 target

    // rows
    for (auto i = 0; i < 32; i++) {
        // columns
        for (auto j = 0; j < 16; j++) {
            // draw as 8x8 blocks
            auto src = (i * srcPitch + j) * 16;
            auto dst = (i * dstPitch + j) * 8;

            // rows
            for (auto k = 0; k < 8; k++) {
                // 16 bytes per 8x8 tile
                // 8 rows, each row takes two bytes
                // each pixel in the byte is a color bit for a column
                auto rowSrc = src + k;
                auto pattern1 = PPU_RAM[rowSrc + 0];
                auto pattern2 = PPU_RAM[rowSrc + 8];

                // columns
                for (auto l = 0; l < 8; l++) {
                    auto pixel1 = (pattern1 & (1 << (7 - l))) ? 1 : 0;
                    auto pixel2 = (pattern2 & (1 << (7 - l))) ? 1 : 0;

                    auto color = (pixel1 ? 1 : 0) + (pixel2 ? 2 : 0);
                    tCPU::byte paletteId = GetColorFromPalette(1, 0, color);
                    tPaletteEntry &rgbColor = colorPalette[paletteId];

                    auto pixelAddr = dst + k * dstPitch + l;
                    raster->patternTable[pixelAddr * 4 + 0] = rgbColor.B; // b
                    raster->patternTable[pixelAddr * 4 + 1] = rgbColor.G; // g
                    raster->patternTable[pixelAddr * 4 + 2] = rgbColor.R; // r
                    raster->patternTable[pixelAddr * 4 + 3] = 0xff; // alpha
                }
            }
        }
    }
}

/*
 * Render attributes
 *
 * 64 bytes for attributes
 * 8x8 attributes table, 1 byte per attribute
 * each attribute covers 32x32 pixels
 * each attribute covers 2x2 super-tiles (16x16 pixels) with 2 color bits
 * each super-tile covers 2x2 set of tiles (8x8 pixels each)
 * thus each attribute tile is made up of 4x4 pattern tiles
 */
void PPU::RenderDebugAttributes(tCPU::word NametableAddress) const {
    // clear attribute table debug view (128x128@32bit)
    for (int x = 0; x < 128; x++) {
        for (int y = 0; y < 128; y++) {
            raster->attributeTable[y * 128 * 4 + x * 4 + 0] = 0x33; // b
            raster->attributeTable[y * 128 * 4 + x * 4 + 1] = 0x33; // g
            raster->attributeTable[y * 128 * 4 + x * 4 + 2] = 0x33; // r
            raster->attributeTable[y * 128 * 4 + x * 4 + 3] = 0xff; // alpha
        }
    }

    // rows
    for (int i = 0; i < 8; i++) {
        // columns
        for (int j = 0; j < 8; j++) {
            // 32x32 tile attributes, 2 bits for each 16x16 sub-tile (which in effect is 4 pattern tiles)
            tCPU::byte attribute = PPU_RAM[NametableAddress + 0x3C0 + i * 8 + j];

            tCPU::byte upperLeft = Bits<0, 1>::Get(attribute);
            tCPU::byte upperRight = Bits<2, 3>::Get(attribute);
            tCPU::byte lowerLeft = Bits<4, 5>::Get(attribute);
            tCPU::byte lowerRight = Bits<6, 7>::Get(attribute);

            // destination is a 256x256 grid
            unsigned int dstAddr = (i * 256 + j) * 32;
            tCPU::byte color = 0;

            // render four 16x16 blocks
            for (unsigned int b = 0; b < 2; b++) {
                for (unsigned int c = 0; c < 2; c++) {
                    unsigned int blockAddr = dstAddr + (b * 256 + c) * 16;

                    if (!b && !c)
                        color = upperLeft;
                    else if (b && !c)
                        color = upperRight;
                    else if (!b && c)
                        color = lowerLeft;
                    else
                        color = lowerRight;

                    // row
                    for (unsigned int k = 0; k < 16; k++) {
                        // column
                        for (unsigned int l = 0; l < 16; l++) {
                            unsigned int pixelAddr = (blockAddr + k * 256 + l) * 4;

                            raster->attributeTable[pixelAddr + 0] = color * 64; // b
                            raster->attributeTable[pixelAddr + 1] = color * 64; // g
                            raster->attributeTable[pixelAddr + 2] = color * 64; // r
                            raster->attributeTable[pixelAddr + 3] = 0xff; // alpha
                        }
                    }

                }
            }
        }
    }
}

void PPU::RenderDebugColorPalette() const {// render color palette
    // colors
    for (auto nametableId = 0; nametableId < 4; nametableId++) {
        for (auto p = 0; p < 32; p++) {

            auto size = 8;
            auto paletteType = 0; // 0 = background, 1 = sprite
            auto paletteId = PPU_RAM[0x3F00 + paletteType * 16 + p + nametableId * 4];
            auto pitch = 256;
            auto dst = nametableId * pitch * size + p * size;

            tPaletteEntry &color = colorPalette[paletteId];

            // draw as 8x8 blocks
            // rows
            for (auto y = 0; y < size; y++) {
                // columns
                for (auto x = 0; x < size; x++) {
                    auto pixelAddr = dst + y * pitch + x;

                    raster->palette[pixelAddr * 4 + 0] = color.B; // b
                    raster->palette[pixelAddr * 4 + 1] = color.G; // g
                    raster->palette[pixelAddr * 4 + 2] = color.R; // r
                    raster->palette[pixelAddr * 4 + 3] = 0xff; // alpha
                }
            }
        }
    }
}

/**
 * Render all nametables for whole world view
 *
 * Four nametables (2x2) each one 1024 bytes:
 * 960 bytes for tile indices + 64 bytes for attributes
 */
void PPU::RenderDebugNametables() {
    // clear nametable debug view (1024x1024@32bit)
    for (int x = 0; x < 512; x++) {
        for (int y = 0; y < 512; y++) {
            raster->nametables[y * 512 * 4 + x * 4 + 0] = 0x33; // b
            raster->nametables[y * 512 * 4 + x * 4 + 1] = 0x33; // g
            raster->nametables[y * 512 * 4 + x * 4 + 2] = 0x33; // r
            raster->nametables[y * 512 * 4 + x * 4 + 3] = 0xff; // alpha
        }
    }

    // two rows of nametables
    for (auto a = 0; a < 2; a++) {
        // two columns of nametables
        for (auto b = 0; b < 2; b++) {
            // each nametable:

            // rows (240 pixels vertically)
            for (unsigned int i = 0; i < 30; i++) {
                // columns (256 pixels horizontally)
                for (unsigned int j = 0; j < 32; j++) {
                    // 8x8 tile number
                    // first 960 bytes contain tiles

//                    int ScrolledX = i * 8 + horizontalScrollOrigin;
//
//                    // horizontal scrolled tile position
//                    int ScrolledI = ScrolledX / 8;
//
//                    // only need to wrap around once per scanline (i hope..)
//                    if (ScrolledX >= 256) {
//                        NametableAddress = settings.NameTableAddress;
//                        NametableId = (NametableAddress - 0x2000) / 0x400;
//
//                        if (NametableId == 0)
//                            NametableAddress += 0x400;
//                        else
//                            NametableAddress -= 0x400;
//
//                        ScrolledX %= 256;
//                        ScrolledI = ScrolledX / 8;
//                    }

                    auto nametableAddress = settings.NameTableAddress + a * 2048 + b * 1024;

                    tCPU::byte tileNumber = PPU_RAM[nametableAddress + i * 32 + j];

                    auto AttrI = (int) floor(j / 4.0);
                    auto AttrJ = (int) floor(i / 4.0);

                    // after tiles, we have two scanlines for attributes
                    tCPU::byte attribute = PPU_RAM[nametableAddress + 0x3C0 + AttrJ * 8 + AttrI];

                    int offsetI = (j / 2) % 2;
                    int offsetJ = (i / 2) % 2;

                    tCPU::byte upperLeft = Bits<0, 1>::Get(attribute);
                    tCPU::byte upperRight = Bits<2, 3>::Get(attribute);
                    tCPU::byte lowerLeft = Bits<4, 5>::Get(attribute);
                    tCPU::byte lowerRight = Bits<6, 7>::Get(attribute);

                    // grab attributes for the tile
                    tCPU::byte upperBits;

                    if (!offsetI && !offsetJ)
                        upperBits = upperLeft;
                    else if (offsetI && !offsetJ)
                        upperBits = upperRight;
                    else if (!offsetI && offsetJ)
                        upperBits = lowerLeft;
                    else
                        upperBits = lowerRight;

                    // render 8x8 block
                    int offsetY = (i * 8 * 512 * 4) + a * 8 * 512 * 4 * 32;
                    int offsetX = (j * 8 * 4) + b * 8 * 4 * 32;

                    // row
                    for (unsigned short k = 0; k < 8; k++) {
                        // lookup row byte pattern
                        tCPU::byte PatternByte0 = PPU_RAM[settings.BackgroundPatternTableAddress + tileNumber * 16 + k];
                        tCPU::byte PatternByte1 = PPU_RAM[settings.BackgroundPatternTableAddress + tileNumber * 16 + k +
                                                          8];

                        // column
                        for (short l = 0; l < 8; l++) {
                            int pixelBit0 = PatternByte0 & (1 << l) ? 255 : 0;
                            int pixelBit1 = PatternByte1 & (1 << l) ? 255 : 0;
                            tCPU::byte lowerBits = (pixelBit0 ? 1 : 0) + (pixelBit1 ? 2 : 0);
                            tCPU::byte paletteId = GetColorFromPalette(0, upperBits, lowerBits);

                            tPaletteEntry &color = colorPalette[paletteId];

                            auto offsetBlockY = offsetY + offsetX + k * 512 * 4;
                            auto offsetBytes = offsetBlockY + (7 - l) * 4;

                            // write 4 bytes BGRA
                            // offset into correct quadrant
                            raster->nametables[offsetBytes + 0] = color.B; // b
                            raster->nametables[offsetBytes + 1] = color.G; // g
                            raster->nametables[offsetBytes + 2] = color.R; // r
                            raster->nametables[offsetBytes + 3] = 0xff; // alpha
                        }
                    }
                }
            }

            // render viewport scrolling

            // rows
            for (auto i = 0; i < 256; i++) {
                auto offsetBytes = i * 512 * 4 + (horizontalScrollOrigin) * 4;
                raster->nametables[offsetBytes + 0] = 0x33; // b
                raster->nametables[offsetBytes + 1] = 0x66; // g
                raster->nametables[offsetBytes + 2] = 0x99; // r
                raster->nametables[offsetBytes + 3] = 0xff; // alpha

                offsetBytes = i * 512 * 4 + (horizontalScrollOrigin) * 4 + 256 * 4;
                raster->nametables[offsetBytes + 0] = 0x33; // b
                raster->nametables[offsetBytes + 1] = 0x66; // g
                raster->nametables[offsetBytes + 2] = 0x99; // r
                raster->nametables[offsetBytes + 3] = 0xff; // alpha

                // top line
                offsetBytes = (horizontalScrollOrigin) * 4 + i * 4;
                raster->nametables[offsetBytes + 0] = 0x33; // b
                raster->nametables[offsetBytes + 1] = 0x66; // g
                raster->nametables[offsetBytes + 2] = 0x99; // r
                raster->nametables[offsetBytes + 3] = 0xff; // alpha

                // bottom line
                offsetBytes = 256 * 512 * 4 + (horizontalScrollOrigin) * 4 + i * 4;
                raster->nametables[offsetBytes + 0] = 0x33; // b
                raster->nametables[offsetBytes + 1] = 0x66; // g
                raster->nametables[offsetBytes + 2] = 0x99; // r
                raster->nametables[offsetBytes + 3] = 0xff; // alpha
            }
        }
    }
}

void
PPU::loadRom(Cartridge &rom) {
    for (uint8_t i = 0; i < rom.header.numChrPages; i++) {
        writeChrPage(rom.characterDataPages[i].buffer);
    }
}

/**
 * Write 8kB chr-rom page to PPU memory
 * Populates the pattern table
 */
void
PPU::writeChrPage(uint8_t buffer[]) {

//    for (int j = 0; j < CHR_ROM_PAGE_SIZE; j += 8) {
//        PrintInfo("%05X %02X %02X %02X %02X %02X %02X %02X %02X", (j), (int) buffer[j + 0], (int) buffer[j + 1],
//                  (int) buffer[j + 2], (int) buffer[j + 3], (int) buffer[j + 4], (int) buffer[j + 5],
//                  (int) buffer[j + 6], (int) buffer[j + 7]);
//    }

    memcpy(PPU_RAM, buffer, 0x2000);
}