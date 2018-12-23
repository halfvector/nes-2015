#include "PPU.h"
#include "Logging.h"

MemoryMapper::MemoryMapper(unsigned char *ppuRam, unsigned char *cpuRam) {
    this->PPU_RAM = ppuRam;
    this->CPU_RAM = cpuRam;
    this->PRG_BANKS = new unsigned char[0x20000]; // 128KiB (ie megaman1)
    chrBank = 0;
    prgBank = 0;
}

void
MemoryMapper::loadRom(Cartridge &rom) {
    PrintInfo("Initializing Memory Mapper #%d", rom.info.memoryMapperId);

    for (uint8_t i = 0; i < rom.header.numChrPages; i++) {
        PrintInfo("  Writing chr page %d to CPU @ 0x%X", i, 0x8000 + CHR_ROM_PAGE_SIZE * i);
        memcpy(PPU_RAM + 0x8000 + CHR_ROM_PAGE_SIZE * i, rom.characterDataPages[i].buffer, CHR_ROM_PAGE_SIZE);
    }

    for (uint8_t i = 0; i < rom.header.numPrgPages; i++) {
        PrintInfo("  Writing prg page %d to PPU @ 0x%X", i, 0x10000 + PRG_ROM_PAGE_SIZE * i);
        memcpy(CPU_RAM + 0x10000 + PRG_ROM_PAGE_SIZE * i, rom.programDataPages[i].buffer, PRG_ROM_PAGE_SIZE);
        memcpy(PRG_BANKS + PRG_ROM_PAGE_SIZE * i, rom.programDataPages[i].buffer, PRG_ROM_PAGE_SIZE);
    }

    memoryMapperId = rom.info.memoryMapperId;

    switch(memoryMapperId) {
        case MEMORY_MAPPER_UNROM: {
            // last bank into second PRG ROM position
            PrintInfo("Loading last PRG ROM into CPU 0xC000");
            memcpy(CPU_RAM + 0xC000, rom.programDataPages[rom.header.numPrgPages - 1].buffer, PRG_ROM_PAGE_SIZE);
            prgBank = rom.header.numPrgPages - 1;
            // chose bank switching mask based on number of pages to switch
            // use first 2 bits for switching between 4 pages
            // use first 3 bits for switching between 7 pages (eg: Metal Gear)
            prgBankMask = rom.header.numPrgPages > 4 ? 0x7 : 0x3;
        } break;

        case MEMORY_MAPPER_CNROM: {
            // last bank into second CHR ROM position
            PrintInfo("Loading last CHR ROM into PPU 0x0000");
            memcpy(PPU_RAM, rom.characterDataPages[0].buffer, CHR_ROM_PAGE_SIZE);
            chrBank = 0;
        } break;


        default: {
            PrintInfo("Unsupported memory mapper %d", memoryMapperId);
        } break;
    }
}

/**
 * Calculate PPU_RAM address for pattern table using a memory mapper
 */
tCPU::word
MemoryMapper::getEffectivePPUAddress(tCPU::word address) {
    switch(memoryMapperId) {
        case MEMORY_MAPPER_NROM: {
            // address is unchanged
        } break;

        case MEMORY_MAPPER_UNROM: {
            // address is unchanged
        } break;

        case MEMORY_MAPPER_CNROM: {
            // switch between four 8KiB banks. capped at 32KiB of CHR.
            address = address + 0x8000 + chrBank * CHR_ROM_PAGE_SIZE;
        } break;

        default: {
            PrintInfo("Unsupported memory mapper %d", memoryMapperId);
        } break;
    }


//    switch(address) {
//        case 0x0000 ... 0x07ff:
//            bank = CPU_RAM[0x8800] & 0x3;
//            break;
//        case 0x0800 ... 0x0fff:
//            bank = CPU_RAM[0x9800] & 0x3;
//            break;
//        case 0x1000 ... 0x17ff:
//            bank = CPU_RAM[0xA800] & 0x3;
//            break;
//        case 0x1800 ... 0x1fff:
//            bank = CPU_RAM[0xB800] & 0x3;
//            break;
//        default:
//            PrintInfo("Unhandled case: address=0x%X", address);
//    }

//    if (bank != 0) {
//        PrintInfo("bank = %d", bank);
//    }

    // map into CHR-ROM banks in PPU-RAM
    return address;
}

void
MemoryMapper::writeByteCPUMemory(tCPU::word address, tCPU::byte value) {
    // address must be in range [$4020-$FFFF]
    // this is space the CPU can directly address (16 bit)
    // and it belongs exclusively to the cartridge
    // writes here will likely be mapper registers

    switch(memoryMapperId) {
        case MEMORY_MAPPER_NROM:
            // do nothing
            CPU_RAM[address] = value;
            break;

        case MEMORY_MAPPER_UNROM: {
            if(address >= 0x8000 && address <= 0xFFFF) {
                auto newBank = value & prgBankMask; // grab lower 2 or 3 bits
                if(newBank != prgBank) {
//                    PrintInfo("switching to PRG bank = %d", newBank);
                }
                prgBank = newBank;
            } else {
                CPU_RAM[address] = value;
            }
        } break;

        case MEMORY_MAPPER_CNROM: {
            if(address >= 0x8000 && address <= 0xFFFF) {
                auto newBank = value & 0x3; // only lower 2 bits
                if(newBank != chrBank) {
//                    PrintInfo("switching to CHR bank = %d", newBank);
                }
                chrBank = newBank;
            } else {
                CPU_RAM[address] = value;
            }
        } break;

        default: {
            PrintInfo("Unsupported memory mapper %d", memoryMapperId);
        } break;
    }
}

tCPU::byte
MemoryMapper::readByteCPUMemory(tCPU::word address) {
    switch(memoryMapperId) {
        case MEMORY_MAPPER_NROM:
            // address is unchanged
            break;

        case MEMORY_MAPPER_UNROM: {
            // switch between four 16KiB PRG banks for the first ROM bank
            if(address < 0xC000) {
                auto relative = address - 0x8000;
                auto adjusted = relative + 0x10000 + prgBank * PRG_ROM_PAGE_SIZE;
                auto value = CPU_RAM[adjusted];
                return value;
            }
        } break;

        case MEMORY_MAPPER_CNROM: {
            // address is unchanged
        } break;

        default: {
            PrintInfo("Unsupported memory mapper %d", memoryMapperId);
        } break;
    }

    return CPU_RAM[address];
}
