#include "../Logging.h"

using namespace boost;
typedef std::chrono::high_resolution_clock clock_type;

const unsigned int ARRAY_SIZE = 500;
char sampleArray[ARRAY_SIZE];

void initializeMemory() {
    // fill array with zeroes. last entry is the one we are looking for.
    memset(sampleArray, 0, ARRAY_SIZE);
    sampleArray[ARRAY_SIZE - 1] = 1;
}

void testAnyOf() {
    PrintInfo("-------------------------------------------------");

    // warm up the timer
    long span;
    clock_type::time_point start, stop;
    clock_type::now();
    clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    // calculate timer overhead
    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    PrintInfo("* Observed timer overhead: %i nanoseconds") % overhead;

    bool matchFound = false;

    /**
     * C++11 STL's any_of() + lambda
     */
    start = clock_type::now();

    matchFound = std::any_of(
            std::begin(sampleArray), std::end(sampleArray),
            [](int value) {
                return value != 0;
            }
    );

    stop = clock_type::now();
    span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    assert(matchFound);
    PrintInfo("C++11 std::any_of(): %i nanoseconds") % (span - overhead);
}

void testRangeLoop() {
    PrintInfo("-------------------------------------------------");

    // warm up the timer
    long span;
    clock_type::time_point start, stop;
    clock_type::now();
    clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    // calculate timer overhead
    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    PrintInfo("* Observed timer overhead: %i nanoseconds") % overhead;
    bool matchFound;

    /**
     * Branching + C++14's auto deduced range-loop iteration
     */
    atomic_thread_fence(std::memory_order_acq_rel);
    start = clock_type::now();

    matchFound = false;
    for (auto &bit : sampleArray) {
        if (bit) {
            matchFound = true;
        }
    }

    stop = clock_type::now();
    atomic_thread_fence(std::memory_order_acq_rel);
    span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    assert(matchFound);
    PrintInfo("Branch + C++11 range-loop: %i nanoseconds") % (span - overhead);
}

void testBranching() {
    PrintInfo("-------------------------------------------------");

    // warm up the timer
    long span;
    clock_type::time_point start, stop;
    clock_type::now();
    clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    // calculate timer overhead
    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    PrintInfo("* Observed timer overhead: %i nanoseconds") % overhead;
    bool matchFound;

    /**
     * Branching + regular for-loop
     */
    atomic_thread_fence(std::memory_order_acq_rel);
    start = clock_type::now();

    matchFound = false;
    for (uint16_t i = 0; i < ARRAY_SIZE; i++) {
        if (sampleArray[i] != 0) {
            matchFound = true;
        }
    }

    stop = clock_type::now();
    atomic_thread_fence(std::memory_order_acq_rel);
    span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    assert(matchFound);
    PrintInfo("Branch + for-loop: %i nanoseconds") % (span - overhead);

    /**
     * Branching + pointer iteration
     */
    atomic_thread_fence(std::memory_order_acq_rel);
    start = clock_type::now();

    matchFound = false;
    char *endPtr = sampleArray + ARRAY_SIZE;
    for (char *bitPtr = sampleArray; bitPtr != endPtr; bitPtr++) {
        if (*bitPtr != 0) {
            matchFound = true;
        }
    }

    stop = clock_type::now();
    atomic_thread_fence(std::memory_order_acq_rel);
    span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    assert(matchFound);
    PrintInfo("Branch + ptr-iteration: %i nanoseconds") % (span - overhead);
}

void testBranchlessBoolean() {
    PrintInfo("-------------------------------------------------");

    // warm up the timer
    long span;
    clock_type::time_point start, stop;
    clock_type::now();
    clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    // calculate timer overhead
    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    PrintInfo("* Observed timer overhead: %i nanoseconds") % overhead;
    bool matchFound;

    /**
     * Branchless (bitwise arithmetic) + regular for-loop
     */
    atomic_thread_fence(std::memory_order_acq_rel);
    start = clock_type::now();

    matchFound = false;
    for (uint16_t i = 0; i < ARRAY_SIZE; i++) {
        matchFound |= sampleArray[i];
    }

    stop = clock_type::now();
    atomic_thread_fence(std::memory_order_acq_rel);
    span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    assert(matchFound);
    PrintInfo("Branchless (boolean) + for-loop: %i nanoseconds") % (span - overhead);

    /**
     * Branchless (bitwise arithmetic) + pointer iteration
     */
    atomic_thread_fence(std::memory_order_acq_rel);
    start = clock_type::now();

    matchFound = false;
    char *endPtr = sampleArray + ARRAY_SIZE;
    for (char *bitPtr = sampleArray; bitPtr != endPtr; bitPtr++) {
        matchFound |= *bitPtr;
    }

    stop = clock_type::now();
    atomic_thread_fence(std::memory_order_acq_rel);
    span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    assert(matchFound);
    PrintInfo("Branchless (boolean) + ptr-iteration: %i nanoseconds") % (span - overhead);
}

void testBranchlessInteger() {
    PrintInfo("-------------------------------------------------");

    // warm up the timer
    long span;
    clock_type::time_point start, stop;
    clock_type::now();
    clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    // calculate timer overhead
    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    PrintInfo("* Observed timer overhead: %i nanoseconds") % overhead;
    int matchFoundInt = 0;

    /**
     * Branchless (integer bitwise arithmetic) + regular for-loop
     */
    atomic_thread_fence(std::memory_order_acq_rel);
    start = clock_type::now();

    matchFoundInt = 0;
    for (uint16_t i = 0; i < ARRAY_SIZE; i++) {
        matchFoundInt |= sampleArray[i];
    }

    stop = clock_type::now();
    atomic_thread_fence(std::memory_order_acq_rel);

    span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    assert(matchFoundInt == 1);
    PrintInfo("Branchless (integer) + for-loop: %i nanoseconds") % (span - overhead);

    /**
     * Branchless (integer bitwise arithmetic) + pointer iteration
     */
    atomic_thread_fence(std::memory_order_acq_rel);
    start = clock_type::now();

    matchFoundInt = 0;
    char* endPtr = sampleArray + ARRAY_SIZE;
    for (char *bitPtr = sampleArray; bitPtr != endPtr; bitPtr++) {
        matchFoundInt |= *bitPtr;
    }

    stop = clock_type::now();
    atomic_thread_fence(std::memory_order_acq_rel);

    span = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    assert(matchFoundInt == 1);
    PrintInfo("Branchless (integer) + ptr-iteration: %i nanoseconds") % (span - overhead);
}

/**
* Test various loop iterations
*/
int main() {
    initializeMemory();

    testAnyOf();
    testRangeLoop();
    testBranchlessBoolean();
    testBranchlessInteger();

    return 0;
}