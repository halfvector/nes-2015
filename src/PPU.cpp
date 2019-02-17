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
            uint8_t B, G, R, A;
        };

        uint32_t ColorValue;
    };


};

tPaletteEntry colorPalette[64] = {
        {0x80, 0x80, 0x80, 0xFF},
        {0xBB, 0x00, 0x00, 0xFF},
        {0xBF, 0x00, 0x37, 0xFF},
        {0xA6, 0x00, 0x84, 0xFF},
        {0x6A, 0x00, 0xBB, 0xFF},
        {0x1E, 0x00, 0xB7, 0xFF},
        {0x00, 0x00, 0xB3, 0xFF},
        {0x00, 0x26, 0x91, 0xFF},
        {0x00, 0x2B, 0x7B, 0xFF},
        {0x00, 0x3E, 0x00, 0xFF},
        {0x0D, 0x48, 0x00, 0xFF},
        {0x22, 0x3C, 0x00, 0xFF},
        {0x66, 0x2F, 0x00, 0xFF},
        {0x00, 0x00, 0x00, 0xFF},
        {0x05, 0x05, 0x05, 0xFF},
        {0x05, 0x05, 0x05, 0xFF},

        {0xC8, 0xC8, 0xC8, 0xFF},
        {0xFF, 0x59, 0x00, 0xFF},
        {0xFF, 0x3C, 0x44, 0xFF},
        {0xCC, 0x33, 0xB7, 0xFF},
        {0xAA, 0x33, 0xFF, 0xFF},
        {0x5E, 0x37, 0xFF, 0xFF},
        {0x1A, 0x37, 0xFF, 0xFF},
        {0x00, 0x4B, 0xD5, 0xFF},
        {0x00, 0x62, 0xC4, 0xFF},
        {0x00, 0x7B, 0x3C, 0xFF},
        {0x15, 0x84, 0x1E, 0xFF},
        {0x66, 0x95, 0x00, 0xFF},
        {0xC4, 0x84, 0x00, 0xFF},
        {0x11, 0x11, 0x11, 0xFF},
        {0x09, 0x09, 0x09, 0xFF},
        {0x09, 0x09, 0x09, 0xFF},

        {0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0x95, 0x00, 0xFF},
        {0xFF, 0x84, 0x6F, 0xFF},
        {0xFF, 0x6F, 0xD5, 0xFF},
        {0xCC, 0x77, 0xFF, 0xFF},
        {0x99, 0x6F, 0xFF, 0xFF},
        {0x59, 0x7B, 0xFF, 0xFF},
        {0x5F, 0x91, 0xFF, 0xFF},
        {0x33, 0xA2, 0xFF, 0xFF},
        {0x00, 0xBF, 0xA6, 0xFF},
        {0x6A, 0xD9, 0x51, 0xFF},
        {0xAE, 0xD5, 0x4D, 0xFF},
        {0xFF, 0xD9, 0x00, 0xFF},
        {0x66, 0x66, 0x66, 0xFF},
        {0x0D, 0x0D, 0x0D, 0xFF},
        {0x0D, 0x0D, 0x0D, 0xFF},

        {0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xBF, 0x84, 0xFF},
        {0xFF, 0xBB, 0xBB, 0xFF},
        {0xFF, 0xBB, 0xD0, 0xFF},
        {0xEA, 0xBF, 0xFF, 0xFF},
        {0xCC, 0xBF, 0xFF, 0xFF},
        {0xB7, 0xC4, 0xFF, 0xFF},
        {0xAE, 0xCC, 0xFF, 0xFF},
        {0xA2, 0xD9, 0xFF, 0xFF},
        {0x99, 0xE1, 0xCC, 0xFF},
        {0xB7, 0xEE, 0xAE, 0xFF},
        {0xEE, 0xF7, 0xAA, 0xFF},
        {0xFF, 0xEE, 0xB3, 0xFF},
        {0xDD, 0xDD, 0xDD, 0xFF},
        {0x11, 0x11, 0x11, 0xFF},
        {0x11, 0x11, 0x11, 0xFF}
};

tCPU::byte screenAttributes[16][16];

PPU::PPU(Raster *raster) : raster(raster) {
    firstWriteToSFF = true, sprite0HitInThisFrame = false, sprite0HitInThisScanline = false,
    inHBlank = false, inVBlank = false;

    currentScanline = 0, scanlinePixel = 0, cycles = 0, vramAddress14bit = 0,
    tempVRAMAddress = 0, latchedVRAMByte = 0, statusRegister = 0,
    horizontalScrollOrigin = 0, verticalScrollOrigin = 0, reloadBits = 0;

    // clear memory
    memset(SPR_RAM, 248, 256);
    memset(PPU_RAM, 0, 0x10000);
}

/**
 * Read Status register ($2002)
 */
