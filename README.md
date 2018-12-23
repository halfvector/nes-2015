# Nintendo Emulator

## Current Status
* Realtime CPU, PPU, and ALU emulation

### CPU
* Load iNES cartridge, parse headers, CHR, and PRG roms
* Enough features to play Super Mario Bros, Donkey Kong, Arkanoid, Gradius, and Metal Gear
* All memory addressing modes. All standard and some obscure opcodes.
* Decent cycle counting, including iffy page-boundary-crossing penalty calculator
* Memory Mapped IO with most PPU and some ALU addresses.
* Very nice disassembler output

### PPU
* Vertical and Horizontal scrolling, Sprite DMA, Address Mirroring
* Per-pixel scanline advancement
* Sprite-0 hit detection, HBlank/VBlank support

### APU
* Basic implementation: two square waves, enough to play SMB intro music.

### Supported Memory Mappers
* NROM (iNES Mapper 0)
* UNROM (iNES Mapper 2)
* CNROM (iNES Mapper 3)

### Input
* Basic joystick support

## Dependency management using `conan`
1. Add repo: ``conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan``
2. Download dependencies: `./conan-resolve-deps.sh`
3. CLion/CMake will pickup dependencies automatically during build step

## All 2015 goals were met!
Target Platform: Ported from Windows to OS X
Primary Goals: CPU & PPU Performance, using C++14 features, scanline-accurate CPU<->PPU synchronization
Stretch Goals: APU emulation, memory-mapper supports, pixel-accurate synchronization

Compatibility Goals:
* Primary: Super Mario Bros & Donkey Kong
* Secondary: other 32kB ROM + 8kB VROM games
* Stretch: Official Nintendo memory mapper powered games