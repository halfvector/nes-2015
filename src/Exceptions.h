#pragma once

#include "Logging.h"

template<typename ExceptionType>
class ExceptionBase : public std::runtime_error {
public:
    explicit ExceptionBase(const char *str) : std::runtime_error(str) {}

    static void emit(const char *fmt, ...) {
        char buffer[1024];
        va_list args;
        va_start (args, fmt);
        vsprintf(buffer, fmt, args);
        va_end (args);

        PrintError(buffer);
        throw ExceptionType(buffer);
    }
};