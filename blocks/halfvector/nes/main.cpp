#include <iostream>
#include "boost/format.hpp"
#include "boost/filesystem.hpp"
#include "halfvector/easylogging/easylogging++.h"
#include "rom.h"

using namespace boost;
using namespace boost::filesystem;

INITIALIZE_EASYLOGGINGPP

void loadCartridge(char const *string, Cartridge rom) {
    std::fstream fh;
    fh.open(string, std::fstream::in | std::fstream::binary);

    if(!fh.is_open())
        throw std::runtime_error((format("failed to open rom: %s") % string).str());

    fh.read((char *) &rom.header, sizeof(rom.header));

    LOG(INFO) << "signature: " << rom.header.signature;
    LOG(INFO) << "programData: " << format("0x%02x") % rom.header.programData;
    LOG(INFO) << "characterData: " << format("0x%02x") % rom.header.characterData;

    fh.close();
}

int main() {
    TIMED_FUNC(root);

    LOG(INFO) << "CWD: " << boost::filesystem::current_path();

    Cartridge cartridge;
    loadCartridge("Super Mario Bros (E).nes", std::move(cartridge));

    root.checkpoint("Cartridge Loaded");
}
