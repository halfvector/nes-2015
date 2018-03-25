#pragma once

#include "Logging.h"

template<typename ExceptionType>
class ExceptionBase : public std::runtime_error {
public:
    ExceptionBase(std::string const &str) : std::runtime_error(str) {}

    template<typename... Ts>
    static ExceptionType emit(const char* format, Ts &&... args) {
        boost::format fmt(format);
        std::string result = boostFormatWrapper(fmt, std::forward<Ts>(args)...);
        PrintError(result);
        throw new ExceptionType(result);
    }
};