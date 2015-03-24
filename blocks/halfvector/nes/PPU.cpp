#include "PPU.h"
#include "Logging.h"

PPU::PPU() {
    firstWriteToSFF = true;

    // Status register is 0 on boot
    statusRegister = 0;
}

/**
 * Read Status register ($2002)
 */
tCPU::byte
PPU::getStatusRegister() {
    tCPU::byte returnValue = statusRegister;

    PrintDbg("PPU; Status register: %s") % std::bitset<8>(statusRegister).to_string();
    PrintDbg("PPU; -> Scanline: %d, HBlank: %d, Pixel: %d") % currentScanline
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

void
PPU::setVerticalBlank() {
    statusRegister |= 1 << 7;
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

//        if( Settings.BackgroundVisible.Value && Settings.SpriteVisible.Value )
//        {
//            m_14bitVRAMAddress &= 0xFBE0;
//            m_14bitVRAMAddress |= (m_TempVRAMAddress & 0x041F);
//            m_14bitVRAMAddress &= 0x7FFF;
//        }

        //PrintNotice( "Start of Scanline; m_14bitVRAMAddress = 0x%X / m_TempVRAMAddress = %s", m_14bitVRAMAddress, Word::AsBinary( m_TempVRAMAddress ).c_str() );
    }

    scanlinePixel++;
}