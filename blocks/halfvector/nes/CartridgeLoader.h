#pragma once

#include "Cartridge.h"

class CartridgeLoader {
public:
    Cartridge loadCartridge(char const *filePath);

protected:
    void readHeader(std::fstream &, Cartridge &);
    void readData(std::fstream &, Cartridge &);
    void analyzeHeader(Cartridge &);
};