tCPU::byte
PPU::getStatusRegister() {
    tCPU::byte returnValue = statusRegister;

    if(Loggy::Enabled == Loggy::DEBUG) {
        PrintInfo("PPU; Status register: %s", std::bitset<8>(statusRegister).to_string().c_str());
        PrintInfo("PPU; -> Scanline: %d, HBlank: %d, Pixel: %d", currentScanline, inHBlank, scanlinePixel);
    }

    // reset vblank on register read
    statusRegister &= ~(1 << 7);
    inVBlank = false;

    // reset $2005 and $2006 write modes
    firstWriteToSFF = true;
    firstWriteToSFF2 = true;

//    // detect sprite 0 collision with current scanline
//    for(int i = 0; i < 64; i++ ){
//        SPR_RAM[i * 4];
//    }

    return returnValue;
}

void
PPU::execute(int numCycles) {
    // each cycle is 1 pixel

    /**
     *   0-239 = rendering scanlines
     *     240 = idle scanline / post-rendering
     * 241-260 = vertical blanking
     *     261 = pre-rendering
     */

    for (int i = 0; i < numCycles; i++) {
        if (currentScanline < 240) {
            // [0,239]
            // process scanline
            advanceRenderableScanline();
        } else if (currentScanline < 241) {
            // [240]
            advanceBlankScanline();
        } else if (currentScanline < 261) {
            // [241,260]
            if (currentScanline == 241 && scanlinePixel == 1) {
                setVerticalBlank();
            }
//            if (currentScanline == 261 && scanlinePixel == 280) {
//                // copy all horizontal scrolling information from temp to vram addy
//                if (settings.BackgroundVisible && settings.SpriteVisible) {
//                    vramAddress14bit &= ~0x041F; // zero out position bits
//                    vramAddress14bit |= (tempVRAMAddress & 0x041F); // copy over just the position bits
//                    vramAddress14bit &= 0x7FFF; // ensure 15th bit is zero
//
//                    PrintInfo("Scanline %d Pixel %d; vramAddress14bit = 0x%X; tempVRAMAddress = %s",
//                              currentScanline, scanlinePixel,
//                              vramAddress14bit, std::bitset<16>(vramAddress14bit).to_string().c_str());
//                }
//            }

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

            // reset nametable
            settings.NameTableAddress = 0x2000;
        }
    }
}

/**
 * Vertical Blanking Interval started
 */
void
PPU::setVerticalBlank() {
    statusRegister |= 1 << 7; // set vblank bit
    inVBlank = true;

//    PrintInfo("GenerateInterruptOnVBlank = %d / vblankNmiAwaiting = %d",
//              settings.GenerateInterruptOnVBlank, vblankNmiAwaiting);

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
    if (scanlinePixel < 257) {
        // [0,256]
        // drawing pixels
        inHBlank = false;
    } else if (scanlinePixel == 257) {
        // [257]
        // last pixel of the scanline. enter hblank.
        inHBlank = true;
        onEnterHBlank();
    } else if (scanlinePixel < 340) {
        // [258, 340]
        // in hblank
        inHBlank = true;
    } else {
        // [340]
        // last pixel of
        // leave hblank and increment scanline
        inHBlank = false;
        currentScanline++;
        scanlinePixel = -1;
    }

    scanlinePixel++;
}

void PPU::onEnterHBlank() {
    // copy all horizontal scrolling information from temp to vram addy
    if (settings.BackgroundVisible && settings.SpriteVisible) {
        vramAddress14bit &= ~0x041F; // zero out position bits
        vramAddress14bit |= (tempVRAMAddress & 0x041F); // copy over just the position bits
        vramAddress14bit &= 0x7FFF; // ensure 15th bit is zero

        auto tileScrollOffset = vramAddress14bit & 0x0FFF;
//        PrintInfo("on hblank; tile scroll x = %d", tileScrollOffset);

//        PrintInfo("Scanline %d Pixel %d; vramAddress14bit = 0x%X; tempVRAMAddress = %s",
//                  currentScanline, scanlinePixel,
//                  vramAddress14bit, std::bitset<16>(vramAddress14bit).to_string().c_str());
    }

    renderScanline(currentScanline);

    // scanline somewhere within sprite 0
    if (sprite0HitInThisFrame) {
        // if we already matched a sprite0 hit this frame
        // dont bother with any hit-detection this frame
        return;
    }

//    // check if this scanline hit sprite0
//    tCPU::byte Y = SPR_RAM[0] + 1;
//    tCPU::byte Tile = SPR_RAM[1];
//    tCPU::byte attributes = SPR_RAM[2];
//    tCPU::byte X = SPR_RAM[3];
//
//    // ignore sprite if Y is set to 249
//    if (Y == 249) {
//        return;
//    }
//
//    if (Y >= 247) {
//        return;
//    }

//    if (currentScanline >= Y && currentScanline <= (Y + 8)) {
    if (!sprite0HitInThisFrame && sprite0HitInThisScanline) {
        statusRegister |= Bit<6>::Set(true);
        sprite0HitInThisFrame = true;
        sprite0HitInThisScanline = false;
    }
//    }
}

