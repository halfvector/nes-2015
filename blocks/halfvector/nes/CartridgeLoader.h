#pragma once

#include "CartridgeFormat.h"

class CartridgeLoader {
public:
    Cartridge readCartridge(char const *string);

protected:
    void readHeader(std::fstream &, Cartridge &);
    void readData(std::fstream &, Cartridge &);
    void analyzeHeader(Cartridge &);
};