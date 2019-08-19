#include "PPU.h"
#include "Logging.h"

MemoryMapper::MemoryMapper(unsigned char *ppuRam, unsigned char *cpuRam) {
    this->PPU_RAM = ppuRam;
    this->CPU_RAM = cpuRam;
//    this->PRG_BANKS = new unsigned char[0x100000]; // 1MiB
    chrBank = 0;
    prgBank = 0;
    chrBank0 = 0;
    chrBank1 = 0;
    shiftRegister = 0x10; // 5th bit set
    control = 0xC;
    prgBankSwapMode = SWAP_UPPER;
    prgBankSwapSize = SWAP_16K;
    prgBankMode = 0x3;
}

void
MemoryMapper::loadRom(Cartridge &rom) {
    for (uint8_t i = 0; i < rom.header.numChrPages; i++) {
        PrintInfo("  Writing chr page %d to PPU @ 0x%X", i, 0x8000 + CHR_ROM_PAGE_SIZE * i);
        memcpy(PPU_RAM + 0x8000 + CHR_ROM_PAGE_SIZE * i, rom.characterDataPages[i].buffer, CHR_ROM_PAGE_SIZE);
    }

    for (uint8_t i = 0; i < rom.header.numPrgPages; i++) {
        PrintInfo("  Writing prg page %d to CPU @ 0x%X", i, PRG_ROM_OFFSET + PRG_ROM_PAGE_SIZE * i);
        memcpy(CPU_RAM + PRG_ROM_OFFSET + PRG_ROM_PAGE_SIZE * i, rom.programDataPages[i].buffer, PRG_ROM_PAGE_SIZE);
//        memcpy(PRG_BANKS + PRG_ROM_PAGE_SIZE * i, rom.programDataPages[i].buffer, PRG_ROM_PAGE_SIZE);
    }

    PrintInfo("Initializing Memory Mapper #%d", rom.info.memoryMapperId);
    memoryMapperId = rom.info.memoryMapperId;
    numPrgBanks = rom.header.numPrgPages;

    switch(memoryMapperId) {
        case MEMORY_MAPPER_NROM: {
            PrintInfo("Loading last PRG ROM into CPU 0xC000");
            prgBank = rom.header.numPrgPages - 1;
        } break;

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

        case MEMORY_MAPPER_SXROM: {
            PrintInfo("Loading last PRG ROM into CPU 0xC000");
//            memcpy(CPU_RAM + 0xC000, rom.programDataPages[rom.header.numPrgPages - 1].buffer, PRG_ROM_PAGE_SIZE);
            prgBank = rom.header.numPrgPages - 1;
        } break;

        case MEMORY_MAPPER_CNROM: {
            // last bank into second CHR ROM position
            PrintInfo("Loading last CHR ROM into PPU 0x0000");
//            memcpy(PPU_RAM, rom.characterDataPages[0].buffer, CHR_ROM_PAGE_SIZE);
            chrBank = 0;
        } break;


        default: {
            PrintInfo("Unsupported memory mapper %d", memoryMapperId);
            exit(1);
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
            address = address + 0x8000;
        } break;

        case MEMORY_MAPPER_UNROM: {
            // address is unchanged
        } break;

        case MEMORY_MAPPER_SXROM: {
            if (useLargeChrBankMode) {
                // drop lower bit on chr bank 0, rounding down address to fit 8kb page
                address = address + 0x8000 + (chrBank0 & ~0x1) * CHR_ROM_PAGE_SIZE;
            } else {
                // switch two separate 4k banks
                if (address < 0x1000) {
                    // chr bank 0
                    address = address + 0x8000 + (chrBank0) * CHR_ROM_PAGE_SIZE;
                } else {
                    // chr bank 1
                    address = address - 0x1000 + 0x8000 + (chrBank1) * CHR_ROM_PAGE_SIZE;
                }
            }
        } break;

        case MEMORY_MAPPER_CNROM: {
            // switch between four 8KiB banks. capped at 32KiB of CHR.
            address = address + 0x8000 + chrBank * CHR_ROM_PAGE_SIZE;
        } break;

        default: {
            PrintInfo("Unsupported memory mapper %d", memoryMapperId);
        } break;
    }

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

        case MEMORY_MAPPER_SXROM: {
            if(address >= 0x8000 && address <= 0xFFFF) {
                // writing a single bit into the load-register

                if(value & 0x80) {
                    // a value with bit-7 set, clears shift register to default state
                    shiftRegister = 0x10;
//                    PrintInfo("Reset shift register");

                    // reset prg rom bank mode
                    control |= 0xC;
                    prgBankSwapMode = SWAP_UPPER;
                    prgBankSwapSize = SWAP_16K;
                    prgBankMode = 0x3;
                    useLargeChrBankMode = true;
                    mirroring = 0;
                } else {
                    // a value with bit-7 clear, shifts bit-0 onto shift register
                    bool commit = (shiftRegister & 0x1); // has register has been shifted 4 times?

                    shiftRegister >>= 1u;
                    shiftRegister |= (value & 0x1) << 4;
//                    PrintInfo("Wrote to shift register: %d%d%d%d%db (from 0x%02X to 0x%02X)",
//                             (shiftRegister & 0x10) >> 4, (shiftRegister & 0x08) >> 3,
//                             (shiftRegister & 0x04) >> 2, (shiftRegister & 0x02) >> 1,
//                             shiftRegister & 0x01, value, shiftRegister);

                    if(commit) {
                        switch(address) {
                            // bits 14 and 13 of address select which register to save to
                            case 0x8000 ... 0x9FFF: {
                                /**
                                 * Control register (5 bits)
                                 * 43210
                                 * -----
                                 * CPPMM
                                 * |||||
                                 * |||++- Mirroring (0: one-screen, lower bank; 1: one-screen, upper bank;
                                 * |||               2: vertical; 3: horizontal)
                                 * |++--- PRG ROM bank mode (0, 1: switch 32 KB at $8000, ignoring low bit of bank number;
                                 * |                         2: fix first bank at $8000 and switch 16 KB bank at $C000;
                                 * |                         3: fix last bank at $C000 and switch 16 KB bank at $8000)
                                 * +----- CHR ROM bank mode (0: switch 8 KB at a time; 1: switch two separate 4 KB banks)
                                 */

                                control = shiftRegister;
                                PrintInfo("MMC1: Control = 0x%02X (via 0x%04X)", shiftRegister, address);

                                prgBankSwapMode = ((control >> 2) & 1) ? SWAP_UPPER : SWAP_LOWER;
                                prgBankSwapSize = ((control >> 3) & 1) ? SWAP_16K : SWAP_32K;

                                prgBankMode = (control >> 2) & 3;
                                if(prgBankMode <= 1) {
                                    PrintInfo("MMC1: PRG ROM bank mode = switch 32KB at $8000");
                                } else if (prgBankMode == 2) {
                                    PrintInfo("MMC1: PRG ROM bank mode = fix first bank at $8000 and switch 16KB bank at $C000");
                                } else if (prgBankMode == 3) {
                                    PrintInfo("MMC1: PRG ROM bank mode = fix last bank at $C000 and switch 16KB bank at $8000");
                                }
                                useLargeChrBankMode = !((control >> 4) & 1);
                                if(useLargeChrBankMode) {
                                    PrintInfo("MMC1: CHR ROM bank mode = switch 8KB at a time");
                                } else {
                                    PrintInfo("MMC1: CHR ROM bank mode = switch two separate 4KB banks");
                                }
                                mirroring = (control & 3);
                                if(mirroring <= 1) {
                                    PrintInfo("MMC1: mirroring = one-screen");
                                } else if (mirroring == 2) {
                                    PrintInfo("MMC1: mirroring = vertical");
                                } else if (mirroring == 3) {
                                    PrintInfo("MMC1: mirroring = horizontal");
                                }
                            } break;
                            case 0xA000 ... 0xBFFF: {
                                /**
                                 * CHR Bank 1 register (5 bits)
                                 * 43210
                                 * -----
                                 * CCCCC
                                 * |||||
                                 * +++++- Select 4 KB or 8 KB CHR bank at PPU $0000 (low bit ignored in 8 KB mode)
                                 */
                                chrBank0 = shiftRegister;
                                PrintInfo("MMC1: CHR bank 0 = 0x%02X (via 0x%04X)", shiftRegister, address);
                            } break;
                            case 0xC000 ... 0xDFFF: {
                                /**
                                 * CHR Bank 1 register (5 bits)
                                 * 43210
                                 * -----
                                 * CCCCC
                                 * |||||
                                 * +++++- Select 4 KB CHR bank at PPU $1000 (ignored in 8 KB mode)
                                 */
                                chrBank1 = shiftRegister;
                                PrintInfo("MMC1: CHR bank 1 = 0x%02X (via 0x%04X)", shiftRegister, address);
                            } break;
                            case 0xE000 ... 0xFFFF: {
                                /**
                                 * PRG ROM bank register (5 bits)
                                 * 43210
                                 * -----
                                 * RPPPP
                                 * |||||
                                 * |++++- Select 16 KB PRG ROM bank (low bit ignored in 32 KB mode)
                                 * +----- PRG RAM chip enable (0: enabled; 1: disabled; ignored on MMC1A)
                                 */
                                prgBank = shiftRegister & 0xF;
                                int prgRamChip = !((shiftRegister >> 4) & 1);
//                                PrintInfo("MMC1: PRG bank = 0x%02X / PRG RAM enable = %d (via 0x%04X)", prgBank, prgRamChip, address);
                            } break;
                            default:
                                PrintError("Unexpected address: %x with value %d", address, value);
                        }

                        shiftRegister = 0x10;
                    }
                }
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
        case MEMORY_MAPPER_NROM: {
            if (address >= 0x8000 && address < 0xC000) {
                auto relative = address - 0x8000;
                auto adjusted = relative + PRG_ROM_OFFSET + 0 * PRG_ROM_PAGE_SIZE;
                auto value = CPU_RAM[adjusted];
                return value;
            } else {
                auto relative = address - 0xC000;
                auto adjusted = relative + PRG_ROM_OFFSET + (numPrgBanks-1) * PRG_ROM_PAGE_SIZE;
                auto value = CPU_RAM[adjusted];
                return value;
            }
        } break;

        case MEMORY_MAPPER_UNROM: {
            // switch between four 16KiB PRG banks for the first ROM bank
            if(address < 0xC000) {
                auto relative = address - 0x8000;
                auto adjusted = relative + PRG_ROM_OFFSET + prgBank * PRG_ROM_PAGE_SIZE;
                auto value = CPU_RAM[adjusted];
                return value;
            }
        } break;

        case MEMORY_MAPPER_SXROM: {
            if (prgBankMode <= 1) {
                // 32KiB swap size. always replaces full prg rom bank range
                int bigPrgBank = prgBank & ~0x1; // drop low bit

                PrintInfo("MMC1: PRG ROM bank mode = switch 32KB at $8000 (%d)", bigPrgBank);
                if (address >= 0x8000 && address < 0xFFFF) {
                    auto relative = address - 0x8000;
                    auto adjusted = relative + PRG_ROM_OFFSET + bigPrgBank * PRG_ROM_PAGE_SIZE * 2;
                    auto value = CPU_RAM[adjusted];
                    return value;
                }
            }
            else if (prgBankMode == 2) {
                // first bank at 0x8000 and switch 16KB bank at 0xC000
                if (address >= 0x8000 && address < 0xC000) {
                    auto relative = address - 0x8000;
                    auto adjusted = relative + PRG_ROM_OFFSET + 0 * PRG_ROM_PAGE_SIZE;
                    auto value = CPU_RAM[adjusted];
                    return value;
                } else {
                    auto relative = address - 0xC000;
                    auto adjusted = relative + PRG_ROM_OFFSET + prgBank * PRG_ROM_PAGE_SIZE;
                    auto value = CPU_RAM[adjusted];
                    return value;
                }
            }
            else if (prgBankMode == 3) {
                // last bank at 0xC000 and switch 16KB bank at 0x8000
                if (address >= 0x8000 && address < 0xC000) {
                    auto relative = address - 0x8000;
                    auto adjusted = relative + PRG_ROM_OFFSET + prgBank * PRG_ROM_PAGE_SIZE;
                    auto value = CPU_RAM[adjusted];
                    return value;
                } else { // [0xC000, 0xFFFF]
                    auto relative = address - 0xC000;
                    auto adjusted = relative + PRG_ROM_OFFSET + (numPrgBanks - 1) * PRG_ROM_PAGE_SIZE;
                    auto value = CPU_RAM[adjusted];
                    return value;
                }
            }
        } break;

        case MEMORY_MAPPER_CNROM: {
            // fixed mapper, works for either 16kb (single) or 32kb (double) prg roms
            if (address >= 0x8000 && address < 0xC000) {
                auto relative = address - 0x8000;
                auto adjusted = relative + PRG_ROM_OFFSET + 0 * PRG_ROM_PAGE_SIZE;
                auto value = CPU_RAM[adjusted];
                return value;
            } else { // [0xC000, 0xFFFF]
                auto relative = address - 0xC000;
                auto adjusted = relative + PRG_ROM_OFFSET + (numPrgBanks-1) * PRG_ROM_PAGE_SIZE;
                auto value = CPU_RAM[adjusted];
                return value;
            }
        } break;

        default: {
            PrintInfo("Unsupported memory mapper %d", memoryMapperId);
        } break;
    }

    return CPU_RAM[address];
}
