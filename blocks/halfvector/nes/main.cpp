#include "Cartridge.h"
#include "CartridgeLoader.h"
#include "Logging.h"
#include "CPU.h"
#include "DI.h"
#include "Backtrace.h"
#include "MemoryStack.h"
#include "GUI.h"

#define __USE_GNU
#define _GNU_SOURCE 1

#include <iostream>
#include <typeinfo>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>
#include <unistd.h>

using namespace boost;

auto f = [](auto type){ std::cout << typeid(decltype(type)).name() << std::endl; };

//class config : public boost::di::config {
//public:
//    auto policies() const noexcept {
//        return di::make_policies(f);
//    }
//};

//auto
//NesModule::getInjector() {
//    // ppu
//    PPU ppu;
//    // i/o port mapper
//    MemoryIO* mmio = new MemoryIO(&ppu);
//    // cpu memory
//    Memory* memory = new Memory(mmio);
//    // ppu memory
//    tCPU::byte ppuMemory[0x4000];
//    // cpu registers
//    Registers* registers = new Registers;
//
//    // setup injectable instances
//    auto injector = di::make_injector(
//            di::bind<Memory *>.to(memory),
//            di::bind<MemoryIO *>.to(mmio),
//            di::bind<Registers *>.to(registers)
//    );
//
//    return injector;
//}

void onTerminate() {;
}

void onUnhandledException() {
}

int main(int argc, char ** argv) {
//    std::set_terminate(&onTerminate);
//    std::set_unexpected(&onUnhandledException);

    Backtrace::install();

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

//    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format,
//            "\033[0;33m%level\033[1;30m |\033[1;30m %msg \033[1;30m @ %fbase:%line");
//    defaultConf.set(el::Level::Info, el::ConfigurationType::Format,
//            "\033[0;33m%level\033[1;30m |\033[1;37m %msg \033[1;30m @ %fbase:%line");
//    defaultConf.set(el::Level::Error, el::ConfigurationType::Format,
//            "\033[0;31m%level\033[1;30m |\033[0;31m %msg \033[1;30m @ %fbase:%line");
//    defaultConf.set(el::Level::Warning, el::ConfigurationType::Format,
//            "\033[0;31m%level\033[1;30m |\033[0;31m %msg \033[1;30m @ %fbase:%line");

    defaultConf.set(el::Level::Debug, el::ConfigurationType::Format, "%level | %msg");
    defaultConf.set(el::Level::Info, el::ConfigurationType::Format, "%level | %msg");
    defaultConf.set(el::Level::Error, el::ConfigurationType::Format, "%level | %msg");
    defaultConf.set(el::Level::Warning, el::ConfigurationType::Format, "%level | %msg");

    el::Loggers::reconfigureLogger("default", defaultConf);

    TIMED_FUNC(root);

    CartridgeLoader loader;
    Cartridge rom = loader.loadCartridge("../roms/supermariobros.nes");

    auto onVblankNmiSet = []() {

    };

    // raster output
    Raster* raster = new Raster();
    // ppu
    PPU* ppu = new PPU(raster);
    // i/o port mapper
    MemoryIO* mmio = new MemoryIO(ppu);
    // cpu memory
    Memory* memory = new Memory(mmio);
    // cpu registers
    Registers* registers = new Registers();
    // cpu stack
    Stack* stack = new Stack(memory, registers);
    // rendering
    GUI* gui = new GUI(raster->screenBuffer);

    mmio->setMemory(memory);

    // setup injectable instances
//    auto injector = di::make_injector(
//        di::bind<Stack*>.to(stack),
//        di::bind<Memory*>.to(memory),
//        di::bind<MemoryIO*>.to(mmio),
//        di::bind<Registers*>.to(registers)
//    );

    // cpu
//    auto cpu = injector.create<std::shared_ptr<CPU>>();
    CPU* cpu = new CPU(registers, memory, stack);
    cpu->load(rom);
    cpu->reset();

    PrintDbg("Reset program-counter to 0x%X") % registers->PC;

    auto doVblankNMI = [&]() {
        PrintDbg("Doing VBlank NMI (pushes to stack)");
        stack->pushStackWord(registers->PC);
        stack->pushStackWord(registers->P.asByte());

        // disable irq
        registers->P.I = 1;
        tCPU::word address = memory->readWord(0xFFFA);

        registers->PC = address;

        cpu->addCycles(7);
    };

    for(int i = 0; i < 60000; i ++) {
        // grab next instruction
        tCPU::byte opCode = memory->readByteDirectly(registers->PC);

        // step cpu
        int cpuCycles = cpu->executeOpcode(opCode);

        // step ppu in sync with cpu
        ppu->execute(cpuCycles * 3);

        if(ppu->pullNMI()) {
            doVblankNMI();
        }

        if(ppu->enteredVBlank()) {
            gui->render();
        }
    }

    SDL_Delay(2000);

    PrintDbg("Ran for %d cycles") % (int) cpu->getCycleRuntime();

    delete gui;
}

