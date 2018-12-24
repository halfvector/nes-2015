#include "Logging.h"

int targetCounter = 0;
const int NUM_ITERATIONS = 1000;
typedef std::chrono::high_resolution_clock clock_type;

void resetCounter() {
    targetCounter = 0;
}

void targetFunction() {
    ++targetCounter;
}

void functionPointers() {
    // warm up the timer
    clock_type::time_point start, stop;
    clock_type::now();
    clock_type::now();

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

    long nanosecondsPerCall = span / targetCounter;

    PrintInfo("Function Pointer: total = %i nanoseconds; overhead = %i ns; per call = %i ns")
            % (span - overhead)
            % overhead
            % nanosecondsPerCall;
}

void functionLambda() {
    // warm up the timer
    clock_type::time_point start, stop;
    clock_type::now();
    clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    auto ptr = []() {
        ++targetCounter;
    };

    resetCounter();

    start = clock_type::now();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ptr();
    }
    stop = clock_type::now();

    long span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    long nanosecondsPerCall = span / targetCounter;

    PrintInfo("Function Lambda: total = %i nanoseconds; overhead = %i ns; per call = %i ns")
            % (span - overhead)
            % overhead
            % nanosecondsPerCall;
}

/**
 * Test method-ptr vs lambda for callbacks in a tight loop
 */
int main() {
    functionPointers();
    functionLambda();
    return 0;
}