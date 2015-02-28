#include <iostream>
#include "boost/format.hpp"
#include "boost/filesystem.hpp"
#include "CartridgeFormat.h"
#include "CartridgeLoader.h"
#include "Logging.h"

using namespace boost;
using namespace boost::filesystem;

INITIALIZE_EASYLOGGINGPP;

el::Logger *log = el::Loggers::getLogger("default");

int main() {
    TIMED_FUNC(root);

    // run performance tests
    //simpleLoopPerformanceTests();

    LOG(INFO) << "CWD: " << boost::filesystem::current_path();

    CartridgeLoader loader;
    Cartridge rom = loader.readCartridge("../roms/Super Mario Bros (E).nes");
}

