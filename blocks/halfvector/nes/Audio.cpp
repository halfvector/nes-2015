
#include "Audio.h"
#include "Logging.h"

/**
 * References:
 * https://nesdoug.com/2015/12/02/14-intro-to-sound/
 * https://wiki.nesdev.com/w/index.php/APU_Envelope
 * http://www.fceux.com/web/help/fceux.html?NESSound.html
 * https://nesdoug.com/2015/12/02/14-intro-to-sound/
 * http://nintendoage.com/forum/messageview.cfm?catid=22&threadid=22484
 * https://wiki.nesdev.com/w/index.php/Nerdy_Nights_sound
 * https://wiki.nesdev.com/w/index.php/APU_Pulse
 * https://wiki.nesdev.com/w/index.php/APU_Frame_Counter
 */

static bool dutyCycleSequence[4][8] = {
        {1, 0, 0, 0, 0, 0, 0, 0}, // 0, 12.5%
        {1, 1, 0, 0, 0, 0, 0, 0}, // 1, 25%
        {1, 1, 1, 1, 0, 0, 0, 0}, // 2, 50%
        {1, 1, 1, 1, 1, 1, 0, 0}  // 3. 75%
};

const double cpuFrequency = 1789773;

void
Audio::populate(Uint8 *stream, int len) {
    for (int i = 0; i < len; i++) {
        int total = 0;
        int step = square1.phase / 8192;
        int value = dutyCycleSequence[square1.dutyCycle][step] ? 2 * square1.volume : 0;
        square1.phase += cpuFrequency / (16 * (square1.note + 1));
        square1.phase %= 65536;
        total = value;

        step = square2.phase / 8192;
        value = dutyCycleSequence[square2.dutyCycle][step] ? 2 * square2.volume : 0;
        square2.phase += cpuFrequency / (16 * (square2.note + 1));
        square2.phase %= 65536;
        total += value;

        stream[i] = total;
    }
}

Audio::Audio() {
    // open a single audio channel with unsigned 8-bit samples
    // 44.1 khz and 1024 sample buffers
    // callback is fired 43 times a second
    SDL_AudioSpec spec = {
            .freq = 44100,
            .format = AUDIO_U8,
            .samples = 1024,
            .channels = 1,

            // functor
            .callback = populateFuncPtr,
            .userdata = this,
    };

    if (SDL_OpenAudio(&spec, NULL) < 0) {
        PrintError("Failed to open audio: %s", SDL_GetError());
        exit(1);
    }
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

    if (square1) {
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
    this->square1.sweep.enabled = value & (1 << 7);
    this->square1.sweep.decrease = value & (1 << 3); // decrease or increase the wavelength
    this->square1.sweep.shift = value & 0x7; // right shift amount
    this->square1.sweep.period = (value >> 4) & 0x7; // update rate

    tCPU::byte refreshRateFreq = 120 / (this->square1.sweep.period + 1);

    auto frequency = cpuFrequency / (16 * (this->square1.note + 1));

    if (this->square1.sweep.enabled) {
        PrintInfo("  sweep enabled = %d, decrease = %d, shift = %d, period = %d (%d hz)",
                  this->square1.sweep.enabled, this->square1.sweep.decrease,
                  this->square1.sweep.shift, this->square1.sweep.period,
                  refreshRateFreq);
        PrintInfo("  frequency = %d", frequency);
    }
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
    this->square2.sweep.enabled = value & (1 << 8);
    this->square2.sweep.decrease = value & (1 << 3); // decrease or increase the wavelength
    this->square2.sweep.shift = value & 0x7; // right shift amount
    this->square2.sweep.period = (value >> 4) & 0x7; // update rate

    if (this->square2.sweep.enabled) {
        PrintDbg("  sweep enabled = %d, increase = %d, shift = %d",
                 this->square2.sweep.enabled, this->square2.sweep.decrease, this->square2.sweep.shift);
    }
}

void Audio::execute(int cycles) {
    apuCycles += cycles;

    if (apuCycles >= 14913) {
        apuCycles = 0;

        executeHalfFrame();
    }
}

void Audio::configureFrameSequencer(tCPU::byte value) {
    bool mode = value & (1 << 7);
    bool clearInterrupt = value & (1 << 6);

    PrintInfo("  frame rate mode = %d, clear interrupt = %d", mode, clearInterrupt);

    if (mode) {
        // 5-step sequence
    } else {
        // 4-step sequence
    }
}

void Audio::executeHalfFrame() {
    // execute sweep unit

    if (this->square1.sweep.enabled) {
        if (this->square1.sweep.shift > 0) {
            PrintInfo("Applying note sweep, shift delta = %d", this->square1.note >> this->square1.sweep.shift);
        }
    }


    if (this->square1.sweep.decrease) {
//        this->square1.note -= this->square1.note >> this->square1.sweep.shift;
    } else {
//        this->square1.note += this->square1.note >> this->square1.sweep.shift;
    }

    // execute length counters

}


