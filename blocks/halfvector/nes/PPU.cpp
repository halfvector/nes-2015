#include "PPU.h"
#include "Registers.h"
#include "Logging.h"
#include "Memory.h"
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
        {0x80,0x80,0x80}, {0x00,0x00,0xBB}, {0x37,0x00,0xBF}, {0x84,0x00,0xA6},
        {0xBB,0x00,0x6A}, {0xB7,0x00,0x1E}, {0xB3,0x00,0x00}, {0x91,0x26,0x00},
        {0x7B,0x2B,0x00}, {0x00,0x3E,0x00}, {0x00,0x48,0x0D}, {0x00,0x3C,0x22},
        {0x00,0x2F,0x66}, {0x00,0x00,0x00}, {0x05,0x05,0x05}, {0x05,0x05,0x05},

        {0xC8,0xC8,0xC8}, {0x00,0x59,0xFF}, {0x44,0x3C,0xFF}, {0xB7,0x33,0xCC},
        {0xFF,0x33,0xAA}, {0xFF,0x37,0x5E}, {0xFF,0x37,0x1A}, {0xD5,0x4B,0x00},
        {0xC4,0x62,0x00}, {0x3C,0x7B,0x00}, {0x1E,0x84,0x15}, {0x00,0x95,0x66},
        {0x00,0x84,0xC4}, {0x11,0x11,0x11}, {0x09,0x09,0x09}, {0x09,0x09,0x09},

        {0xFF,0xFF,0xFF}, {0x00,0x95,0xFF}, {0x6F,0x84,0xFF}, {0xD5,0x6F,0xFF},
        {0xFF,0x77,0xCC}, {0xFF,0x6F,0x99}, {0xFF,0x7B,0x59}, {0xFF,0x91,0x5F},
        {0xFF,0xA2,0x33}, {0xA6,0xBF,0x00}, {0x51,0xD9,0x6A}, {0x4D,0xD5,0xAE},
        {0x00,0xD9,0xFF}, {0x66,0x66,0x66}, {0x0D,0x0D,0x0D}, {0x0D,0x0D,0x0D},

        {0xFF,0xFF,0xFF}, {0x84,0xBF,0xFF}, {0xBB,0xBB,0xFF}, {0xD0,0xBB,0xFF},
        {0xFF,0xBF,0xEA}, {0xFF,0xBF,0xCC}, {0xFF,0xC4,0xB7}, {0xFF,0xCC,0xAE},
        {0xFF,0xD9,0xA2}, {0xCC,0xE1,0x99}, {0xAE,0xEE,0xB7}, {0xAA,0xF7,0xEE},
        {0xB3,0xEE,0xFF}, {0xDD,0xDD,0xDD}, {0x11,0x11,0x11}, {0x11,0x11,0x11}
};

tCPU::byte screenAttributes[16][16];

PPU::PPU(Raster* raster) : raster(raster) {
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

    PrintPpu("PPU; Status register: %s") % std::bitset<8>(statusRegister).to_string();
    PrintPpu("PPU; -> Scanline: %d, HBlank: %d, Pixel: %d") % currentScanline
            % inHBlank % scanlinePixel;

    // reset vblank on register read
    statusRegister &= ~(1 << 7);

    // reset $2005 and $2006 write modes
    firstWriteToSFF = true;

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
            if(currentScanline == 243 && scanlinePixel == 0) {
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
        }
    }
}

/**
 * Vertical Blanking Interval started
 */
