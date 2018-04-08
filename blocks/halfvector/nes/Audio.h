#ifndef NES_AUDIO_H
#define NES_AUDIO_H

#include <SDL2/SDL_audio.h>
#include "Platform.h"

struct Sweep {
    bool enabled;
    bool decrease;
    tCPU::byte shift;
    tCPU::byte period;
};

struct SquareEnvelope {
    tCPU::byte volume;
    bool sawEnvelopeDisabled;
    bool lengthCounterDisabled;
    tCPU::byte dutyCycle;
    int phase = 0;

    tCPU::word note; // 11 bit note period
    tCPU::byte duration; // 3 bit duration

    Sweep sweep;
};

class Audio {
public:
    Audio();
    void close();

    void populate(Uint8 *stream, int len);
    static void populateFuncPtr(void *data, Uint8 *stream, int len) {
        static_cast<Audio*>(data)->populate(stream, len);
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

    void execute(int cycles);

private:
    tCPU::byte channelStatus;
    tCPU::word apuCycles;

    SquareEnvelope square1;
    SquareEnvelope square2;

    void executeHalfFrame();
};


#endif