void PPU::renderScanline(const tCPU::word Y) {
    // tiles are 8x8 pixel in size.
    ushort tileSizePixels = 8;
    // row number within tile
    ushort tileRow = ushort(Y % tileSizePixels);
    // tile horizontal position within frame
    ushort tileY = ushort(floor(Y / tileSizePixels));

    // attributes are 32x32 pixels in size
    ushort attributeY = ushort(tileY / 4);

    // decode scanline
    // 256 pixels in 32 bytes, each byte a tile consisting of 8 pixels

    tCPU::word nametableAddy = settings.NameTableAddress;

    ushort tileScroll = ushort(vramAddress14bit & 0xFF);
    ushort attributeScroll = ushort(vramAddress14bit & 0x0C00)
                             | ushort((vramAddress14bit >> 4) & 0x38)
                             | ushort((vramAddress14bit >> 2) & 0x07);

//    tileScroll = 0;
    attributeScroll = 0; // ignoring attribute scroll from vram, using tile instead


    int *backgroundRender = (int *) raster->screenBuffer + Y * 256;
    tCPU::byte *backgroundMask = raster->backgroundMask + Y * 256;

    /**
     * Render background tiles.
     * 32 tiles per scanline
     */

    ushort numTiles = 32; // 32 tiles per scanline
    ushort numAttributes = 8; // 8 attributes per scanline (4 per tile)

    if(settings.BackgroundVisible)
    for (ushort i = 0; i <= numTiles; i++) {
        auto nametable = nametableAddy;
        auto nametableAttributeOffset = nametableAddy + 0x3C0;

        if(i + tileScroll >= 32) {
            // read from nametable on the right when doing horizontal scrolling
            nametable += 0x400;
            nametableAttributeOffset += 0x400;
        }

        auto tileIdx = ReadByteFromPPU(nametable + tileY * numTiles + (i + tileScroll) % 32);

        // each attribute block is 32x32 pixels
        // there are 8 attribute blocks per scanline
        // each attribute block covers 4 tiles
        ushort attributeBase = nametableAttributeOffset + attributeScroll;
        ushort attributeX = (((i + tileScroll) % 32) >> 2); // new attribute block every 4 tiles
        auto attribute = ReadByteFromPPU(attributeBase + attributeY * numAttributes + attributeX);
//        auto preScaledUpperBits = (attribute >> ((i & 2) | ((tileY & 2) << 1))) & 3;
//        preScaledUpperBits *= 4;

        int offsetX = (tileY / 2) % 2;
        int offsetY = ((i+tileScroll) / 2) % 2;

        tCPU::byte upperLeft = Bits<0, 1>::Get(attribute);
        tCPU::byte upperRight = Bits<2, 3>::Get(attribute);
        tCPU::byte lowerLeft = Bits<4, 5>::Get(attribute);
        tCPU::byte lowerRight = Bits<6, 7>::Get(attribute);

        // grab attributes for the tile
        tCPU::byte upperBits;

        if (!offsetY && !offsetX)
            upperBits = upperLeft;
        else if (offsetY && !offsetX)
            upperBits = upperRight;
        else if (!offsetY && offsetX)
            upperBits = lowerLeft;
        else
            upperBits = lowerRight;

        // render single row within the 8x8 pixel tile
        // each pattern block is 16 bytes long: two sections of 8 bytes each
        // each byte corresponds to 8 pixels. total: 2 bits per pixel
        ushort patternBytes = 16;
        ushort patternSize = 8;
        ushort patternOffset = settings.BackgroundPatternTableAddress + tileIdx * patternBytes;
        auto patternByte0 = ReadByteFromPPU(patternOffset + tileRow);
        auto patternByte1 = ReadByteFromPPU(patternOffset + tileRow + patternSize);

        int startX = 0;
        int endX = 8;

        // fine scrolling works by trimming [0,7] pixels from the first tile
        // ie only render last 3 pixels from tile 1 means succeeding tiles will be rendered at an offset
        if (i == 0)
            startX = horizontalScrollOrigin % 8;
//        if (i == 31)
//            endX = horizontalScrollOrigin % 8;

        // no more inner loop branches! -20k cycles per scanline
//        for (auto column = startX; column < endX; column++) {
        for (int column = startX; column < endX; column++) {
            auto xOffset = 7 - column;

            // this pixel has two color bits from the pattern table available
            tCPU::byte pixelBit0 = (patternByte0 >> xOffset) & 1;
            tCPU::byte pixelBit1 = (patternByte1 >> xOffset) & 1;

            tCPU::byte lowerBits = pixelBit0 | (pixelBit1 << 1);

            tCPU::byte paletteId = GetColorFromPalette(0, upperBits, lowerBits);
            tPaletteEntry &color = colorPalette[paletteId];

            // tile scroll debugging
//            auto bgra = 0xFF << 24 | (Y) << 16 | (((i+tileScroll) % 32 * 8 + column)) << 8 | 0;

            // attribute scroll debug
//            auto bgra = 0xFF << 24 | (attributeX * 32) << 16 | (attributeY * 32) << 8 | 0;

            // red = upper-bits
            // green = lower-bits
//            auto bgra = 0xFF << 24 | (upperBits * 64) << 16 | (lowerBits * 64) << 8 | 0;

//            auto bgra = 0xFF << 24 | (tileIdx) << 16;

            // update final color output
            *(backgroundRender++) = color.ColorValue;

            // update mask
            *(backgroundMask++) = lowerBits * 64;
        }
    }

    // now we have to jump around memory to fetch the palette ids
    // its 1.5x faster doing it here than in the loop above :>

    // do it backwards to make bitblts more native

    //tCPU::byte* spriteRamStream = SPR_RAM;
//    tCPU::dword *spriteRamStream32bit = (tCPU::dword *) SPR_RAM;

    // 64 possible sprites, can only draw 8 of them per scanline
    int numSpritesDrawn = 0;

    // iterate through all sprites and find ones that need to be rendered on this scanline
    if(settings.SpriteVisible)
    for (auto i = 0; i < 256; i += 4) {
        auto spriteY = SPR_RAM[i] + 1;

        if (spriteY == 249) {    // sprite wants to be ignored
            continue;
        }

        auto tileIdx = SPR_RAM[i + 1];
        auto attributes = SPR_RAM[i + 2];

        auto spriteRow = (Y - spriteY) % 8;

        bool renderLargeSprites = settings.SpriteSize == SPRITE_SIZE_8x16;

        // handle 8x16 sprites
        auto patternTable = settings.SpritePatternTableAddress;
        if(renderLargeSprites) {
            // 8x16 sprites
            if (spriteY > Y || (spriteY + 16) <= Y) {    // current scanline does not overlap the sprite
                continue;
            }

            // lsb of tile index determines which pattern table to use
            if((tileIdx & 1) == 0) {
                patternTable = settings.BackgroundPatternTableAddress;
            }

            // are we inside first 8x8 tile of the 8x16 sprite?
            if((spriteY + 8) > Y) {
                // adjust tile number
                tileIdx --;
            }

        } else {
            // 8x8 sprites
            if (spriteY > Y || (spriteY + 8) <= Y) {    // current scanline does not overlap the sprite
                continue;
            }
        }

        auto spriteX = SPR_RAM[i + 3];

        if (spriteY > 238 || spriteX > 248) {
            // not so properly ignored
            continue;
        }

//        PrintInfo("Y=%d tile=0x%X SpriteX=%d and SpriteY=%d", Y, tileIdx, spriteX, spriteY);

        numSpritesDrawn++;

        bool verticalFlip = Bit<7>::IsSet(attributes);
        bool horizontalFlip = Bit<6>::IsSet(attributes);
        bool spriteBehindBG = Bit<5>::IsSet(attributes);
        tCPU::byte colorUpperBits = Bits<0, 1>::Get(attributes);

        // render one row of the sprite

        auto spriteHeight = renderLargeSprites ? 15 : 7;
        auto patternY = verticalFlip ? (spriteHeight - spriteRow) : spriteRow;

        int spriteOffset = patternTable + tileIdx * 16;
        tCPU::byte patternByte0 = ReadByteFromPPU(spriteOffset + patternY);
        tCPU::byte patternByte1 = ReadByteFromPPU(spriteOffset + patternY + 8);

        for (int column = 0; column < 8; column++) {
            auto patternColumn = horizontalFlip ? (column) : (7 - column);
            tCPU::byte pixelBit0 = (patternByte0 >> patternColumn) & 1;
            tCPU::byte pixelBit1 = (patternByte1 >> patternColumn) & 1;

            // lower two bits
            tCPU::byte colorLowerBits = pixelBit0 | (pixelBit1 << 1);

            tCPU::byte paletteId = GetColorFromPalette(1, colorUpperBits, colorLowerBits);

            // absolute position on output screen
            auto screenX = spriteX + column;

            if ((!spriteBehindBG || raster->backgroundMask[Y * 256 + screenX] == 0) && colorLowerBits) {
                tPaletteEntry &color = colorPalette[paletteId];

                // write final color output
                ((int *) raster->screenBuffer)[Y * 256 + screenX] = color.ColorValue;
                // write mask
                raster->spriteMask[Y * 256 + screenX] = colorLowerBits * 64;
                raster->backgroundMask[Y * 256 + screenX] += 0xf0;
            }

            // test sprite-0 hit detection against background mask
            if (!sprite0HitInThisFrame && !sprite0HitInThisScanline && i == 0) {
                // if we got a sprite0 color pixel over a non-transparent background pixel
                // a transparent pixel has lower two bits = 0 (should do 'bg & 0x3')
                if ((raster->backgroundMask[Y * 256 + screenX]) != 0 && colorLowerBits != 0) {
                    sprite0HitInThisScanline = true;
//                    PrintInfo("Sprite-0 collision detected");
                }
            }
        }
    }

    if (numSpritesDrawn >= 8) {
        //PrintPpu("sprite overflow!" );
        statusRegister |= Bit<5>::Set(true);
    }
}

