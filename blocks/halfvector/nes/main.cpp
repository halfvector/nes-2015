#include "Cartridge.h"
#include "CartridgeLoader.h"
#include "Logging.h"
#include "CPU.h"

int main() {
    TIMED_FUNC(root);

    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format, "%level | %msg");
    defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%level | %msg");
    el::Loggers::reconfigureLogger("default", defaultConf);

    CartridgeLoader loader;
    Cartridge rom = loader.loadCartridge("../roms/supermariobros.nes");

    CPU cpu;
    cpu.load(rom);

    cpu.run();
}

