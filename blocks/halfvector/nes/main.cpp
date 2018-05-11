#include "Cartridge.h"
#include "CartridgeLoader.h"
#include "Logging.h"
#include "CPU.h"
#include "DI.h"
#include "Backtrace.h"
#include "MemoryStack.h"
#include "GUI.h"
#include "Joypad.h"
#include "Audio.h"

#include <iostream>
#include <typeinfo>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

using namespace boost;
typedef std::chrono::high_resolution_clock clock_type;


int main(int argc, char **argv) {
    Backtrace::install();

    CartridgeLoader loader;
    Cartridge rom = loader.loadCartridge("../roms/supermariobros.nes"); // intro works, cannot tile scroll
//    Cartridge rom = loader.loadCartridge("../roms/SuperMarioClouds.nes"); // draws zeroes instead of clouds, almost scrolls
//    Cartridge rom = loader.loadCartridge("../roms/stars.nes"); // draws tiles instead of stars
//    Cartridge rom = loader.loadCartridge("../roms/scanline.nes"); // stabler
//    Cartridge rom = loader.loadCartridge("../roms/scroll.nes"); // doesn't work at all
//    Cartridge rom = loader.loadCartridge("../roms/color_test.nes"); // doesn't work at all
//    Cartridge rom = loader.loadCartridge("../roms/nestest.nes");
//    Cartridge rom = loader.loadCartridge("../roms/gradius.nes"); // scrolling intro almost works
//    Cartridge rom = loader.loadCartridge("../roms/megaman1.nes");
//    Cartridge rom = loader.loadCartridge("../roms/donkey_kong.nes"); // draws zeroes instead of sprites
//    Cartridge rom = loader.loadCartridge("../roms/apu_mixer/square.nes");

//    Cartridge rom = loader.loadCartridge("../roms/Karateka (J) [p1].nes");
//    Cartridge rom = loader.loadCartridge("../roms/Defender 2 (U).nes");
//    Cartridge rom = loader.loadCartridge("../roms/all/nrom/Slalom (U).nes");


    // raster output
    auto raster = new Raster();
    // ppu
    auto ppu = new PPU(raster);
    ppu->loadRom(rom);
    // apu
    auto audio = new Audio();
    // controllers
    auto joypad = new Joypad();
    // i/o port mapper
    auto mmio = new MemoryIO(ppu, joypad, audio);
    // cpu memory
    auto memory = new Memory(mmio);
    mmio->setMemory(memory);
    // cpu registers
    auto registers = new Registers();
    // cpu stack
    auto stack = new Stack(memory, registers);
    // rendering
    auto gui = new GUI(raster);

    // cpu
    auto cpu = new CPU(registers, memory, stack);
    cpu->load(rom);
    cpu->reset(); // read PC from RESET vector

    gui->render();


    PrintDbg("Reset program-counter to 0x%X", registers->PC);

    // https://www.pagetable.com/?p=410
    auto doVblankNMI = [&]() {
//        PrintInfo("Doing VBlank NMI (pushes to stack)");
        // clear break flag
        registers->P.B = 0;
        stack->pushStackWord(registers->PC);
        stack->pushStackByte(registers->P.asByte());

        // disable irq
        registers->P.I = 1;
        tCPU::word address = memory->readWord(0xFFFA);

        registers->PC = address;

        cpu->addCycles(7);
    };

//    registers->PC = 0xC000; // needed for nestest.nes
    registers->P.X = 1;
    registers->P.I = 1;
    registers->S = 0xFD;

    gui->render();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {}

    std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::now();

    auto start = std::chrono::high_resolution_clock::now();

    bool alive = true;
    while (alive) {
        // grab next instruction
        tCPU::byte opCode = memory->readByteDirectly(registers->PC);

        // step cpu
        int cpuCycles = cpu->executeOpcode(opCode);

        // step ppu in sync with cpu
        ppu->execute(cpuCycles * 3);

        // step apu in sync with cpu
        audio->execute(cpuCycles);

        // super mario brothers will spin in a `jmp $8057` loop until vblank
        if (ppu->pullNMI()) {
            doVblankNMI();
        }

        if (ppu->enteredVBlank()) {
            ppu->renderDebug();
            gui->render();
            ppu->clear();

            // pump the event loop to ensure window visibility
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                switch (e.type) {
                    case SDL_KEYDOWN: {
                        switch (e.key.keysym.sym) {
                            case SDLK_SPACE:
                                joypad->buttonDown(JoypadButtons::Select);
                                break;
                            case SDLK_c:
                                joypad->buttonDown(JoypadButtons::Start);
                                break;
                            case SDLK_RIGHT:
                                joypad->buttonDown(JoypadButtons::Right);
                                break;
                            case SDLK_LEFT:
                                joypad->buttonDown(JoypadButtons::Left);
                                break;
                            case SDLK_UP:
                                joypad->buttonDown(JoypadButtons::Up);
                                break;
                            case SDLK_DOWN:
                                joypad->buttonDown(JoypadButtons::Down);
                                break;
                            case SDLK_a:
                                joypad->buttonDown(JoypadButtons::A);
                                break;
                            case SDLK_b:
                                joypad->buttonDown(JoypadButtons::B);
                                break;
                            case SDLK_q:
                                alive = false;
                                break;
                            case SDLK_d:
                                if(Loggy::Enabled == Loggy::INFO) {
                                    printf("Debug output enabled\n");
                                    Loggy::Enabled = Loggy::DEBUG;
                                } else {
                                    printf("Debug output disabled\n");
                                    Loggy::Enabled = Loggy::INFO;
                                }
                        }
                    } break;
                    case SDL_KEYUP: {
                        switch (e.key.keysym.sym) {
                            case SDLK_SPACE:
                                joypad->buttonUp(JoypadButtons::Select);
                                break;
                            case SDLK_c:
                                joypad->buttonUp(JoypadButtons::Start);
                                break;
                            case SDLK_RIGHT:
                                joypad->buttonUp(JoypadButtons::Right);
                                break;
                            case SDLK_LEFT:
                                joypad->buttonUp(JoypadButtons::Left);
                                break;
                            case SDLK_UP:
                                joypad->buttonUp(JoypadButtons::Up);
                                break;
                            case SDLK_DOWN:
                                joypad->buttonUp(JoypadButtons::Down);
                                break;
                            case SDLK_a:
                                joypad->buttonUp(JoypadButtons::A);
                                break;
                            case SDLK_b:
                                joypad->buttonUp(JoypadButtons::B);
                                break;
                        }
                    } break;
                }
            }
        }
    }

    auto stop = clock_type::now();
    auto span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    auto freq = cpu->getCycleRuntime() / (span / 1e3);
    printf("Executed %lld cycles in %1.f seconds; %.1f microseconds per op; %.3f mhz (real 1.789 mhz)\n",
           cpu->getCycleRuntime(), span / 1e9,
           span / 1e3 / cpu->getCycleRuntime(), freq);

    delete gui;

    audio->close();
}

