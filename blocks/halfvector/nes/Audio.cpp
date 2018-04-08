
#include "Audio.h"
#include "Logging.h"
#include "Registers.h"

/**
 * References:
 * https://nesdoug.com/2015/12/02/14-intro-to-sound/
 * https://wiki.nesdev.com/w/index.php/APU_Envelope
 * http://www.fceux.com/web/help/fceux.html?NESSound.html
 * https://nesdoug.com/2015/12/02/14-intro-to-sound/
 * http://nintendoage.com/forum/messageview.cfm?catid=22&threadid=22484
 * https://wiki.nesdev.com/w/index.php/Nerdy_Nights_sound
 */

/* These are in charge of maintaining our sine function */
float sinPos;
float sinStep;

/* These are the audio card settings */
#define FREQ 44100
#define SAMPLES 1024

/* This is basically an arbitrary number */
#define VOLUME 127.0

void init() {
//    square_table [n] = 95.52 / (8128.0 / n + 100)

}

static bool DutyCycle[4][8] = {
        { 1, 0, 0, 0, 0, 0, 0, 0 }, // 0, 12.5%
        { 1, 1, 0, 0, 0, 0, 0, 0 }, // 1, 25%
        { 1, 1, 1, 1, 0, 0, 0, 0 }, // 2, 50%
        { 1, 1, 1, 1, 1, 1, 0, 0 }  // 3. 75%
};

const double cpuFrequency = 1789773;

/**
 * Duty cycle generator
 * 0 = 1/8 on
 * 1 = 2/8 on
 * 2 = 4/8 on
 * 3 = 6/8 on
 */
void
Audio::populate(Uint8 *stream, int len) {
    for (int i = 0; i < len; i++) {
//        f = CPU / (16 * (t + 1))
//        t = (CPU / (16 * f)) - 1
        int total = 0;
        int step = square1.phase / 8192;
        int value = DutyCycle[square1.dutyCycle][step] ? 2 * square1.volume : 0;
        square1.phase += cpuFrequency / (16 * (square1.note+1));
        square1.phase %= 65536;
        total = value;

        step = square2.phase / 8192;
        value = DutyCycle[square2.dutyCycle][step] ? 2 * square2.volume : 0;
        square2.phase += cpuFrequency / (16 * (square2.note+1));
        square2.phase %= 65536;
        total += value;

        stream[i] = total;
    }
}

Audio::Audio() {
    /* This will hold our data */
    SDL_AudioSpec spec;
    /* This will hold the requested frequency */
    long reqFreq = 440;
    /* This is the duration to hold the note for */
    int duration = 1;

    /* Set up the requested settings */
    spec.freq = FREQ;
    spec.format = AUDIO_U8;
    spec.channels = 1;
    spec.samples = SAMPLES;
    spec.callback = populateFuncPtr;
    spec.userdata = this;

    /* Open the audio channel */
    if (SDL_OpenAudio(&spec, NULL) < 0) {
        /* FAIL! */
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        exit(1);
    }

    /* Initialize the position of our sine wave */
    sinPos = 0;
    /* Calculate the step of our sin wave */
    sinStep = 2 * M_PI * reqFreq / FREQ;

//    SDL_PauseAudio(0);
}

void
Audio::close() {
    /* Then turn it off again */
    SDL_PauseAudio(1);

    /* Close audio channel */
    SDL_CloseAudio();
}

void
Audio::setChannelStatus(tCPU::byte status) {
    this->channelStatus = status;

    bool square1 = status & 1;
    bool square2 = status & 2;
    bool triangle = status & 4;
    bool noise = status & 8;
    bool dmc = status & 16;

    PrintApu("Set channels: Square 1 = %d / Square 2 = %d / Triangle = %d / Noise = %d / DMC = %d",
             square1, square2, triangle, noise, dmc);

    if(square1) {
        SDL_PauseAudio(0);
    } else {
        SDL_PauseAudio(1);
    }
}