// palettetype: 0 = background, 1 = sprites
tCPU::byte
PPU::GetColorFromPalette(int paletteType, int upperBits, int lowerBits) {
    tCPU::byte Color = 0;

    if(lowerBits == 0) {
        // specifically for super mario brothers
        return ReadByteFromPPU(0x3F00);
    }

    // address is 5 bits long
    if (lowerBits <= 3) {
        Color = ReadByteFromPPU(0x3F00 | paletteType << 4 | upperBits << 2 | lowerBits);
    } else {
        PrintError("PPU::GetColorFromPalette(%d, %d); Invalid Color Id", upperBits, lowerBits);
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

    tCPU::byte nameTableIdx = value & 0x3; // bits 0 + 1
    settings.NameTableAddress = 0x2000 + nameTableIdx * 0x400;

//    PrintInfo("Setting nametable address to 0x%X", settings.NameTableAddress);

    // increment vram address (on port $2007 activity) by 1 (horizontal) or 32 (vertical) bytes
    settings.DoVerticalWrites = bits.test(2);
    settings.SpritePatternTableAddress = bits.test(3) ? 0x1000 : 0x0000;
    settings.BackgroundPatternTableAddress = bits.test(4) ? 0x1000 : 0x0000;

    settings.SpriteSize = bits.test(5) ? SPRITE_SIZE_8x16 : SPRITE_SIZE_8x8;

//    PrintInfo("settings.SpriteSize = 8x16 = %d", settings.SpriteSize);

//    PrintInfo("Set control register 1; value = %X", value);

    settings.GenerateInterruptOnSprite = bits.test(6);
    settings.GenerateInterruptOnVBlank = bits.test(7);

    tempVRAMAddress &= 0xF3FF;
    tempVRAMAddress |= (value & 0x03) << 10;

//    PrintInfo("Set base nametable address to %X", settings.NameTableAddress);
//    PrintInfo("Set base background pattern table address to %X", settings.BackgroundPatternTableAddress);
//    PrintInfo("Set base sprite pattern table address to %X", settings.SpritePatternTableAddress);
//    PrintInfo("Generate an NMI at start of vertical blanking interval: %d", settings.GenerateInterruptOnVBlank);
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
 * $2006
 */
void
PPU::setVRamAddressRegister2(tCPU::byte value) {
    if (firstWriteToSFF2) {
        // first write -- high byte
        latchedVRAMByte = value;
        tempVRAMAddress &= 0x80FF;
        tempVRAMAddress |= (value & 0x3F) << 8; // 6 bits to high byte

//        PrintInfo("tempVRAMAddress first part = 0x%X (0x%X)", tempVRAMAddress, value);
    } else {
        // second write -- low byte
//        vramAddress14bit = ((tCPU::word) latchedVRAMByte) << 8;
//        vramAddress14bit |= value;

        tempVRAMAddress &= 0xFF00; // clear low byte
        tempVRAMAddress |= value; // set low byte
        vramAddress14bit = tempVRAMAddress;
//        vramAddress14bit &= 0x7FFF; // clear highest bit

//        PrintInfo("2nd write to $2006 @ scanline = %d & pixel = %d : vramAddress14bit = 0x%X",
//                  currentScanline, scanlinePixel, vramAddress14bit);

//        auto tileScrollOffset = vramAddress14bit & 0x0FFF;
//        PrintInfo("$2006; tile scroll x = %d", tileScrollOffset);
    }

    // flip
    firstWriteToSFF2 = !firstWriteToSFF2;
}

tCPU::byte
PPU::readFromVRam() {
    tCPU::byte value;

    if (vramAddress14bit % 0x4000 <= 0x3EFF) {    // latch value, return old
        value = latchedVRAMByte;
        latchedVRAMByte = ReadByteFromPPU(vramAddress14bit);
//        PrintInfo("New Latch = 0x%02X; Returning Old Latch value 0x%02X", (int) latchedVRAMByte, (int) value);
    } else {
        value = ReadByteFromPPU(vramAddress14bit);
//        PrintInfo("Returning Direct (Non-Latched) VRAM value: 0x%02X", (int) value);
    }

    AutoIncrementVRAMAddress();

    return value;
}

void
PPU::writeToVRam(tCPU::byte value) {
    WriteByteToPPU(vramAddress14bit, value);
    AutoIncrementVRAMAddress();
}

tCPU::word
PPU::GetEffectiveAddress(tCPU::word address) {
    if (address < 0x2000) {
        // pattern table chr-rom page

        // use a memory mapper if one is available
        if(mapper != nullptr) {
           address = mapper->getEffectivePPUAddress(address);
        }

//        PrintInfo("Address is < 0x2000; Referencing CHR-ROM @ $%04X", address);

        return address;

    }

    // PPU
//    if (address >= 0x2008 && address <= 0x3FFF) {
//        // mirror [$2000,$2007] on repeat
//        auto mirrored = address - 0x2008 % 8;
//        PrintInfo("Address in range: (0x2008-0x3FFF); Mirror of $%04X", mirrored);
//    }

    if (address >= 0x2000 && address < 0x3000) {
        /*
         *        (0,0)     (256,0)     (511,0)
         *          +-----------+-----------+
         *          |           |           |
         *          |           |           |
         *          |   $2000   |   $2400   |
         *          |     #0    |     #1    |
         *          |           |           |
         *   (0,240)+-----------+-----------+(511,240)
         *          |           |           |
         *          |           |           |
         *          |   $2800   |   $2C00   |
         *          |     #2    |     #3    |
         *          |           |           |
         *          +-----------+-----------+
         *        (0,479)   (256,479)   (511,479)
         */
        int nametableId = floor((address - 0x2000) / 0x400);
        int offset = address % 0x400;

        // nametables mirroring
        if (settings.mirroring == VERTICAL_MIRRORING) {
            if (nametableId == 2)
                nametableId = 0;
            if (nametableId == 3)
                nametableId = 1;
        }

        if (settings.mirroring == HORIZONTAL_MIRRORING) {
            if (nametableId == 1)
                nametableId = 0;
            if (nametableId == 3)
                nametableId = 2;
        }

        address = nametableId * 0x400 + 0x2000 + offset;

//        PrintInfo("Mirrored 0x%X to 0x%X (nametableId=%d)", address, mirrored, nametableId);
//        address = mirrored;
    }

    if (address >= 0x3000 && address <= 0x3EFF) {
        // mirror of 2000h-2EFFh
        auto mirror = address - 0x1000;
//        PrintInfo("Address in range: (0x3000-0x3EFF); Mirror of $%04X", mirror);
        address = mirror;
    }

    if (address >= 0x3F20 && address <= 0x3FFF) {
        // 7 mirrors of 3F00h-3F1Fh :(
//        PrintInfo("Hit 7 Mirror Address: 0x%X", (int) address);
        address -= ((address - 0x3F20) % 0x1F) * 0x1F;
//        PrintInfo("-> Resolved it to: 0x%X", (int) address);
    }

    // mirrors 3F10h,3F14h,3F18h,3F1Ch -> 3F00h,3F04h,3F08h,3F0Ch
    // FIXME: im not sure if these mirrors are ranges or just single byte entries
    if (address == 0x3F10 || address == 0x3F14 || address == 0x3F18 || address == 0x3F1C)
        address -= 0x10;

    if (address >= 0x3F00 && address <= 0x3F1F) {
//        PrintDbg("address $%04X is a BG/Sprite Palette!", address);
    }

    if (address >= 0x4000) {
//        PrintInfo("Address in range: (0x4000-0x10000); Mirror of $%04X? (FIXME: writing through)", address % 0x4000);
//        address = address % 0x4000;
    }

    return address;
}

tCPU::byte
PPU::ReadByteFromPPU(tCPU::word Address) {
    tCPU::word EffectiveAddress = GetEffectiveAddress(Address);
    tCPU::byte Value = PPU_RAM[EffectiveAddress];
//    PrintPpu("Read 0x%02X from PPU RAM @ 0x%04X", (int) Value, (int) EffectiveAddress);
    return Value;
}

bool
PPU::WriteByteToPPU(tCPU::word Address, tCPU::byte Value) {
    tCPU::word EffectiveAddress = GetEffectiveAddress(Address);
    PPU_RAM[EffectiveAddress] = Value;

    if(Address >= 0x8000) {
        PrintInfo("Wrote 0x%02X to PPU RAM @ 0x%04X (0x%04X)", Value, EffectiveAddress, Address);
    }
    if (EffectiveAddress >= 0x3f00 && EffectiveAddress < 0x4000) {
        PrintDbg("Wrote 0x%02X to PPU RAM @ 0x%04X (0x%04X)", Value, EffectiveAddress, Address);
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
        horizontalScrollOrigin = value & 0x7;

        tempVRAMAddress &= 0xFFE0;
        tempVRAMAddress |= value >> 3;

        auto coarseX = tempVRAMAddress & 0x001F;
//        PrintInfo("coarse x = %d / fine x = %d", coarseX, horizontalScrollOrigin);
    } else {
        // second write
        verticalScrollOrigin = value;

        tempVRAMAddress &= 0x8FFF;
        tempVRAMAddress |= (value & 0x07) << 12;
        tempVRAMAddress &= 0xFC1F;
        tempVRAMAddress |= (value & 0xF8) << 2;


//        PrintInfo("tempVRAMAddress = 0x%04X second", (int) tempVRAMAddress);
//        PrintInfo("scanline = %d / pixel = %d / verticalScrollOrigin = %d / tempVRAMAddress = 0x%X",
//                  currentScanline, scanlinePixel, verticalScrollOrigin, tempVRAMAddress);

//        auto tileScrollOffset = vramAddress14bit & 0x0FFF;
//        PrintInfo("$2005; tile scroll x = %d", tileScrollOffset);
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
    PrintDbg("DMA Transfer from RAM @ $%04X to SPR-RAM (scanline: %d, pixel: %d)", (int) startAddress, currentScanline, scanlinePixel);

    for (tCPU::word i = 0; i < 256; i++) {
        SPR_RAM[i] = memory->readByte(startAddress + i);
    }

//    vramAddress14bit = 0;
}

void PPU::clear() {
    // clear final output (256x256@32bit)
    uint32_t clearPattern = 0xff333333;
    memset_pattern4(raster->screenBuffer, &clearPattern, 256 * 256 * 4);

    // clear pattern table debug view (128x256@32bit)
    memset_pattern4(raster->patternTable, &clearPattern, 128 * 256 * 4);

    // clear palette table debug view (256x32@32bit)
    memset_pattern4(raster->palette, &clearPattern, 256 * 32 * 4);

    // clear sprite mask debug view (256x256@16bit)
    memset(raster->spriteMask, 128, 256 * 256 * 2);

    // clear background mask debug view (256x256@16bit)
    memset(raster->backgroundMask, 128, 256 * 256 * 2);
}

void PPU::renderDebug() {
//    RenderBackgroundTiles();
//    RenderSpriteTiles();

    RenderDebugNametables();
    RenderDebugAttributes(settings.NameTableAddress);
    RenderDebugPatternTables();
    RenderDebugColorPalette();
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

        auto upperBits = Bits<1, 2>::Get(attributes);

        // render 8x8 block

        int offsetY = y * 256 * 4;
        int offsetX = x * 4;

        if (y >= 239 || y < 0) {
            continue;
        }

//        PrintInfo("%d: x/y = %d,%d: flip bits horizontal=%x vertical=%x", i, x, y, Bit<6>::QuickIsSet(attributes), Bit<7>::QuickIsSet(attributes));

        auto flipH = Bit<6>::QuickIsSet(attributes);

        // TODO: handle 8x16 sprites
        auto tileIdx = tileNumber;
        auto patternTable = settings.SpritePatternTableAddress;
        if(settings.SpriteSize == SPRITE_SIZE_8x16) {
            if((tileNumber & 1) == 0) {
                patternTable = settings.BackgroundPatternTableAddress;
            }

            tileIdx --;
        }

        // row
        for (auto k = 0; k < 8; k++) {
            tCPU::byte PatternByte0 = PPU_RAM[patternTable + tileIdx * 16 + k];
            tCPU::byte PatternByte1 = PPU_RAM[patternTable + tileIdx * 16 + k + 8];

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
            }
        }

        if(settings.SpriteSize == SPRITE_SIZE_8x16) {
            if((tileNumber & 1) == 0) {
                patternTable = settings.BackgroundPatternTableAddress;
            } else {
                patternTable = settings.SpritePatternTableAddress;
            }
            tileIdx ++;

            int offsetY = (y + 8) * 256 * 4;
            for (auto k = 0; k < 8; k++) {
                tCPU::byte PatternByte0 = PPU_RAM[patternTable + tileIdx * 16 + k];
                tCPU::byte PatternByte1 = PPU_RAM[patternTable + tileIdx * 16 + k + 8];

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
                }
            }
        }
    }
}

