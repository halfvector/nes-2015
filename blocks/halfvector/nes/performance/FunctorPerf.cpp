#include "../Logging.h"

int g_TargetCounter = 0;

void resetCounter() {
    g_TargetCounter = 0;
}

void targetFunction() {
    ++g_TargetCounter;
}

const int NUM_ITERATIONS = 1000;

typedef std::chrono::high_resolution_clock clock_type;

void functionPointers() {
    // warm up the timer
    clock_type::time_point start = clock_type::now();
    clock_type::time_point stop = clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    typedef void (*funcPtr)();
    funcPtr ptr = &targetFunction;

    resetCounter();

    start = clock_type::now();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ptr();
    }
    stop = clock_type::now();

    long span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    long nanosecondsPerCall = span / g_TargetCounter;

    PrintInfo("Function Pointer: total = %i nanoseconds; overhead = %i ns; per call = %i ns")
            % (span - overhead)
            % overhead
            % nanosecondsPerCall;
}

void functionLambda() {
    // warm up the timer
    clock_type::time_point start = clock_type::now();
    clock_type::time_point stop = clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    auto ptr = []() {
        ++g_TargetCounter;
    };

    resetCounter();

    start = clock_type::now();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ptr();
    }
    stop = clock_type::now();

    long span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    long nanosecondsPerCall = span / g_TargetCounter;

    PrintInfo("Function Lambda: total = %i nanoseconds; overhead = %i ns; per call = %i ns")
            % (span - overhead)
            % overhead
            % nanosecondsPerCall;
}

int main() {
    functionPointers();
    functionLambda();
    return 0;
}