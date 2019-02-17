
#include "Audio.h"
#include "Logging.h"
#include <thread>
#include <chrono>
#include <fftw3.h>

#define AUDIO_ENABLED true

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
 * http://web.textfiles.com/games/nessound.txt -- excellent envelope decay and sweep unit details
 */

static int dutyCycleSequence[4][8] = {
        {1, 0, 0, 0, 0, 0, 0, 0}, // 0, 12.5%
        {1, 1, 0, 0, 0, 0, 0, 0}, // 1, 25%
        {1, 1, 1, 1, 0, 0, 0, 0}, // 2, 50%
        {1, 1, 1, 1, 1, 1, 0, 0}  // 3. 75%
};

static int triangleSequence[] = {
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

const double cpuFrequency = 1789773;

/*
 * The triangle wave channel has the ability to generate an output triangle
wave with a resolution of 4-bits (16 steps), in the range of 27.3 Hz to 55.9
KHz.
 */

void
Audio::populate(Uint8 *stream, int len) {
    for (int i = 0; i < len; i++) {
        Uint8 total = 128;

        int step = square1.phase / 2048;
        int value = dutyCycleSequence[square1.dutyCycle][step] ? 1 * square1.volume : 0;
        square1.phase += cpuFrequency / (16 * (square1.timerPeriodReloader - 1));
        square1.phase %= 65536;
        total += value;

//        step = square2.phase / 8192;
//        value = dutyCycleSequence[square2.dutyCycle][step] ? 2 * square2.volume : 0;
//        square2.phase += cpuFrequency / (16 * (square2.timerPeriodReloader));
//        square2.phase %= 65536;
//        total += value;

        stream[i] = total;
    }
}

Audio::Audio() {
    square1.buffer = new tCPU::byte[2048];
    memset(square1.buffer, 128, 2048);

    buffer = new tCPU::byte[bufferSize];
    memset(buffer, 128, bufferSize);

    fftInput = (double *) fftw_malloc(sizeof(double) * fftSize);
    fftOutput = (double *) fftw_malloc(sizeof(double) * fftSize);

    fftPlan = fftw_plan_r2r_1d(fftSize, fftInput, fftOutput, FFTW_R2HC, 0);

    // open a single audio channel with unsigned 8-bit samples
    // 44.1 khz and 1024 sample buffers
    // callback is fired 43 times a second
    SDL_AudioSpec spec = {
            .freq = 44100,
            .format = AUDIO_U8,
            .samples = 2048,
            .channels = 1,

            // functor
//            .callback = populateFuncPtr,
            .userdata = this,
    };

#if AUDIO_ENABLED
    if (SDL_OpenAudio(&spec, NULL) < 0) {
        PrintError("Failed to open audio: %s", SDL_GetError());
        exit(1);
    }
#endif

    PrintApu("Audio silence value: %d", spec.silence);
}

void
Audio::close() {
#if AUDIO_ENABLED
    /* Then turn it off again */
    SDL_PauseAudio(1);

    /* Close audio channel */
    SDL_CloseAudio();
#endif
}

void
Audio::setChannelStatus(tCPU::byte status) {
    this->channelStatus = status;

    square1.enabled = status & 1;
    square2.enabled = status & 2;
    triangle.enabled = status & 4;
    bool noise = status & 8;
    bool dmc = status & 16;

    PrintApu("Set channels: Square 1 = %d / Square 2 = %d / Triangle = %d / Noise = %d / DMC = %d",
             square1.enabled, square2.enabled, triangle.enabled, noise, dmc);

#if AUDIO_ENABLED
    if (square1.enabled || square2.enabled || triangle.enabled) {
        SDL_PauseAudio(0);
    } else {
        SDL_PauseAudio(1);
    }
#endif
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
    this->square1.volume = value & 0x0f; // constant volume or envelope decay period

    // if bit is set (1): envelope decay is disabled and volume is sent directly to DAC
    // else: volume is used as a decay rate (240Hz/(volume+1) to decrement volume
    this->square1.sawEnvelopeDisabled = value & (1 << 4); // if true, use constant volume, envelope decay is disabled
    this->square1.lengthCounterDisabled = value & (1 << 5);
    this->square1.dutyCycle = (value & 0xc0) >> 6;

    PrintApu("  volume = %d / saw-disabled = %d / length-disabled = %d / duty = %d",
             this->square1.volume, this->square1.sawEnvelopeDisabled,
             this->square1.lengthCounterDisabled, this->square1.dutyCycle);
}

// $4003
void Audio::setSquare1NoteHigh(tCPU::byte value) {
    square1.timerPeriod &= 0x00ff; // clear upper bits
    square1.timerPeriod |= (value & 0x7) << 8; // OR upper 3 bits
    square1.lengthCounterLoad = (value & 0xf8) >> 3; // upper 5 bits

    square1.timerPeriodReloader = square1.timerPeriod + 1;
    square1.timerValue = square1.timerPeriodReloader;
//    square1.dutyStep = 0;

    PrintApu("  timerPeriod (high bits) = %d / lengthCounter = %d", this->square1.timerPeriod,
             this->square1.lengthCounterLoad);
}

void Audio::setSquare1NoteLow(tCPU::byte value) {
    this->square1.timerPeriod &= 0xff00; // clear lower 8 bits
    this->square1.timerPeriod |= value; // OR lower 8 bits
    PrintApu("  timerPeriod (low bits) = %d", this->square1.timerPeriod);
}

void Audio::setSquare1Sweep(tCPU::byte value) {
    this->square1.sweep.enabled = value & (1 << 7);
    this->square1.sweep.decrease = value & (1 << 3); // decrease or increase the wavelength
    this->square1.sweep.shift = value & 0x7; // right shift amount
    this->square1.sweep.period = (value >> 4) & 0x7; // update rate

    tCPU::byte refreshRateFreq = 120 / (this->square1.sweep.period + 1);

    auto frequency = cpuFrequency / (16 * (this->square1.timerPeriod + 1));

    if (this->square1.sweep.enabled) {
        PrintApu("  sweep enabled = %d, decrease = %d, shift = %d, period = %d (%d hz)",
                 this->square1.sweep.enabled, this->square1.sweep.decrease,
                 this->square1.sweep.shift, this->square1.sweep.period,
                 refreshRateFreq);
        PrintApu("  frequency = %d", frequency);
    }
}

void
Audio::setSquare2Envelope(tCPU::byte value) {
    square2.volume = value & 0x0f;
    square2.sawEnvelopeDisabled = value & (1 << 4);
    square2.lengthCounterDisabled = value & (1 << 5);
    square2.dutyCycle = (value & 0xc0) >> 6;

    PrintDbg("  volume = %d / saw-disabled = %d / length-disabled = %d / duty = %d",
             square2.volume, square2.sawEnvelopeDisabled,
             square2.lengthCounterDisabled, square2.dutyCycle);
}

void Audio::setSquare2NoteHigh(tCPU::byte value) {
    square2.timerPeriod &= 0x00ff; // clear upper bits
    square2.timerPeriod |= (value & 0x7) << 8; // OR upper 3 bits
    square2.lengthCounterLoad = (value & 0xf8) >> 3; // upper 5 bits

    square2.timerPeriodReloader = square2.timerPeriod + 1;
    square2.timerValue = square2.timerPeriodReloader;
//    square2.dutyStep = 0;

    PrintApu("  timerPeriod (high bits) = %d / lengthCounter = %d", this->square2.timerPeriod,
             this->square2.lengthCounterLoad);
}

void Audio::setSquare2NoteLow(tCPU::byte value) {
    this->square2.timerPeriod &= 0xff00; // clear lower 8 bits
    this->square2.timerPeriod |= value; // OR lower 8 bits
    PrintApu("  timerPeriod (low bits) = %d", this->square2.timerPeriod);
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

void Audio::configureFrameSequencer(tCPU::byte value) {
    bool mode = value & (1 << 7);
    bool clearInterrupt = value & (1 << 6);

    PrintApu("  frame rate mode = %d, clear interrupt = %d", mode, clearInterrupt);

    if (mode) {
        this->frameCounterMode = FIVE_STEP;
    } else {
        this->frameCounterMode = FOUR_STEP;
    }

    this->issueIRQ = !clearInterrupt;

    // reset length counters
//    triangle.lengthCounter = 0;
}

void Audio::execute(int cpuCycles) {
    // for simplicity we will use 1 apu cycle = 1 cpu cycle
    apuCycles += cpuCycles;
    apuSampleCycleCounter += cpuCycles;

    // 1789773 cycles per second for nes CPU
    // 44100 samples per second for soundcard
    // 1789773 / 44100 = 40.5844217 cpu cycles per sample
    // 1789773 / 48000 = 37.2869375 cpu cycles per sample
    // we need to sample the APU about every 40 cpu cycles
    // and once enough samples are collected, push to the soundcard
    //   512 samples = 12msec of audio
    //   1024 samples = 23msec of audio
    //   2048 = 46msec of audio
    // vsync @ 60hz = 16msec per frame
    // with up to 5-10msec delay per frame for vsync, 1024 is smallest safe buffer to use

    static int sampleInterval = 40;

    if (apuSampleCycleCounter >= sampleInterval) {
        // clock pulse channels every other CPU cycle
        for (int i = 0; i < apuSampleCycleCounter; i += 2) {
            if (square1.timerValue == 0) {
                // timer hit, step in duty cycle
                square1.dutyStep = (square1.dutyStep + 1) % 8;
                // reset timer
                square1.timerValue = square1.timerPeriodReloader;
            } else {
                // countdown
                square1.timerValue--;
            }

            if (square2.timerValue == 0) {
                // timer hit, step in duty cycle
                square2.dutyStep = (square2.dutyStep + 1) % 8;
                // reset timer
                square2.timerValue = square2.timerPeriodReloader;
            } else {
                // countdown
                square2.timerValue--;
            }
        }

        // clock triangle channel every CPU cycle
        for (int i = 0; i < apuSampleCycleCounter; i++) {
            if (triangle.counterMode == LENGTH_COUNTER && triangle.lengthCounter > 0) {
                if (triangle.timerValue == 0) {
                    triangle.dutyStep = (triangle.dutyStep + 1) % 32;
                    triangle.timerValue = triangle.timerPeriodReloader;
                } else {
                    triangle.timerValue--;
                }
            }
        }

        // output square wave
        int value1 = 0, value2 = 0, value3 = 0;
        if (square1.enabled) {
            value1 = dutyCycleSequence[square1.dutyCycle][square1.dutyStep] ? 7 * square1.volume : 0;

            if (square1.lengthCounterLoad == 0) {
//                value1 = 0;
            }

            if (square1.timerPeriod < 8) {
                // silence what will be otherwise supersonic/popping
//                value1 = 0;
            }
        }
        if (square2.enabled) {
            value2 = dutyCycleSequence[square2.dutyCycle][square2.dutyStep] ? 7 * square2.volume : 0;
        }

        if (triangle.enabled) {
//            if(triangle.counterMode == LENGTH_COUNTER && triangle.lengthCounter > 0) {
            value3 = (triangleSequence[triangle.dutyStep]) * 7;
//            }
        }


        sampleWaveTime += 1.0 / 44100.0;

        double sampleAmplitude = 50;
        double sampleFrequency = 440;

//        value1 = value2 = value3 = 0;

        // sample clean sine wave
//        value1 = int(sampleAmplitude * sin(sampleFrequency * 2 * M_PI * sampleWaveTime));

        // sample clean square wave
//        value1 = sampleAmplitude * (2 * (2 * floor(sampleFrequency * sampleWaveTime) - floor(sampleFrequency * 2 * sampleWaveTime)) + 1);

        // sample clean pulse wave
//        value1 = sampleAmplitude * ((2 * floor(sampleFrequency * sampleWaveTime) - floor(sampleFrequency * 2 * sampleWaveTime)) + 1);

        // nes sequencer pulse wave
//        int step = (int) floor(8 * (sampleFrequency * sampleWaveTime)) % 8;
//        value1 = sampleAmplitude * dutyCycleSequence[2][step];

        // sample clean triangle wave
//        value1 = sampleAmplitude * M_2_PI * asin(sin(sampleFrequency * 2 * M_PI * sampleWaveTime));
//        value1 = 105 * (M_1_PI * asin(sin(sampleFrequency * 2 * M_PI * sampleWaveTime)) + .5);

        // nes sequencer triangle wave
//        int step = (int) round(30 * (sampleFrequency * sampleWaveTime)) % 30;
//        value1 = (triangleSequence[step]) * 7;

        // convert volume to unsigned-char amplitude value where 128 is silence
//        int normalizedSquareOut = 95.88 / (8128 / (value1+value2) + 100);
//        if(bufferWriteIdx < bufferSize - 1) {

        int amplitude = value1 + value2 + value3;

        buffer[bufferWriteIdx++] = 128 + amplitude;
//            bufferWriteIdx = ++bufferWriteIdx % bufferSize;
//            bufferAvailable++;
//        }
        apuSampleCycleCounter = 0;

        fftInput[fftIdx++] = amplitude;

        if (fftIdx == 2048) {
            fftw_execute(fftPlan);

            // sample rate = 44100
            // fft size = 2048
            // each bucket = 21 samples

            int peakPower = 0;
            int peakPowerIdx = 0;
            for (int i = 1; i < 1024; i++) {
                int power = sqrt(fftOutput[i] * fftOutput[i]);
                if (power > peakPower) {
                    peakPower = power;
                    peakPowerIdx = i;
                }
            }

            int bucketFreq = 44100 / 2048;

            int rows = 5;
            int columns = 100;
            int rowPower = peakPower / rows;

            printf("peakPower = %d @ frequency = %d\n", peakPower, peakPowerIdx * bucketFreq);

            for (int r = rows; r >= 0; r--) {
                int cutoff = r * rowPower;

                if (r == rows) {
                    for (int i = 0; i < columns; i += 10) {
                        printf("%-10d", i * bucketFreq);
                    }
                    printf("\n");
                }

                for (int i = 0; i < columns; i++) {
                    int power = sqrt(fftOutput[i] * fftOutput[i]);
                    if (power >= cutoff) {
                        printf("|");
                    } else {
                        printf(" ");
                    }
                }

                printf("\n");
            }


            fftIdx = 0;
        }

        const int samplesPerSecond = 44100;
        const int writeSize = 441; // 10 msec worth of samples @ 44.1khz

        if (bufferWriteIdx >= writeSize) {
            int queued = SDL_GetQueuedAudioSize(1);
//            if(queued > 2000 && sampleInterval < 55) {
//                sampleInterval++;
//            }
//
//            if(queued < 1000 && sampleInterval > 38) {
//                sampleInterval --;
//            }

            PrintApu("Soundcard has %d bytes queued. sampleInterval = %d", queued, sampleInterval);

            if (queued > 20000) {
                SDL_ClearQueuedAudio(1);
                PrintApu("*** DROPPED AUDIO QUEUE ***");
            }

            if (square1.enabled || square2.enabled || triangle.enabled) {
                if (SDL_QueueAudio(1, buffer, bufferWriteIdx) != 0) {
                    PrintError("SDL_QueueAudio() had an error: %s", SDL_GetError());
                }
            }

            queued += bufferWriteIdx;

            // keep queue from growing too large
            const int maxQueueSampleDepth = samplesPerSecond / 5;
            if (queued > maxQueueSampleDepth) {
                int delay = (int) round(double(queued - maxQueueSampleDepth) / (double(samplesPerSecond) / 1000.0));
                if (delay > 1) {
                    auto now = std::chrono::high_resolution_clock::now();
                    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
                    auto actual_sleep = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() - now);
                    PrintApu("throttling apu: wanted=%d msec got=%d msec", delay, actual_sleep);
                }
            }

            bufferWriteIdx -= writeSize;
        }
    }

    // progress frame sequencer
    switch (apuCycles) {
        case 37281:
            if (this->frameCounterMode == FIVE_STEP) {
                executeQuarterFrame();
                executeHalfFrame();
                apuCycles = 0;
            }
            break;
        case 29828:
            if (this->frameCounterMode == FOUR_STEP) {
                executeQuarterFrame();
                executeHalfFrame();
                apuCycles = 0;

                if (issueIRQ) {
                    issueIRQ = false;
                    PrintApu("UNIMPLEMENTED: Issue IRQ at last tick of 4 step sequencer");
                }
            }
            break;
        case 22371:
            executeQuarterFrame();
            break;
        case 14913:
            executeQuarterFrame();
            executeHalfFrame();
            break;
        case 7457:
            executeQuarterFrame();
            break;
        default:
            break;
    }
}

void Audio::executeQuarterFrame() {
    // update envelopes and triangle's linear counter (~240hz)

    if (!this->square1.lengthCounterDisabled && this->square1.lengthCounterLoad > 0) {
        this->square1.lengthCounterLoad--;
    }

    if (triangle.linearCounter > 0) {
        triangle.linearCounter--;
    }
}

void Audio::executeHalfFrame() {
    // update length counters and sweep units (~120hz)


    if (this->square1.sweep.enabled) {
        if (this->square1.sweep.shift > 0) {
//            PrintApu("Applying note sweep, shift delta = %d", this->square1.note >> this->square1.sweep.shift);
        }
    }


    if (this->square1.sweep.decrease) {
//        this->square1.note -= this->square1.note >> this->square1.sweep.shift;
    } else {
//        this->square1.note += this->square1.note >> this->square1.sweep.shift;
    }

    // execute length counters
    if (triangle.lengthCounter > 0) {
        triangle.lengthCounter--;
        //PrintApu("Updated triangle length counter to %d", triangle.lengthCounter);
    }

}

void Audio::setTriangleDuration(tCPU::byte value) {
    // bit 7 halts the length-counter, starting the linear-counter
    triangle.counterMode = ((value & 0x80) != 0) ? LINEAR_COUNTER : LENGTH_COUNTER;
    triangle.linearCounterLoad = value & 0x7f;

    triangle.controlFlagEnabled = ((value & 0x80) != 0);

    PrintApu("  counter-type = %s / linear-counter-load = %d",
             triangle.counterMode == LENGTH_COUNTER ? "length-counter" : "linear-counter",
             triangle.linearCounterLoad);
}

void Audio::setTrianglePeriodHigh(tCPU::byte value) {
    // set upper 3 bits of the 11-bit register
    triangle.timerPeriod &= 0x00ff; // clear upper 8 bits
    triangle.timerPeriod |= (value & 0x7) << 8; // OR upper 3 bits
    triangle.timerPeriodReloader = triangle.timerPeriod + 1;

    int lengthCounterIdx = (value >> 3) & 0x1f; // use upper 5 bits
    int lengthLUT[] = {10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22,
                       192, 24, 72, 26, 16, 28, 32, 30};

    triangle.lengthCounter = lengthLUT[lengthCounterIdx];
    triangle.linearCounterReloadEnabled = true;

    PrintApu("  timerPeriod = %d / length-counter = %d (idx = %d)", triangle.timerPeriod, triangle.lengthCounter,
             lengthCounterIdx);
}

void Audio::setTrianglePeriodLow(tCPU::byte value) {
    // set lower 8 bits of the 11-bit register
    triangle.timerPeriod &= 0xff00; // clear lower 8 bits
    triangle.timerPeriod |= value; // OR lower 8 bits

//    PrintApu("  timerPeriod (low bits) = %d", triangle.timerPeriod);
}


