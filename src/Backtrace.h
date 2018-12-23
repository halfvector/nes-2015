#pragma once

#include <cstring>
#include <sys/ucontext.h>  // get REG_EIP frome ucontext.h

class Backtrace {
public:
    static void install();
    static void backtrace(void* address);
    static void demangle(const char* mangled, char* unmangled, size_t bufferSize);
    static void* getCallerAddress(ucontext_t *uc);
};
