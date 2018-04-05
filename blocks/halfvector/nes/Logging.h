#pragma once

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <spdlog/logger.h>
#include <iostream>

struct Loggy {
    enum Type {
        DEBUG = 0x1, INFO = 0x2, WARNING = 0x4, ERROR = 0x8
    };
    char methodName[128];
    Type type;

    static Type Enabled;

    static Loggy log(const char *name, Type type);

    void simplifyFunctionName(const char *name, char *shorter, size_t maxLength);

    void println(const char *fmt, ...);
};

// universal
#define Logging                false

#define PrintDbg               if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintInfo              if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::INFO).println
#define PrintWarning           if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::WARNING).println
#define PrintError             if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::ERROR).println
#define PrintCpu               if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintMemory            if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintMemoryIO          if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintPpu               if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintUnimplementedIO   if(Logging) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println


static std::string boostFormatWrapper(boost::format &f) {
    return boost::str(f);
}

template<class T, class... Ts>
static std::string boostFormatWrapper(boost::format &f, T &&t, Ts &&... args) {
    return boostFormatWrapper(f % std::forward<T>(t), std::forward<Ts>(args)...);
}

template<typename... Ts>
static void ThrowException(std::string const &fmt, Ts &&... args) {
    boost::format f(fmt);
    auto result = boostFormatWrapper(f, std::forward<Ts>(args)...);
    throw std::runtime_error(result);
}

static void PrintBitState(char byte, char mask, const char *label) {
    PrintInfo("%s: %s", label, ((byte & mask) ? "True" : "False"));
}

static void PrintFlagState(char byte, char mask, const char *label, const char *onString, const char *offString) {
    PrintInfo("%s: %s", label, ((byte & mask) ? onString : offString));
}
