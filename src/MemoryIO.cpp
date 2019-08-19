#include "MemoryIO.h"
#include "Exceptions.h"
#include "Joypad.h"
#include <functional>

template<>
struct MemoryIOHandler<0x2002> {
    static tCPU::byte read(PPU *ppu) {
        tCPU::byte value = ppu->getStatusRegister();
        PrintMemoryIO("Reading port 0x%04X; status register = 0x%02X", 0x2002, (int) value);
        return value;
    }
};

template<>
struct MemoryIOHandler<0x2000> {
    static tCPU::byte read(PPU *ppu) {
        tCPU::byte value = ppu->getControlRegister1();
        PrintMemoryIO("Reading port 0x%04X; status register = 0x%02X", 0x2000, (int) value);
        return value;
    }

    static void write(PPU *ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2000 - PPU Control Register 1", (int) value);
        ppu->setControlRegister1(value);
    }
};

template<>
struct MemoryIOHandler<0x2001> {
    static void write(PPU *ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2001 - PPU Control Register 2", (int) value);
        ppu->setControlRegister2(value);
    }
};

template<>
struct MemoryIOHandler<0x2003> {
    static void write(PPU *ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2003 - Sprite RAM Address", (int) value);
        ppu->setSprRamAddress(value);
    }
};

template<>
struct MemoryIOHandler<0x2004> {
    static tCPU::byte read(PPU *ppu) {
        tCPU::byte value = ppu->readSpriteMemory();
        PrintMemoryIO("Read 0x%02X from port $2004 - Sprite RAM I/O Register", (int) value);
        return value;
    }

    static void write(PPU *ppu, tCPU::byte value) {
        PrintMemoryIO("Write 0x%02X to port $2004 - Sprite RAM I/O Register", (int) value);
        ppu->writeSpriteMemory(value);
    }
};

template<>
struct MemoryIOHandler<0x2005> {
    static void write(PPU *ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2005 - VRAM Address Register 1", (int) value);
        ppu->setVRamAddressRegister1(value);
    }
};

template<>
struct MemoryIOHandler<0x2006> {
    static void write(PPU *ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2006 - VRAM Address Register 2", (int) value);
        ppu->setVRamAddressRegister2(value);
    }
};

template<>
struct MemoryIOHandler<0x2007> {
    static tCPU::byte read(PPU *ppu) {
        tCPU::byte value = ppu->readFromVRam();
        PrintMemoryIO("Read 0x%02X from port $2007 - VRAM I/O Register", (int) value);
        return value;
    }

    static void write(PPU *ppu, tCPU::byte value) {
        PrintMemoryIO("Writing 0x%02X to port $2007 - VRAM I/O Register", (int) value);
        ppu->writeToVRam(value);
    }
};

// Audio - Square 1

template<>
struct MemoryIOHandler<0x4000> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4000 - APU - Square 1 - Envelope Generator (Tone and Volume)", (int) value);
        apu->setSquare1Envelope(value);
    }
};

template<>
struct MemoryIOHandler<0x4001> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4001 - APU - Square 1 - Sweep", (int) value);
        apu->setSquare1Sweep(value);
    }
};

template<>
struct MemoryIOHandler<0x4002> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4002 - APU - Square 1 - Period (low bits)", (int) value);
        apu->setSquare1NoteLow(value);
    }
};

template<>
struct MemoryIOHandler<0x4003> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4003 - APU - Square 1 - Duration and Period (high bits)", (int) value);
        apu->setSquare1NoteHigh(value);
    }
};

// Audio - Square 2

template<>
struct MemoryIOHandler<0x4004> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4004 - APU - Square 2 - Envelope Generator (Tone and Volume)", (int) value);
        apu->setSquare2Envelope(value);
    }
};

template<>
struct MemoryIOHandler<0x4005> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4005 - APU - Square 2 - Sweep", (int) value);
        apu->setSquare2Sweep(value);
    }
};

template<>
struct MemoryIOHandler<0x4006> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4006 - APU - Square 2 - Period (low bits)", (int) value);
        apu->setSquare2NoteLow(value);
    }
};

template<>
struct MemoryIOHandler<0x4007> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4007 - APU - Square 2 - Duration and Period (high bits)", (int) value);
        apu->setSquare2NoteHigh(value);
    }
};

// Audio - Triangle
template<>
struct MemoryIOHandler<0x4008> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4008 - APU - Triangle - Length Counter Halt and Linear Counter", (int) value);
        apu->setTriangleDuration(value);
    }
};

