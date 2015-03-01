#include <iostream>
#include "boost/format.hpp"
#include "boost/filesystem.hpp"
#include "Cartridge.h"
#include "CartridgeLoader.h"
#include "Logging.h"
#include "CPU.h"

using namespace boost;
using namespace boost::filesystem;

int main() {
    TIMED_FUNC(root);

    CartridgeLoader loader;
    Cartridge rom = loader.loadCartridge("../roms/Super Mario Bros (E).nes");

    CPU cpu;
    cpu.load(rom);

    cpu.run();
}

