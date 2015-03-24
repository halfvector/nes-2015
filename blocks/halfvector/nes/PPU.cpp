#include "PPU.h"
#include "Logging.h"
#include <bitset>

PPU::PPU() {
    firstWriteToSFF = true;

    tCPU::byte vblankFlag = 1 << 7;
    tCPU::byte hitFlag = 1 << 6;
    statusRegister = vblankFlag | hitFlag;
}

// everynes.txt has details
tCPU::byte
PPU::getStatusRegister() {
    tCPU::byte returnValue = statusRegister;

    PrintDbg("PPU; Status register: %s") % std::bitset<8>(statusRegister).to_string();
    PrintDbg("PPU; -> Scanline: %d, HBlank: %d, Pixel: %d") % currentScanline
            % inHBlank % scanlinePixel;

    // reset vblank and hit flags to zero
    statusRegister &= ~(1 << 7);
    //statusRegister &= ~(1 << 6);

    // reset 2005 and 2006 write modes
    firstWriteToSFF = true;

    return returnValue;
}

void
PPU::execute(int numCycles) {
    for(int i = 0; i < numCycles; i ++) {
        if(currentScanline < 240) {
            // process scanline
            processScanline();
        } else
        if( currentScanline < 243) {
            // process hblank until we reset to the start of the next scanline
            if( ++scanlinePixel == 341 ) {
                currentScanline ++;
                scanlinePixel = 0;
            }
        } else
        if(currentScanline == 243 && scanlinePixel == 0) {
            statusRegister |= 1 << 7;
        } else
        if(currentScanline >= 262) {
            currentScanline = 0;
            scanlinePixel = 0;
            inHBlank = false;
            inVBlank = false;

            // reset vblank and overflow flags
            statusRegister &= ~(1 << 7);
            statusRegister &= ~(1 << 5);
        }
    }
}

void
PPU::processScanline() {
    if( scanlinePixel < 255 ) {
        // drawing pixels
        inHBlank = false;
    }
    else if( scanlinePixel == 255 ) {
        // last pixel of the scanline, right before hblank
//        EnterHBlank();
        inHBlank = true;
    }
    else if( scanlinePixel < 340 ) {
        inHBlank = true;
    }
    else
    {	// finished hblank; leaving hblank and increment scanline
        inHBlank = false;
        currentScanline ++;
        scanlinePixel = -1;

//        if( Settings.BackgroundVisible.Value && Settings.SpriteVisible.Value )
//        {
//            m_14bitVRAMAddress &= 0xFBE0;
//            m_14bitVRAMAddress |= (m_TempVRAMAddress & 0x041F);
//            m_14bitVRAMAddress &= 0x7FFF;
//        }

        //PrintNotice( "Start of Scanline; m_14bitVRAMAddress = 0x%X / m_TempVRAMAddress = %s", m_14bitVRAMAddress, Word::AsBinary( m_TempVRAMAddress ).c_str() );
    }

    scanlinePixel ++;
}