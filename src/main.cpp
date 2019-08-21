#include "Cartridge.h"
#include "CartridgeLoader.h"
#include "Logging.h"
#include "CPU.h"
#include "Backtrace.h"
#include "MemoryStack.h"
#include "GUI.h"
#include "Joypad.h"
#include "Audio.h"

#include <cstdio>
#include <thread>
#include <chrono>
#include <gperftools/profiler.h>
#include <cpucounters.h>
#include <x86intrin.h>

using namespace std::chrono_literals;
typedef std::chrono::high_resolution_clock clock_type;

void collectInputEvents(Joypad *pJoypad, bool *pBoolean);

void printLibVersions();

Cartridge loadCartridge();

static inline uint64_t rdtsc(void)
{
    uint32_t eax = 0, edx;

    __asm__ __volatile__("cpuid;"
                         "rdtsc;"
    : "+a" (eax), "=d" (edx)
    :
    : "%rcx", "%rbx", "memory");

    __asm__ __volatile__("xorl %%eax, %%eax;"
                         "cpuid;"
    :
    :
    : "%rax", "%rbx", "%rcx", "%rdx", "memory");

    return (((uint64_t)edx << 32) | eax);
}
int main(int argc, char **argv) {
    //Backtrace::install();
    printLibVersions();

    Cartridge rom = loadCartridge();

    // raster output
    auto raster = new Raster();
    // ppu
    auto ppu = new PPU(raster);
    ppu->loadRom(rom);
    // apu
    auto audio = new Audio(raster);
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

    // memory mapper
    auto mmc = new MemoryMapper(ppu->getPpuRam(), memory->getByteArray());
    ppu->useMemoryMapper(mmc);
    memory->useMemoryMapper(mmc);

    // cpu
    auto cpu = new CPU(registers, memory, stack);
    cpu->load(rom);

    // load rom into memory mapper last, as it may override PRG ROM
    mmc->loadRom(rom);

    // read PC from RESET vector
    cpu->reset();

    // pump event loop to get window to appear before emulation starts
    gui->render();

    PrintDbg("Reset program-counter to 0x%X", registers->PC);

    // https://www.pagetable.com/?p=410
    auto doVblankNMI = [&]() {
//        PrintInfo("Triggering NMI");

        // clear break flag
        registers->P.B = 0;
        stack->pushStackWord(registers->PC);
        stack->pushStackByte(registers->P.asByte());

        // disable irq
        registers->P.I = 1;
        tCPU::word address = memory->readWord(NMI_VECTOR_ADDR);

        registers->PC = address;

        cpu->addCycles(7);
    };

    auto doIRQ = [&]() {
//        PrintInfo("Triggering IRQ");

        // clear break flag
        registers->P.B = 0;
        stack->pushStackWord(registers->PC);
        stack->pushStackByte(registers->P.asByte());

        // disable irq
        registers->P.I = 1;
        tCPU::word address = memory->readWord(IRQ_BRK_VECTOR_ADDR);

        registers->PC = address;

        cpu->addCycles(7);
    };

    PrintInfo("registers->PC = 0x%X", registers->PC);

//    Loggy::Enabled = Loggy::DEBUG;
//    registers->PC = 0xC000; // needed for nestest.nes
//    registers->P.X = 1;
//    registers->P.I = 1;
    registers->S = 0xFF;

    gui->render();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {}

    std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::now();

    auto start = std::chrono::high_resolution_clock::now();
    auto last = std::chrono::high_resolution_clock::now();

//    ProfilerStart("nes.prof");
#if 0
    PCM *m = PCM::getInstance();
//    m->resetPMU();
    m->allowMultipleInstances();
    m->program(PCM::DEFAULT_EVENTS, NULL);

    if (m->program() != PCM::Success) {
        printf("Exiting due to PCM initialization failure\n");
        return -1;
    }

    int *meh = new int[10]{
            1, 2, 3, 4, 5, 6, 7, 8, 9, 0
    };

    CoreCounterState before = getCoreCounterState(0);

    auto timestamp1 = rdtsc();
    for (int a = 0; a < 1; a++) {
        for (int i = 0; i < 10; i++) {
            meh[i] = i;
        }
    }
    auto duration = rdtsc() - timestamp1;

    auto timestamp3 = rdtsc();
    CoreCounterState after = getCoreCounterState(0);
    auto overhead = rdtsc() - timestamp3;

    printf("Instructions per Clock: %.3f\n", getIPC(before, after));
    printf("CPU Cycles: %llu\n", getCycles(before, after) - overhead);
    printf("PCM Overhead: %llu\n", overhead);
    printf("CPU Cycles (rdtsc): %llu\n", duration);
    printf("L3 cache hit ratio: %.3f\n", getL3CacheHitRatio(before, after));
    printf("L2 cache hit ratio: %.3f\n", getL2CacheHitRatio(before, after));
    printf("Wasted cycles caused by L3 misses: %.3f\n", getCyclesLostDueL3CacheMisses(before, after));
//    printf("Bytes read from DRAM: %llu\n", getBytesReadFromMC(before, after));

    m->cleanup();
#endif

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

        if (!registers->P.I) {
            // if IRQ is not masked, allow APU to trigger it
            if (audio->pullIRQ()) {
                PrintInfo("Triggering IRQ from APU");
                doIRQ();
            }
        }

//        if(cpu->getCycleRuntime() > 100) {
//            alive = false;
//        }

        if (ppu->enteredVBlank()) {
            if (gui->showDebuggerPPU) {
                ppu->renderDebug();
            }
            gui->render();
            ppu->clear();
            collectInputEvents(joypad, &alive);

            // throttle execution after every screen render
            auto now = std::chrono::high_resolution_clock::now();
            auto span = now - last;
            last = now;

            // throttle to around 60fps
            const std::chrono::milliseconds &goal = 16ms;
            auto gap = std::chrono::duration_cast<std::chrono::milliseconds>(goal - span);
            if (gap > 0ms) {
                now = std::chrono::high_resolution_clock::now();
                std::this_thread::sleep_for(gap);
                auto actual_delay = std::chrono::high_resolution_clock::now() - now;
//                PrintInfo("throttling ppu: wanted=%d msec got=%d msec", gap, std::chrono::duration_cast<std::chrono::milliseconds>(actual_delay));
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
    return 0;
}

Cartridge loadCartridge() {
    CartridgeLoader loader;
    Cartridge rom = loader.loadCartridge("roms/official.nes");
//    Cartridge rom = loader.loadCartridge("roms/nmi_and_irq.nes");
//    Cartridge rom = loader.loadCartridge("roms/irq_and_dma.nes");
//    Cartridge rom = loader.loadCartridge("../roms/SuperMarioClouds.nes"); // draws zeroes instead of clouds, almost scrolls
//    Cartridge rom = loader.loadCartridge("../roms/stars.nes"); // draws tiles instead of stars
//    Cartridge rom = loader.loadCartridge("roms/scanline.nes"); // stabler
//    Cartridge rom = loader.loadCartridge("../roms/scroll.nes"); // doesn't work at all
//    Cartridge rom = loader.loadCartridge("../roms/color_test.nes"); // doesn't work at all
//    Cartridge rom = loader.loadCartridge("../roms/nestest.nes");
//    Cartridge rom = loader.loadCartridge("../roms/instr_timing/instr_timing.nes");
//    Cartridge rom = loader.loadCartridge("../roms/apu_mixer/square.nes");
//    Cartridge rom = loader.loadCartridge("roms/Karateka (J) [p1].nes");
//    Cartridge rom = loader.loadCartridge("../../roms/Defender 2 (U).nes");
//    Cartridge rom = loader.loadCartridge("../roms/all/nrom/Slalom (U).nes");

//    Cartridge rom = loader.loadCartridge("../../src/roms/sound-test/sound-test.nes");

    /////////////////////////////////////////////////
    // mapper=0 aka NROM
//    Cartridge rom = loader.loadCartridge("roms/supermariobros.nes"); // played through at least one level
//    Cartridge rom = loader.loadCartridge("../roms/donkey_kong.nes"); // draws zeroes instead of sprites
//    rom.info.memoryMapperId = 0;

    /////////////////////////////////////////////////
    // mapper=1 aka MMC1
//    Cartridge rom = loader.loadCartridge("roms/zelda.nes"); // intro works with visual glitches
//    Cartridge rom = loader.loadCartridge("roms/Mega Man 2 (U).nes"); // intro broken
//    Cartridge rom = loader.loadCartridge("roms/Teenage Mutant Ninja Turtles (U).nes"); // ??

    /////////////////////////////////////////////////
    // mapper=2 aka UNROM
// 4 or 8 banks of PRG ROMs
//    Cartridge rom = loader.loadCartridge("roms/Metal Gear (U).nes"); // irq audio bug
//    Cartridge rom = loader.loadCartridge("roms/Contra (U).nes"); // sprite rendering glitch and audio i/o 4009 bug
//    Cartridge rom = loader.loadCartridge("roms/all-roms/USA/Duck Tales (U).nes"); // freezes on intro
// Mega Man
// claims to mapper=66 (override)
// intro rendering glitch, seems to be using wrong nametable, freezes on death
//    Cartridge rom = loader.loadCartridge("../roms/all-roms/USA/Mega Man (U).nes");
//    rom.info.memoryMapperId = 2;

    /////////////////////////////////////////////////
    // mapper=3 aka CNROM
//    Cartridge rom = loader.loadCartridge("../roms/all-roms/USA/Donkey Kong Classics (U).nes"); // works but claims to be mapper=64.
//    Cartridge rom = loader.loadCartridge("roms/Arkanoid (U).nes"); // works fine
//    Cartridge rom = loader.loadCartridge("roms/Gradius (U).nes"); // works fine
//    Cartridge rom = loader.loadCartridge("../roms/all-roms/USA/Arkista's Ring (U) [!].nes"); // doesn't boot
//    Cartridge rom = loader.loadCartridge("../roms/all-roms/USA/Cybernoid - The Fighting Machine (U).nes"); // claims to be mapper=67
//    Cartridge rom = loader.loadCartridge("../roms/all-roms/USA/Bump'n'Jump (U).nes"); // doesn't boot
//    Cartridge rom = loader.loadCartridge("../roms/mapper_3_no_bus_conflict_test.nes"); // doesn't work.


    return rom;
}

void printLibVersions() {
    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    PrintInfo("Compiled against SDL v%d.%d.%d", compiled.major, compiled.minor, compiled.patch);
    PrintInfo("Linked against SDL v%d.%d.%d", linked.major, linked.minor, linked.patch);
}

// pump the event loop to ensure window visibility
// collect keyboard events and send them in as joypad events
void collectInputEvents(Joypad *joypad, bool *alive) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_KEYDOWN: {
                switch (e.key.keysym.sym) {
                    case SDLK_SPACE:
                        joypad->buttonDown(Select);
                        break;
                    case SDLK_c:
                        joypad->buttonDown(Start);
                        break;
                    case SDLK_RIGHT:
                        joypad->buttonDown(Right);
                        break;
                    case SDLK_LEFT:
                        joypad->buttonDown(Left);
                        break;
                    case SDLK_UP:
                        joypad->buttonDown(Up);
                        break;
                    case SDLK_DOWN:
                        joypad->buttonDown(Down);
                        break;
                    case SDLK_a:
                        joypad->buttonDown(A);
                        break;
                    case SDLK_b:
                        joypad->buttonDown(B);
                        break;
                    case SDLK_q:
                        *alive = false;
                        break;
                    case SDLK_d:
                        if (Loggy::Enabled == Loggy::INFO) {
                            printf("Debug output enabled\n");
                            Loggy::Enabled = Loggy::DEBUG;
                        } else {
                            printf("Debug output disabled\n");
                            Loggy::Enabled = Loggy::INFO;
                        }
                }
            }
                break;
            case SDL_KEYUP: {
                switch (e.key.keysym.sym) {
                    case SDLK_SPACE:
                        joypad->buttonUp(Select);
                        break;
                    case SDLK_c:
                        joypad->buttonUp(Start);
                        break;
                    case SDLK_RIGHT:
                        joypad->buttonUp(Right);
                        break;
                    case SDLK_LEFT:
                        joypad->buttonUp(Left);
                        break;
                    case SDLK_UP:
                        joypad->buttonUp(Up);
                        break;
                    case SDLK_DOWN:
                        joypad->buttonUp(Down);
                        break;
                    case SDLK_a:
                        joypad->buttonUp(A);
                        break;
                    case SDLK_b:
                        joypad->buttonUp(B);
                        break;
                }
            }
                break;
        }
    }
}

