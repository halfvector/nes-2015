#ifndef _WIN32
#include "Backtrace.h"

#include <regex>
#include <iostream>
#include <string>
#include <execinfo.h>
#include <cxxabi.h>
#include <signal.h>

/**
 * Handle fatal signals by dumping out a stacktrace
 */
void fatalSignalHandler(int sig_num, siginfo_t *info, void *ucontext) {
    ucontext_t *uct = (ucontext_t *) ucontext;

    void* caller_address = Backtrace::getCallerAddress(uct);

    std::cerr << "-------------------------------------------------" << std::endl;

    std::cerr << "Caught '" << strsignal(sig_num) << "' @ address "
              << info->si_addr << " from " << caller_address << std::endl;

    Backtrace::backtrace(caller_address);

    exit(EXIT_FAILURE);
}

// borrowed from https://github.com/cinience/saker/blob/728a98df92bf413b2275b1b3ec8baa2eed4167f6/src/utils/debug.c#L43
// gets address at the time the signal was raised from the EIP (x86) or RIP (x64)
void*
Backtrace::getCallerAddress(ucontext_t *uc) {
#if defined(__APPLE__) && !defined(MAC_OS_X_VERSION_10_6)
    /* OSX < 10.6 */
#if defined(__x86_64__)
    return (void *) uc->uc_mcontext->__ss.__rip;
#elif defined(__i386__)
    return (void *) uc->uc_mcontext->__ss.__eip;
#else
    return (void *) uc->uc_mcontext->__ss.__srr0;
#endif
#elif defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)
    /* OSX >= 10.6 */
#if defined(_STRUCT_X86_THREAD_STATE64) && !defined(__i386__)
    return (void *) uc->uc_mcontext->__ss.__rip;
#else
    return (void *) uc->uc_mcontext->__ss.__eip;
#endif
#elif defined(__linux__)
    /* Linux */
#if defined(__i386__)
    return (void *) uc->uc_mcontext.gregs[14]; /* Linux 32 */
#elif defined(__X86_64__) || defined(__x86_64__)
    return (void *) uc->uc_mcontext.gregs[16]; /* Linux 64 */
#elif defined(__ia64__) /* Linux IA64 */
    return (void *) uc->uc_mcontext.sc_ip;
#endif
#else
    return NULL;
#endif
}

/**
 * install signal handlers to catch crashes and report a nice backtrace
 */
void
Backtrace::install() {
    struct sigaction sigact;
    sigact.sa_flags = SA_ONSTACK | SA_SIGINFO | SA_RESTART | SA_64REGSET;

    sigact.sa_sigaction = fatalSignalHandler;
    if (sigaction(SIGABRT, &sigact, (struct sigaction *)NULL) != 0) {
        std::cerr << "error setting handler for signal " << SIGABRT
                << " (" << strsignal(SIGABRT) << ")\n";
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0) {
        std::cerr << "error setting handler for signal " << SIGABRT
                << " (" << strsignal(SIGABRT) << ")\n";
        exit(EXIT_FAILURE);
    }
}

void
Backtrace::backtrace(void* address) {
    void* trace[50];
    int numFrames = ::backtrace(trace, 50);

    std::cerr << "Backtrace found with " << numFrames << " frames:" << std::endl;
    std::cerr << "-------------------------------------------------" << std::endl;

    // overwrite sigaction with caller's address
    trace[1] = address;

    char** messages = ::backtrace_symbols(trace, numFrames);
    char* unmangled = new char[256];

    // skip first stack frame (points here)
    for (int i = 1; i < numFrames && messages != NULL; ++i) {
        demangle(messages[i], unmangled, 256);
        std::cerr << unmangled << std::endl;
    }

    std::cerr << "-------------------------------------------------" << std::endl;

    delete[] unmangled;
}

void
Backtrace::demangle(const char* mangled, char *unmangled, size_t bufferSize) {
    std::regex regex("(\\d+)\\s+(\\S+)\\s+(0x\\S+) (.*) \\+ (\\d+)");
    std::cmatch matches;

    const char *frameNumber, *memoryAddress, *library, *method;

    if(std::regex_search(mangled, matches, regex)) {
        frameNumber = matches[1].str().c_str();
        library = matches[2].str().c_str();
        memoryAddress = matches[3].str().c_str();

        // attempt to demangle into 'unmangled' buffer
        int status;
        method = abi::__cxa_demangle(matches[4].str().c_str(), 0, 0, &status);

        if(status != 0) {
            // couldn't demangle, copy original function name to 'unmangled' buffer
            method = matches[4].str().c_str();
        }

        // line-number offset function [library]
        snprintf(unmangled, bufferSize, "%3s [%s] %s", frameNumber, library, method);
    }
}
#endif