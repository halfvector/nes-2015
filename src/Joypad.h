#ifndef NES_JOYPAD_H
#define NES_JOYPAD_H


#include "Platform.h"

enum JoypadButtons {
    A = 7,
    B = 6,
    Select = 5,
    Start = 4,
    Up = 3,
    Down = 2,
    Left = 1,
    Right = 0
};

class Joypad {
public:
    tCPU::byte getStatePlayerOne();
    tCPU::byte getStatePlayerTwo();

    void buttonDown(JoypadButtons jb);
    void reset();

    void setStrobe(tCPU::byte value);

    void buttonUp(JoypadButtons button);

private:
    int strobe = 0;
    int currentButton = JoypadButtons::A;
    int buttonStates = 0;
};


#endif