template<>
struct MemoryIOHandler<0x4009> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4008 - APU - Triangle - Unused", (int) value);
    }
};

template<>
struct MemoryIOHandler<0x400A> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4008 - APU - Triangle - Period (low bits)", (int) value);
        apu->setTrianglePeriodLow(value);
    }
};

template<>
struct MemoryIOHandler<0x400B> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4008 - APU - Triangle - Length Counter Reload and Period (high bits)", (int) value);
        apu->setTrianglePeriodHigh(value);
    }
};

// Audio - Noise
template<>
struct MemoryIOHandler<0x400C> {
    static void write(Audio *apu, tCPU::byte value) {
        char buffer[9];
        SDL_itoa(value, buffer, 2);
        PrintDbg("Writing 0x%02X (%08s) to port $400C - APU - Noise - Length Counter Halt and Volume", (int) value, buffer);
        apu->setNoiseEnvelope(value);
    }
};

template<>
struct MemoryIOHandler<0x400D> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $400C - APU - Noise - Unused", (int) value);
    }
};

template<>
struct MemoryIOHandler<0x400E> {
    static void write(Audio *apu, tCPU::byte value) {
        char buffer[9];
        SDL_itoa(value, buffer, 2);
        PrintDbg("Writing 0x%02X (%08s) to port $400E - APU - Noise - Loop Enable and Noise Period", (int) value, buffer);
        apu->setNoisePeriod(value);
    }
};

template<>
struct MemoryIOHandler<0x400F> {
    static void write(Audio *apu, tCPU::byte value) {
        char buffer[9];
        SDL_itoa(value, buffer, 2);
        PrintDbg("Writing 0x%02X (%08s) to port $400F - APU - Noise - Length Counter Reload", (int) value, buffer);
        apu->setNoiseLength(value);
    }
};

// Audio - DMC

template<>
struct MemoryIOHandler<0x4010> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4010 - APU - DMC - Set Flags and Rate", (int) value);
        apu->writeFlagsAndRate(value);
    }
};

template<>
struct MemoryIOHandler<0x4011> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4011 - APU - DMC - Direct write to DAC", (int) value);
        apu->writeDAC(value);
    }
};

template<>
struct MemoryIOHandler<0x4012> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4012 - APU - DMC - Set Sample Address", (int) value);
        apu->setSampleAddress(value);
    }
};

template<>
struct MemoryIOHandler<0x4013> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4013 - APU - DMC - Set Sample Length", (int) value);
        apu->setSampleLength(value);
    }
};

template<>
struct MemoryIOHandler<0x4015> {
    static tCPU::byte read(Audio *apu) {
        tCPU::byte value = apu->getChannelStatus();
        PrintDbg("Read 0x%02X from port $4015 - APU - Channel Control", (int) value);
        return value;
    }

    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4015 - APU - Channel Control", (int) value);
        apu->setChannelStatus(value);
    }
};

template<>
struct MemoryIOHandler<0x4014> {
    static void write(PPU *ppu, Memory *memory, tCPU::byte value) {
        PrintPpu("Writing 0x%02X to port $2006 - VRAM Sprite DMA Xfer", (int) value);
        ppu->StartSpriteXferDMA(memory, value);

        MemoryIO::cpuCyclesPenalty += 513;
    }
};

template<>
struct MemoryIOHandler<0x4016> {
    static tCPU::byte read(Joypad *joypad) {
        tCPU::byte value = joypad->getStatePlayerOne();
        PrintMemoryIO("Read 0x%02X from port 4016 - Joypad 1 status", (int) value);
        return value;
    }

    static void write(Joypad *joypad, tCPU::byte value) {
        joypad->setStrobe(value);
//        PrintMemoryIO("Writing 0x%02X to port 4016 - Configuring Joypad", (int) value);
    }
};

template<>
struct MemoryIOHandler<0x4017> {
    static void write(Audio *apu, tCPU::byte value) {
        PrintDbg("Writing 0x%02X to port $4017 - Configuring APU Frame Counter control", (int) value);
        apu->configureFrameSequencer(value);
    }
};

MemoryIO::MemoryIO(PPU *ppu, Joypad *joypad, Audio *apu)
        : ppu(ppu), joypad(joypad), apu(apu) {
}

class MemoryIOPortException : public ExceptionBase<MemoryIOPortException> {
public:
    MemoryIOPortException(const char *str)
            : ExceptionBase(str) {}
};

int MemoryIO::cpuCyclesPenalty = 0;

