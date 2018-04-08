#include "Joypad.h"

tCPU::byte
Joypad::getStatePlayerOne() {
    tCPU::byte value = (buttonStates & (1 << currentButton)) >> currentButton;
//    value |= 1 << 4;

    currentButton = --currentButton % 8;
    return value;
}

void
Joypad::buttonDown(JoypadButtons jb) {
    // set button
    reset();
    this->buttonStates |= 1 << jb;
}

void
Joypad::reset() {
    this->buttonStates = 0;
    this->currentButton = JoypadButtons::A;
}