void
PPU::setVerticalBlank() {
    statusRegister |= 1 << 7;

    // generate nmi trigger if we have one pending
    if(generateInterruptOnVBlank) {
        vblankNmiAwaiting = true;
        generateInterruptOnVBlank = false;
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
    }
    else if (scanlinePixel == 255) {
        // last pixel of the scanline. enter hblank.
        // TODO: perform sprite-0 hit detection
        inHBlank = true;
        onEnterHBlank();
    }
    else if (scanlinePixel < 340) {
        // in hblank
        inHBlank = true;
    }
    else {
        // leave hblank and increment scanline
        inHBlank = false;
        currentScanline++;
        scanlinePixel = -1;

        if (settings.BackgroundVisible && settings.SpriteVisible) {
            vramAddress14bit &= 0xFBE0;
            vramAddress14bit |= (tempVRAMAddress & 0x041F);
            vramAddress14bit &= 0x7FFF;
        }

        PrintPpu("Start of Scanline; vramAddress14bit = 0x%X / tempVRAMAddress = %s")
                % vramAddress14bit % std::bitset<16>(tempVRAMAddress);
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

    bool verticalFlip = Bit<7>::IsSet(attributes);
    bool horizontalFlip = Bit<6>::IsSet(attributes);
    bool spriteInFrontOfBG = !Bit<5>::IsSet(attributes);
    tCPU::byte spritePalette = Bits<0, 1>::Get(attributes);

    // ignore sprite if Y is set to 249
    if (Y == 249) {
        return;
    }

    if (currentScanline >= Y && currentScanline <= (Y + 8)) {
        bool bFound = false;

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

    int reverseY = 255 - Y;

    tCPU::byte UpperBits = 0;

    //return (Y * Width + X) * 3;
    unsigned char* backgroundBufferPtr = raster->backgroundMask + (Y * 256);
    unsigned char* screenBufferPtr = raster->screenBuffer + (Y * 256) * 4;

    // moving these outside the 32x loop shaves off 10kcs
    j = (int) floor( Y / 8.0 );
    int yOffset = (Y % 8);

    int attributeY = (int) floor( Y / 32.0 );


    //int scrollingOffset = (int) floor(horizontalScrollOrigin / 8);
    //unsigned char* ppuRamTileBasePtr = PPU_RAM + NametableAddress + j * 32 + scrollingOffset;

    // decode scanline
    // 256 pixels in 32 bytes, each byte a tile consisting of 8 pixels

    int lastAttributeNumber = -1, lastTileNumber = -1;
    tCPU::byte AttributeByte;
    tCPU::byte* ppuRamBasePtr = PPU_RAM + settings.BackgroundPatternTableAddress + yOffset;

    tCPU::word NametableAddress = settings.NameTableAddress;
    int NametableId = (NametableAddress - 0x2000) / 0x400;

    if( horizontalScrollOrigin >= 256 )
    {
        if( NametableId == 0 )
            NametableAddress += 0x400;
        else
            NametableAddress -= 0x400;

        NametableId = (NametableAddress - 0x2000) / 0x400;
    }

    int TileAddress = NametableAddress + j * 32 + horizontalScrollOrigin / 8;

    tCPU::byte* tileRamBasePtr = PPU_RAM + TileAddress;

    // only need to warp around once

    static tCPU::byte TileCache[33];
    static tCPU::byte AttributeByteCache[33];

    // tiles are 8x8, so we can reuse a row of tiles for 8 scanlines
    // and i think attributes for 16?
    if( (Y % 8) == 0 )
    {
        // cache the next 8 scanlines worth of tiles!
        for( i = 0; i < 33; i ++ )
        {
            int ScrolledX = i * 8 + horizontalScrollOrigin;

            // horizontal scrolled tile position
            int ScrolledI = ScrolledX / 8;

            // only need to wrap around once per scanline (i hope..)
            if( ScrolledX >= 256 )
            {
                NametableAddress = settings.NameTableAddress;
                NametableId = (NametableAddress - 0x2000) / 0x400;

                if( NametableId == 0 )
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
    for( i = 0; i <= 32; i ++ )
    {
        if( i == 32 && (horizontalScrollOrigin % 8) == 0)
            break;

        tCPU::byte Tile = TileCache[i];
        tCPU::byte preScaledUpperBits = AttributeByteCache[i];

        // 8-pixel row within the tile (Y%8)
        tCPU::byte PatternByte0 = PPU_RAM[settings.BackgroundPatternTableAddress + Tile * 16 + 8 * 0 + yOffset];
        tCPU::byte PatternByte1 = PPU_RAM[settings.BackgroundPatternTableAddress + Tile * 16 + 8 * 1 + yOffset];

        int startX = 0;
        int endX = 8;

        if( i == 0 )
            startX = horizontalScrollOrigin % 8;
        if( i == 32 )
            endX = horizontalScrollOrigin % 8;

        tCPU::byte shiftedPattern0 = PatternByte0;
        tCPU::byte shiftedPattern1 = PatternByte1;

        // no more inner loop branches! -20k cycles per scanline
        for( X = startX; X < endX; X ++ )
        {
            int xOffset = 7 - X;

            // this pixel has two color bits from the pattern table available
            tCPU::byte PixelBit0 = (PatternByte0 >> xOffset) & 1;
            tCPU::byte PixelBit1 = (PatternByte1 >> xOffset) & 1;

            tCPU::byte ColorId = PixelBit0 | (PixelBit1 << 1);

            pixelPaletteStream[currentPixel++] = ColorId == 0 ? 0 : ColorId + preScaledUpperBits;

            *(backgroundBufferPtr++) = ColorId;
        }
    }

    // now we have to jump around memory to fetch the palette ids
    // its 1.5x faster doing it here than in the loop above :>

    // do it backwards to make bitblts more native

    int* screenBufferPtr32bit = (int*) (raster->screenBuffer + (reverseY * 256) * 4);

    for( i = 0; i < 256; i ++ )
    {
        tCPU::byte PaletteId = PPU_RAM[0x3F00 + pixelPaletteStream[i]];

        tPaletteEntry& RGBColor = colorPalette[PaletteId];
        *(screenBufferPtr32bit++) = 0xFF << 24 | RGBColor.R << 16 | RGBColor.G << 8 | RGBColor.B;
    }

    //tCPU::byte* spriteRamStream = SPR_RAM;
    tCPU::dword* spriteRamStream32bit = (tCPU::dword*) SPR_RAM;
    tCPU::word* ppuRamPatternBase = (tCPU::word*) PPU_RAM;

    // 64 possible sprites, can only draw 8 of them per scanline
    int numSpritesDrawn = 0;

    for( i = 0; i < 256; i += 4 )
    {
        tCPU::dword spriteEntry = *(spriteRamStream32bit++);

        tCPU::byte SpriteY = (spriteEntry & 0xff) + 1; // *(spriteRamStream++) + 1;

        if( SpriteY == 249 )
        {	// sprite wants to be ignored
            continue;
        }

        if( SpriteY > Y || (SpriteY + 8) <= Y )
        {	// current scanline does not overlap the sprite
            continue;
        }

        tCPU::byte Tile = (spriteEntry >> 8) & 0xff;
        tCPU::byte Attr = (spriteEntry >> 16) & 0xff;
        tCPU::byte SpriteX = (spriteEntry >> 24) & 0xff;

        if( SpriteY > 238 || SpriteX > 248 ) {
            // not so properly ignored
            continue;
        }

        numSpritesDrawn ++;

        bool VerticalFlip = Bit<7>::IsSet( Attr );
        bool HorizontalFlip = Bit<6>::IsSet( Attr );
        bool SpriteBehindBG = Bit<5>::IsSet( Attr );
        tCPU::byte SpritePalette = Bits<0,1>::Get( Attr );


        //if( SpriteY <= Y && (SpriteY + 8) > Y && SpriteX <= X && (SpriteX + 8) > X )
        //if( SpriteY <= Y && (SpriteY + 8) > Y )
        {
            j = Y - SpriteY;
            int ProperY = VerticalFlip ? (7 - j) : j;

            tCPU::byte PatternBit0 = PPU_RAM[settings.SpritePatternTableAddress + Tile * 16 + ProperY + 8 * 0];
            tCPU::byte PatternBit1 = PPU_RAM[settings.SpritePatternTableAddress + Tile * 16 + ProperY + 8 * 1];

            for( X = SpriteX; X < SpriteX + 8; X ++ )
            {
                int l = X - SpriteX;
                int ProperX = HorizontalFlip ? (l) : (7 - l);

                // the result of these logical operations is not 0/1, so we need to clamp them to a bool
                bool PixelBit0 = PatternBit0 & (1 << ProperX) ? 1 : 0;
                bool PixelBit1 = PatternBit1 & (1 << ProperX) ? 1 : 0;

                // lower two bits
                tCPU::byte Color = (PixelBit0 ? 1 : 0) + (PixelBit1 ? 2 : 0);

                //PrintPpu( "(%d & (1 << %d) = %d", PatternBit0, ProperX, (PatternBit0 & (1 << ProperX)));
                //PrintPpu( "  (%d & (1 << %d) = %d", PatternBit1, ProperX, (PatternBit1 & (1 << ProperX)));

                tCPU::byte PaletteId = GetColorFromPalette( 1, SpritePalette, Color );
                tCPU::byte Luminance = (PaletteId & 0xF0) >> 4;
                tCPU::byte Chrominance = (PaletteId & 0x0F);

                //tPaletteEntry& RGBColor = g_ColorPalette[Chrominance + Luminance * 16];

                raster->spriteMask[Y * 256 + X] = Color;

                // FIXME: add support for sprite priorities

                if( ! SpriteBehindBG && Color ) {
                    int* screenBufferPtr32bit = (int*) (raster->screenBuffer + (reverseY * 256) * 4 + X * 4);

                    tPaletteEntry& RGBColor = colorPalette[Chrominance + Luminance * 16];

                    *(screenBufferPtr32bit) = RGBColor.B | RGBColor.G << 8 | RGBColor.R << 16 | 0xff000000;
                    //*(screenBufferPtr32bit) = g_ColorPalette[Chrominance + Luminance * 16].ColorValue;
                    //screenBufferPtr32bit++;
                }

                if( ! sprite0HitInThisFrame && ! sprite0HitInThisScanline && i == 0 ) {
                    // if we got a sprite0 color pixel over a non-transparent background pixel
                    if( raster->backgroundMask[Y * 256 + X] == Color ) {
                        sprite0HitInThisScanline = true;
                    }
                }
            }
        }
    }

    if( numSpritesDrawn >= 8 )
    {
        //PrintPpu("sprite overflow!" );
        statusRegister |= Bit<5>::Set( true );
    }

    PrintPpu("Rendered scanline Y=%d") % Y;
}

// palettetype: 0 = background, 1 = sprites
tCPU::byte
PPU::GetColorFromPalette( int PaletteType, int NameTableId, int ColorId )
{
    tCPU::byte Color = 0;

    if( ColorId == 0 ) {
        Color = PPU_RAM[0x3F00];
    } else if( ColorId <= 3 ) {
        // i guess the landscape of mario is sprites and not background
        // which makes sense cause u want collision detection with landscape..
        Color = PPU_RAM[0x3F00 + PaletteType * 0x10 + ColorId + NameTableId * 4];
    } else {
        PrintError("PPU::GetColorFromPalette(%d, %d); Invalid Color Id")
                % NameTableId % ColorId;
        throw new std::runtime_error("Unexpected error");
    }

    return Color;
}

void
PPU::setControlRegister1(tCPU::byte value) {
    std::bitset<8> bits(value);

    tCPU::byte nameTableIdx = bits.test(0) + bits.test(1);
    nameTableAddress = 0x2000 + nameTableIdx * 0x400;

    // increment vram address (on port $2007 activity) by 1 (horizontal) or 32 (vertical) bytes
    doVerticalWrites = bits.test(2);
    spritePatternTableAddress = bits.test(3) ? 0x1000 : 0x0000;
    backgroundPatternTableAddress = bits.test(4) ? 0x1000 : 0x0000;

    spriteSize = bits.test(5) ? SpriteSizes::SPRITE_8x16 : SpriteSizes::SPRITE_8x8;

    generateInterruptOnSprite = bits.test(6);
    generateInterruptOnVBlank = bits.test(7);

    vramAddress14bit &= 0xF3FF;
    vramAddress14bit |= (value & 0x03) << 10;

    PrintPpu("Generate an NMI at start of vertical blanking interval: %d")
            % generateInterruptOnVBlank;
}

void
PPU::setControlRegister2(tCPU::byte value) {
    std::bitset<8> bits(value);

    displayTypeMonochrome = bits.test(0);
    backgroundClipping = bits.test(1);
    spriteClipping = bits.test(2);
    backgroundVisible = bits.test(3);
    spriteVisible = bits.test(4);
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
        tempVRAMAddress &= 0x00FF;
        tempVRAMAddress |= (value & 0x3F) << 8; // 6 bits to high byte

        PrintPpu("vramAddress14bit first part = 0x%X") % vramAddress14bit;
    } else {
        // second write -- low byte
        vramAddress14bit = ((tCPU::word) latchedVRAMByte) << 8;
        vramAddress14bit |= value;

        tempVRAMAddress &= 0xFF00; // clear low byte
        tempVRAMAddress |= value; // set low byte
        vramAddress14bit = tempVRAMAddress;
        vramAddress14bit &= 0x7FFF; // clear highest bit

        PrintPpu("vramAddress14bit = 0x%X") % vramAddress14bit;
    }

    // flip
    firstWriteToSFF = !firstWriteToSFF;
}

tCPU::byte
PPU::readFromVRam() {
    tCPU::byte value;

    if (vramAddress14bit <= 0x3EFF) {    // latch value, return old
        value = latchedVRAMByte;
        latchedVRAMByte = ReadInternalMemoryByte(vramAddress14bit);
        PrintDbg("New Latch = 0x%02X; Returning Old Latch value 0x%02X")
                % (int) latchedVRAMByte % (int) value;
    } else {
        value = ReadInternalMemoryByte(vramAddress14bit);
        PrintDbg("Returning Direct (Non-Latched) VRAM value: 0x%02X") % (int) value;
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
    if (address >= 0x3000 && address <= 0x3EFF) {
        // mirror of 2000h-2EFFh
        PrintDbg("Address in range: (0x3000-0x3EFF); Mirror of $%04X")
            % (int) (address - 0x1000);
        address -= 0x1000;
    }
    if (address >= 0x3F20 && address <= 0x3FFF) {
        // 7 mirrors of 3F00h-3F1Fh :(
        PrintDbg("Hit 7 Mirror Address: 0x%X") % (int) address;
        address -= ((address - 0x3F20) % 0x1F) * 0x1F;
        PrintDbg("-> Resolved it to: 0x%X") % (int) address;
    }

    // mirrors 3F10h,3F14h,3F18h,3F1Ch -> 3F00h,3F04h,3F08h,3F0Ch
    // FIXME: im not sure if these mirrors are ranges or just single byte entries
    if (address == 0x3F10 || address == 0x3F14 || address == 0x3F18 || address == 0x3F1C)
        address -= 0x10;


    if (address >= 0x3F00 && address <= 0x3F1F) {
        //PrintDbg( "PPU::GetEffectiveAddress( $%04X ); BG/Sprite Palette!", Address );
    }

    if (address >= 0x4000 && address < 0x10000) {
        PrintDbg("Address in range: (0x4000-0x10000); Mirror of $%04X")
            % (int) (address % 0x4000);
        address = address % 0x4000;
    }

    return address;
}

tCPU::byte
PPU::ReadInternalMemoryByte(tCPU::word Address) {
    tCPU::word EffectiveAddress = GetEffectiveAddress(Address);
    tCPU::byte Value = PPU_RAM[EffectiveAddress];
    PrintMemory("Read 0x%02X from PPU RAM @ 0x%04X") % (int) Value % (int) EffectiveAddress;
    return Value;
}

bool
PPU::WriteInternalMemoryByte(tCPU::word Address, tCPU::byte Value) {
    tCPU::word EffectiveAddress = GetEffectiveAddress(Address);
    PPU_RAM[EffectiveAddress] = Value;
    PrintMemory("Wrote 0x%02X to PPU RAM @ 0x%04X") % (int) Value % (int) EffectiveAddress;
    return true;
}

void
PPU::AutoIncrementVRAMAddress() {
    tCPU::byte incAmount = doVerticalWrites ? 32 : 1;
    vramAddress14bit += incAmount;
    PrintDbg("Incremented by %d bytes") % (int) incAmount;
}

void
PPU::setVRamAddressRegister1(tCPU::byte value) {
    if (currentScanline < 240) {
        if (!inHBlank) {
            return;
        }
    }

    if (firstWriteToSFF) {
        // first write
        horizontalScrollOrigin = value;

        tempVRAMAddress &= 0xFFE0;
        tempVRAMAddress |= (value & 0xF8) >> 3;

        tileXOffset = value & 0x07;
        PrintPpu("tileXOffset = %d") % tileXOffset;
    } else {
        // second write
        verticalScrollOrigin = value;

        tempVRAMAddress &= 0xFC1F;
        tempVRAMAddress |= (value & 0xF8) << 2;
        tempVRAMAddress &= 0x8FFF;
        tempVRAMAddress |= (value & 0x07) << 12;
    }

    PrintDbg("tempVRAMAddress = 0x%04X first write = %d")
            % (int) tempVRAMAddress % (int) firstWriteToSFF;
    PrintDbg("-> Background/Sprite Visibility: %d/%d")
            % (int) backgroundVisible % (int) spriteVisible;

    // flip
    firstWriteToSFF =! firstWriteToSFF;
}

void
PPU::setSprRamAddress(tCPU::byte address) {
    spriteRamAddress = address;
}

void
PPU::writeSpriteMemory(tCPU::byte value) {
    if (spriteRamAddress >= 256) {
        PrintError("spriteRamAddress is out of range; Expected < 256, Actual = %d")
            % (int) spriteRamAddress;
    } else {
        PrintDbg("Write byte $%02X to Sprite RAM @ $%02X")
            % (int) value % (int) spriteRamAddress;
        SPR_RAM[spriteRamAddress] = value;
    }

    // address incremented after every write
    spriteRamAddress++;
}

tCPU::byte PPU::readSpriteMemory() {
    tCPU::byte value = SPR_RAM[spriteRamAddress];
    PrintDbg("Read byte $%02X from SPR-RAM @ $%02X")
            % (int) value % (int) spriteRamAddress;
    return value;
}

void
PPU::StartSpriteXferDMA(Memory *memory, tCPU::byte address) {
    tCPU::word startAddress = address * 0x100;
    PrintDbg("DMA Transfer from RAM @ $%04X to SPR-RAM")
            % (int) startAddress;

    for (tCPU::word i = 0; i < 256; i++) {
        SPR_RAM[i] = memory->readByte(startAddress + i);
    }

    vramAddress14bit = 0;
}
