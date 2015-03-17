#include "Cartridge.h"
#include "CartridgeLoader.h"
#include "Logging.h"
#include "CPU.h"

int main() {
    el::Configurations defaultConf;
    defaultConf.setToDefault();

//    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format,
//            "\033[1;30m╭──\033[0;33m %level\033[1;30m / %fbase:%line / %func\n\033[1;30m╰─>\033[1;37m %msg\n");
//    defaultConf.set(el::Level::Info, el::ConfigurationType::Format,
//            "\033[1;30m╭──\033[0;33m %level\033[1;30m / %fbase:%line / %func\n\033[1;30m╰─>\033[1;37m %msg\n");
//    defaultConf.set(el::Level::Error, el::ConfigurationType::Format,
//            "\033[1;30m╭──\033[0;31m %level\033[1;30m / %fbase:%line / %func\n\033[1;30m╰─>\033[1;37m %msg\n");
//    defaultConf.set(el::Level::Warning, el::ConfigurationType::Format,
//            "\033[1;30m╭──\033[1;31m %level\033[1;30m / %fbase:%line / %func\n\033[1;30m╰─>\033[1;37m %msg\n");

    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format,
            "\033[0;33m%level\033[1;30m |\033[1;30m %msg \033[1;30m @ %fbase:%line");
    defaultConf.set(el::Level::Info, el::ConfigurationType::Format,
            "\033[0;33m%level\033[1;30m |\033[1;37m %msg \033[1;30m @ %fbase:%line");
    defaultConf.set(el::Level::Error, el::ConfigurationType::Format,
            "\033[0;31m%level\033[1;30m |\033[0;31m %msg \033[1;30m @ %fbase:%line");
    defaultConf.set(el::Level::Warning, el::ConfigurationType::Format,
            "\033[0;31m%level\033[1;30m |\033[0;31m %msg \033[1;30m @ %fbase:%line");

    el::Loggers::reconfigureLogger("default", defaultConf);

    TIMED_FUNC(root);

    CartridgeLoader loader;
    Cartridge rom = loader.loadCartridge("../roms/supermariobros.nes");

    CPU cpu;
    cpu.load(rom);

    cpu.run();
}

