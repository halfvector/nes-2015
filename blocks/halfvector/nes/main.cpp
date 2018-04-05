#include "Cartridge.h"
#include "CartridgeLoader.h"
#include "Logging.h"
#include "CPU.h"
#include "DI.h"
#include "Backtrace.h"
#include "MemoryStack.h"
#include "GUI.h"

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
    Cartridge rom = loader.loadCartridge("../roms/supermariobros.nes");

    auto onVblankNmiSet = []() {

    };

    // raster output
    auto raster = new Raster();
    // ppu
    auto ppu = new PPU(raster);
    ppu->loadRom(rom);
    // i/o port mapper
    auto mmio = new MemoryIO(ppu);
    // cpu memory
    auto memory = new Memory(mmio);
    // cpu registers
    auto registers = new Registers();
    // cpu stack
    auto stack = new Stack(memory, registers);
    // rendering
    auto gui = new GUI(raster);

    mmio->setMemory(memory);

    // cpu
    auto cpu = new CPU(registers, memory, stack);
    cpu->load(rom);
    cpu->reset();


    PrintDbg("Reset program-counter to 0x%X", registers->PC);

    // https://www.pagetable.com/?p=410
    auto doVblankNMI = [&]() {
        PrintDbg("Doing VBlank NMI (pushes to stack)");
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

    std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::now();

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 2500000; i++) {
        // grab next instruction
        tCPU::byte opCode = memory->readByteDirectly(registers->PC);

        // step cpu
        int cpuCycles = cpu->executeOpcode(opCode);

//        if(registers->PC == 0x8181) {
//            stack->dump();
//            break;
//        }

        // step ppu in sync with cpu
        ppu->execute(cpuCycles * 3);

        // super mario brothers will spin in a `jmp $8057` loop until vblank
        if (ppu->pullNMI()) {
            doVblankNMI();
        }

        if (ppu->enteredVBlank()) {
            ppu->renderDebug();
            gui->render();
        }
    }

    auto stop = clock_type::now();
    auto span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    printf("Executed %lld cycles in %1.f seconds; %.1f microseconds per op\n", cpu->getCycleRuntime(), span / 1e9, span / 1e3 / cpu->getCycleRuntime());

    delete gui;
}

