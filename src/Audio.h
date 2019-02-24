#ifndef NES_AUDIO_H
#define NES_AUDIO_H

#include <SDL2/SDL_audio.h>
#include <fftw3.h>
#include "Platform.h"
#include "PPU.h"

struct Sweep {
    bool enabled;
    bool decrease;
    tCPU::byte shift;
    tCPU::byte period;
};

struct SquareEnvelope {
    bool enabled = false;
    tCPU::byte volume;
    bool sawEnvelopeDisabled;
    bool lengthCounterDisabled;
    tCPU::byte dutyCycle;
    tCPU::byte dutyStep = 0;
    int phase = 0;

    int timerValue = 0;

    tCPU::word timerPeriod; // 11 bit note period
    tCPU::word timerPeriodReloader; // actual value to use
    tCPU::byte lengthCounterLoad; // 5 bit waveform duration until silence

    Sweep sweep;

    tCPU::byte *buffer;
    int bufferIdx = 0;
    int bufferStart = 0;
};

enum CounterMode {
    LENGTH_COUNTER, LINEAR_COUNTER
};

enum FrameCounterMode {
    FOUR_STEP, FIVE_STEP
};

struct TriangleEnvelope {
    bool enabled = false;
    CounterMode counterMode = LENGTH_COUNTER;
    tCPU::byte linearCounterLoad = 0; // 7-bit linear counter
    tCPU::byte lengthCounter = 0; // 5-bit linear counter
    tCPU::byte linearCounter = 0;
    tCPU::word timerPeriod = 0;
    tCPU::word timerPeriodReloader = 0; // 11 bit note period
    tCPU::word timerValue = 0;

    bool controlFlagEnabled = false;
    bool linearCounterReloadEnabled = false;

    int phase = 0;
    int dutyStep = 0;
};

struct ChannelDebug {
    double *samples, *fft;
    int currentIdx, fftSize;
    fftw_plan plan;
    void initialize(int numSamples);
    bool put(double sample);
    void compute(tCPU::byte *fft, tCPU::byte *waveform);
};

class Audio {
public:
    Audio(Raster *pRaster);

    void close();

    void populate(Uint8 *stream, int len);

    static void populateFuncPtr(void *data, Uint8 *stream, int len) {
        static_cast<Audio *>(data)->populate(stream, len);
    }

    void configureFrameSequencer(tCPU::byte value);

    void setChannelStatus(tCPU::byte status);

    tCPU::byte getChannelStatus();

    void writeDAC(tCPU::byte value);

    void setSquare1Envelope(tCPU::byte value);

    void setSquare1NoteHigh(tCPU::byte value);

    void setSquare1NoteLow(tCPU::byte value);

    void setSquare1Sweep(tCPU::byte value);

    void setSquare2Envelope(tCPU::byte value);

    void setSquare2NoteHigh(tCPU::byte value);

    void setSquare2NoteLow(tCPU::byte value);

    void setSquare2Sweep(tCPU::byte value);

    void execute(int cpuCycles);

    void setTriangleDuration(tCPU::byte value);

    void setTrianglePeriodHigh(tCPU::byte value);

    void setTrianglePeriodLow(tCPU::byte value);

private:
    tCPU::byte channelStatus;
    tCPU::word apuCycles = 0;
    tCPU::word apuSampleCycleCounter = 0;
    tCPU::byte *buffer;
    int bufferReadIdx = 0, bufferWriteIdx = 0, bufferAvailable = 0;
    int bufferSize = 8192 * 8;

    double sampleWaveTime = 0;

    FrameCounterMode frameCounterMode = FOUR_STEP;
    SquareEnvelope square1;
    SquareEnvelope square2;
    TriangleEnvelope triangle;

    void executeHalfFrame();

    void executeQuarterFrame();

    bool issueIRQ = false;

    // debugging
    ChannelDebug square1Debug, square2Debug, triangleDebug;
    Raster *raster;
};


#endif