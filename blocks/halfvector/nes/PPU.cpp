#include "PPU.h"
#include "Logging.h"
#include <bitset>

PPU::PPU() {
    firstWriteToSFF = true;
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