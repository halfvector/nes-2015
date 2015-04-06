#include "MemoryStack.h"

/**
 * Manage Stack
 */

void
Stack::pushStack(unsigned short value) {
    unsigned short address = 0x1FF - reg->S;
    mem->writeByte(address, value);

    if(reg->S < 255) {
    	reg->S ++;
    } else {
    	PrintError("Stack overflow");
    	throw new std::runtime_error("Stack overflow");
    }
}

unsigned char Stack::popStack() {
    throw new std::runtime_error("popStack() not implemented");
}