tCPU::byte
Audio::getChannelStatus() {
    return this->channelStatus;
}

void
Audio::writeDAC(tCPU::byte value) {
    PrintApu("DAC received byte %2X", value);
}

/**
 * Volume controls the channel's volume.  It's 4 bits long so it can have a value from 0-F.  A volume of 0 silences the channel.  1 is very quiet and F is loud.
 *
 * Duty Cycle controls the tone of the Square channel.  It's 2 bits long, so there are four possible values:
 *
 * 00 = a weak, grainy tone.  Think of the engine sounds in RC Pro-Am. (12.5% Duty)
 * 01 = a solid mid-strength tone. (25% Duty)
 * 10 = a strong, full tone, like a clarinet or a lead guitar (50% Duty)
 * 11 = sounds a lot like 01 (25% Duty negated)
 */

void
Audio::setSquare1Envelope(tCPU::byte value) {
    this->square1.volume = value & 0x0f;
    this->square1.sawEnvelopeDisabled = value & (1 << 4);
    this->square1.lengthCounterDisabled = value & (1 << 5);
    this->square1.dutyCycle = (value & 0xc0) >> 6;

    PrintDbg("  volume = %d / saw-disabled = %d / length-disabled = %d / duty = %d",
              this->square1.volume, this->square1.sawEnvelopeDisabled,
              this->square1.lengthCounterDisabled, this->square1.dutyCycle);
}

void Audio::setSquare1NoteHigh(tCPU::byte value) {
    this->square1.note &= 0x00ff; // clear upper bits
    this->square1.note |= (value & 0x7) << 8; // OR upper 3 bits
    this->square1.duration = (value & 0xf8) >> 3; // upper 5 bits

    PrintInfo("  note (w/ high bits) = %d / duration = %d", this->square1.note, this->square1.duration);
}

void Audio::setSquare1NoteLow(tCPU::byte value) {
    this->square1.note &= 0xff00; // clear lower 8 bits
    this->square1.note |= value; // OR lower 8 bits
    PrintInfo("  note (low bits) = %d", this->square1.note);
}

void Audio::setSquare1Sweep(tCPU::byte value) {
    this->square1.sweep = value;

    bool enabled = value & (1 << 8);
    bool increase = value & (1 << 3);
    bool shift = value & 0x7;

    PrintDbg("  sweep enabled = %d, increase = %d, shift = %d", enabled, increase, shift);
}

void
Audio::setSquare2Envelope(tCPU::byte value) {
    this->square2.volume = value & 0x0f;
    this->square2.sawEnvelopeDisabled = value & (1 << 4);
    this->square2.lengthCounterDisabled = value & (1 << 5);
    this->square2.dutyCycle = (value & 0xc0) >> 6;

    PrintDbg("  volume = %d / saw-disabled = %d / length-disabled = %d / duty = %d",
              this->square2.volume, this->square2.sawEnvelopeDisabled,
              this->square2.lengthCounterDisabled, this->square2.dutyCycle);
}

void Audio::setSquare2NoteHigh(tCPU::byte value) {
    this->square2.note &= 0x00ff; // clear upper bits
    this->square2.note |= (value & 0x7) << 8; // OR upper 3 bits
    this->square2.duration = (value & 0xf8) >> 3; // upper 5 bits

    PrintInfo("  note (w/ high bits) = %d / duration = %d", this->square2.note, this->square2.duration);
}

void Audio::setSquare2NoteLow(tCPU::byte value) {
    this->square2.note &= 0xff00; // clear lower 8 bits
    this->square2.note |= value; // OR lower 8 bits
    PrintInfo("  note (low bits) = %d", this->square2.note);
}

void Audio::setSquare2Sweep(tCPU::byte value) {
    this->square2.sweep = value;

    bool enabled = value & (1 << 8);
    bool increase = value & (1 << 3);
    bool shift = value & 0x7;

    PrintDbg("  sweep enabled = %d, increase = %d, shift = %d", enabled, increase, shift);
}


