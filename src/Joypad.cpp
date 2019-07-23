#include "Joypad.h"

tCPU::byte
Joypad::getStatePlayerOne() {
    tCPU::byte value = (buttonStates & (1 << currentButton)) >> currentButton;

    if(!this->strobe) {
        if (--currentButton < 0) {
            currentButton = JoypadButtons::A;
        }
    }
    return value;
}

void
Joypad::buttonDown(JoypadButtons jb) {
    // set button
    this->buttonStates |= 1 << jb;
}

void
Joypad::buttonUp(JoypadButtons jb) {
    // clear button
    this->buttonStates &= ~(1 << jb);
}

void
Joypad::reset() {
    this->buttonStates = 0;
    this->currentButton = JoypadButtons::A;
}

void
Joypad::setStrobe(tCPU::byte value) {
    this->strobe = value;
}