bool
MemoryIO::write(tCPU::word address, tCPU::byte value) {
    switch (address) {
        case 0x2000:
            MemoryIOHandler<0x2000>::write(ppu, value);
            break;

        case 0x2001:
            MemoryIOHandler<0x2001>::write(ppu, value);
            break;

        case 0x2002:
            PrintDbg("Writing to 0x2002 is not supported. Ignoring.");
            break;

        case 0x2003:
            MemoryIOHandler<0x2003>::write(ppu, value);
            break;

        case 0x2004:
            MemoryIOHandler<0x2004>::write(ppu, value);
            break;

        case 0x2005:
            MemoryIOHandler<0x2005>::write(ppu, value);
            break;

        case 0x2006:
            MemoryIOHandler<0x2006>::write(ppu, value);
            break;

        case 0x2007:
            MemoryIOHandler<0x2007>::write(ppu, value);
            break;

            // Audio - square waveform channel 1
        case 0x4000:
            MemoryIOHandler<0x4000>::write(apu, value);
            break;

        case 0x4001:
            MemoryIOHandler<0x4001>::write(apu, value);
            break;

        case 0x4002:
            MemoryIOHandler<0x4002>::write(apu, value);
            break;

        case 0x4003:
            MemoryIOHandler<0x4003>::write(apu, value);
            break;

            // Audio - square waveform channel 2
        case 0x4004:
            MemoryIOHandler<0x4004>::write(apu, value);
            break;

        case 0x4005:
            MemoryIOHandler<0x4005>::write(apu, value);
            break;

        case 0x4006:
            MemoryIOHandler<0x4006>::write(apu, value);
            break;

        case 0x4007:
            MemoryIOHandler<0x4007>::write(apu, value);
            break;

            // Audio - square waveform channel 2
        case 0x4008:
            MemoryIOHandler<0x4008>::write(apu, value);
            break;

        case 0x4009:
            MemoryIOHandler<0x4009>::write(apu, value);
            break;

        case 0x400A:
            MemoryIOHandler<0x400A>::write(apu, value);
            break;

        case 0x400B:
            MemoryIOHandler<0x400B>::write(apu, value);
            break;

        case 0x400C:
            MemoryIOHandler<0x400C>::write(apu, value);
            break;

        case 0x400D:
            MemoryIOHandler<0x400D>::write(apu, value);
            break;

        case 0x400E:
            MemoryIOHandler<0x400E>::write(apu, value);
            break;

        case 0x400F:
            MemoryIOHandler<0x400F>::write(apu, value);
            break;

        case 0x4010:
            MemoryIOHandler<0x4010>::write(apu, value);
            break;
        case 0x4011:
            MemoryIOHandler<0x4011>::write(apu, value);
            break;
        case 0x4012:
            MemoryIOHandler<0x4012>::write(apu, value);
            break;
        case 0x4013:
            MemoryIOHandler<0x4013>::write(apu, value);
            break;

        case 0x4014:
            MemoryIOHandler<0x4014>::write(ppu, memory, value);
            break;

        case 0x4015:
            MemoryIOHandler<0x4015>::write(apu, value);
            break;

        case 0x4016:
//            PrintUnimplementedIO("Skipping Unimplemented I/O Port: $4016 - Joypad 1");
            MemoryIOHandler<0x4016>::write(joypad, value);
            break;

        case 0x4017:
            // when written, frame counter control
            MemoryIOHandler<0x4017>::write(apu, value);
            break;

        default:
            MemoryIOPortException::emit("MemoryIO::write(); address = 0x%04X not supported", address);
    }

    return true;
}

tCPU::byte
MemoryIO::read(tCPU::word address) {
    switch (address) {
        case 0x2000:
            return MemoryIOHandler<0x2000>::read(ppu);

        case 0x2002:
            return MemoryIOHandler<0x2002>::read(ppu);

        case 0x2004:
            return MemoryIOHandler<0x2004>::read(ppu);

        case 0x2007:
            return MemoryIOHandler<0x2007>::read(ppu);

        case 0x4015:
            return MemoryIOHandler<0x4015>::read(apu);

        case 0x4016:
            return MemoryIOHandler<0x4016>::read(joypad);

        case 0x4017:
//            PrintUnimplementedIO("Skipping Unimplemented I/O Port: $4017 - Joypad 2");
            break;

        default:
            MemoryIOPortException::emit("MemoryIO::read(); address = 0x%04X not supported", address);
    }

//    auto reader = ioPortReaders.find(address);
//    if(reader != ioPortReaders.end()) {
//        return reader->second();
//    }
    return 0;
}

void
MemoryIO::setMemory(Memory *memory) {
    this->memory = memory;
}