/**
 * Render background tiles onto final output
 */
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

    auto tileScrollOffset = 0;// vramAddress14bit & 0x0FFF;
    auto attrScrollOffset = 0;//(vramAddress14bit & 0x0C00) | ((vramAddress14bit >> 4) & 0x38) | ((vramAddress14bit >> 2) & 0x07);

    // rows (240 pixels vertically)
    for (unsigned int i = 0; i < 30; i++) {
        // columns (256 pixels horizontally)
        for (unsigned int j = 0; j < 32; j++) {
            // 8x8 tile number
            // first 960 bytes contain tiles
            tCPU::byte tileNumber = PPU_RAM[NametableAddress + tileScrollOffset + i * 32 + j];
            int offsetY = i * 8 * 256 * 4;
            int offsetX = j * 8 * 4;

            auto AttrI = (int) floor(j / 4.0);
            auto AttrJ = (int) floor(i / 4.0);

            // after tiles, we have two scanlines for attributes
            tCPU::byte attribute = PPU_RAM[NametableAddress + attrScrollOffset + 0x3C0 + AttrJ * 8 + AttrI];

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

                    raster->backgroundMask[offsetBytes / 4] = lowerBits ? 0x90 : 0x00;

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

    int base = 0x0000;

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
                auto pattern1 = ReadByteFromPPU(base + rowSrc + 0);
                auto pattern2 = ReadByteFromPPU(base + rowSrc + 8);

                // columns
                for (auto l = 0; l < 8; l++) {
                    auto pixel1 = pattern1 >> (7 - l) & 1;
                    auto pixel2 = pattern2 >> (7 - l) & 1;
                    auto color = pixel1 | (pixel2 << 1);

                    tCPU::byte paletteId = GetColorFromPalette(0, 0, color);
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
void PPU::RenderDebugAttributes(tCPU::word NametableAddress) {
    // clear attribute table debug view (128x128@32bit)
    for (int x = 0; x < 128; x++) {
        for (int y = 0; y < 128; y++) {
            raster->attributeTable[y * 128 * 4 + x * 4 + 0] = 0x33; // b
            raster->attributeTable[y * 128 * 4 + x * 4 + 1] = 0x33; // g
            raster->attributeTable[y * 128 * 4 + x * 4 + 2] = 0x33; // r
            raster->attributeTable[y * 128 * 4 + x * 4 + 3] = 0xff; // alpha
        }
    }

    auto attrScrollOffset = 0;//(vramAddress14bit & 0x0C00) | ((vramAddress14bit >> 4) & 0x38) | ((vramAddress14bit >> 2) & 0x07);

    // rows
    for (int i = 0; i < 8; i++) {
        // columns
        for (int j = 0; j < 8; j++) {
            // 32x32 tile attributes, 2 bits for each 16x16 sub-tile (which in effect is 4 pattern tiles)
            tCPU::byte attribute = PPU_RAM[NametableAddress + attrScrollOffset + 0x3C0 + i * 8 + j];

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

void PPU::RenderDebugColorPalette() {// render color palette
    // colors
    for (auto nametableId = 0; nametableId < 4; nametableId++) {
        for (auto p = 0; p < 32; p++) {

            auto size = 8;
            auto paletteType = 0; // 0 = background, 1 = sprite
            auto paletteId = ReadByteFromPPU(0x3F00 + paletteType * 16 + p + nametableId * 4);
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
    // clear nametable debug view (512x512@32bit)
    uint32_t clearPattern = 0xff333333;
    memset_pattern4(raster->nametables, &clearPattern, 512 * 512 * 4);

    uint32_t* nametables = (uint32_t*) raster->nametables;

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

                    auto nametableAddress = 0x2000 + a * 2048 + b * 1024;

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
                    int offsetY = (i * 8 * 512) + a * 8 * 512 * 32;
                    int offsetX = (j * 8) + b * 8 * 32;

                    // row
                    for (unsigned short k = 0; k < 8; k++) {
                        // lookup row byte pattern
                        tCPU::byte PatternByte0 = ReadByteFromPPU(settings.BackgroundPatternTableAddress + tileNumber * 16 + k);
                        tCPU::byte PatternByte1 = ReadByteFromPPU(settings.BackgroundPatternTableAddress + tileNumber * 16 + k + 8);

                        // column
                        for (short l = 0; l < 8; l++) {
                            int pixelBit0 = PatternByte0 & (1 << l) ? 255 : 0;
                            int pixelBit1 = PatternByte1 & (1 << l) ? 255 : 0;
                            tCPU::byte lowerBits = (pixelBit0 ? 1 : 0) + (pixelBit1 ? 2 : 0);
                            tCPU::byte paletteId = GetColorFromPalette(0, upperBits, lowerBits);

                            tPaletteEntry &color = colorPalette[paletteId];

                            auto offsetBlockY = offsetY + offsetX + k * 512;
                            auto offsetWords = offsetBlockY + (7 - l);

                            // write 4 bytes BGRA
                            // offset into correct quadrant
                            nametables[offsetWords] = color.ColorValue;
                        }
                    }
                }
            }
        }
    }

    // render viewport scrolling
    ushort tileScroll = ushort(vramAddress14bit & 0x0FFF);
    ushort tileScrollPixels = tileScroll * 8;
    ushort nametableOffsetX = ((settings.NameTableAddress - 0x2000) / 0x400) * 256;
    ushort horizontalScrollPixels = nametableOffsetX + horizontalScrollOrigin + tileScrollPixels;

    // rows
    for (auto i = 0; i < 256; i++) {
        // left vertical line
        auto offsetBytes = (i * 512 + (horizontalScrollPixels) % 512) * 4;
        raster->nametables[offsetBytes + 0] = 0xff; // b
        raster->nametables[offsetBytes + 1] = 0xff; // g
        raster->nametables[offsetBytes + 2] = 0xff; // r
        raster->nametables[offsetBytes + 3] = 0xff; // alpha

        // right vertical line
        offsetBytes = (i * 512 + (horizontalScrollPixels + 255) % 512) * 4;
        raster->nametables[offsetBytes + 0] = 0xff; // b
        raster->nametables[offsetBytes + 1] = 0xff; // g
        raster->nametables[offsetBytes + 2] = 0xff; // r
        raster->nametables[offsetBytes + 3] = 0xff; // alpha

        // top line
        offsetBytes = ((i + horizontalScrollPixels) % 512) * 4;
        raster->nametables[offsetBytes + 0] = 0xff; // b
        raster->nametables[offsetBytes + 1] = 0xff; // g
        raster->nametables[offsetBytes + 2] = 0xff; // r
        raster->nametables[offsetBytes + 3] = 0xff; // alpha

        // bottom line
        offsetBytes = (255 * 512 + (i + horizontalScrollPixels) % 512) * 4;
        raster->nametables[offsetBytes + 0] = 0xff; // b
        raster->nametables[offsetBytes + 1] = 0xff; // g
        raster->nametables[offsetBytes + 2] = 0xff; // r
        raster->nametables[offsetBytes + 3] = 0xff; // alpha
    }
}

void
PPU::loadRom(Cartridge &rom) {
    for (uint8_t i = 0; i < rom.header.numChrPages; i++) {
        writeChrPage(i, rom.characterDataPages[i].buffer);
    }

    settings.mirroring = rom.info.mirroring;
}

/**
 * Write 8kB chr-rom pages to PPU memory
 * Populates the pattern tables. MMC roms can bank switch from here.
 */
void
PPU::writeChrPage(uint16_t page, uint8_t buffer[]) {
    memcpy(PPU_RAM, buffer, CHR_ROM_PAGE_SIZE);
}

void
PPU::useMemoryMapper(MemoryMapper *mapper) {
    this->mapper = mapper;

}

