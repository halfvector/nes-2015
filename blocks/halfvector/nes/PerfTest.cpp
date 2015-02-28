#include <iostream>
#include "boost/format.hpp"
#include "boost/filesystem.hpp"
#include "halfvector/easylogging/easylogging++.h"

using namespace boost;

void simpleLoopPerformanceTests()  {
    // fill array with repeating data
    const unsigned int ARRAY_SIZE = 500;
    char sampleArray[ARRAY_SIZE];
    for(int i = 0; i < ARRAY_SIZE; i++) {
        sampleArray[i] = i % 200;
    }

    typedef std::chrono::high_resolution_clock clock_type;

    clock_type::time_point start = clock_type::now();
    clock_type::time_point stop = clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    start = clock_type::now();
    stop = clock_type::now();

    long overhead = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    LOG(INFO) << boost::format("Timing overhead: %|1| nanoseconds") % overhead;

    start = clock_type::now();

    // using fancy c++14 stl any_of + lambda
    // any_of() early-outs, the rest of the tests below do not
    bool headerIsClean = true;
    headerIsClean = std::any_of(
            std::begin(sampleArray), std::end(sampleArray),
            [](int value) {
                return value != 0;
            }
    );

    stop = clock_type::now();
    LOG(INFO) << boost::format("Time: any_of = %|1| nanoseconds") % std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    start = clock_type::now();

    // use c++11 range loop
    headerIsClean = true;
    for (auto &bit : sampleArray) {
        if (bit) {
            headerIsClean = false;
        }
    }

    stop = clock_type::now();
    LOG(INFO) << boost::format("Time: foreach = %|1| nanoseconds") % std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    start = clock_type::now();

    // simple for loop with branching
    headerIsClean = true;
    for (unsigned int i = 0; i < ARRAY_SIZE; i++) {
        if (sampleArray[i] != 0) {
            headerIsClean = false;
        }
    }

    stop = clock_type::now();
    LOG(INFO) << boost::format("Time: for + branch = %|1| nanoseconds") % std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();    start = clock_type::now();

    // ptr iteration
    headerIsClean = true;
    for (char *bitPtr = sampleArray; bitPtr != sampleArray + ARRAY_SIZE; bitPtr++) {
        if (*bitPtr != 0) {
            headerIsClean = false;
        }
    }

    stop = clock_type::now();
    LOG(INFO) << boost::format("Time: for ptr iter = %|1| nanoseconds") % std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    start = clock_type::now();

    // replacing branching with bitwise arithmetic
    headerIsClean = true;
    for (unsigned int i = 0; i < ARRAY_SIZE; i++) {
        headerIsClean |= sampleArray[i];
    }

    stop = clock_type::now();
    LOG(INFO) << boost::format("Time: for branchless = %|1| nanoseconds") % std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();

    // warn if header contains reserved bits (not supported)
    if (!headerIsClean) {
        LOG(INFO) << "heaader is using reserved bits";
    }
}