#pragma once

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <spdlog/logger.h>
#include <iostream>
//#include "halfvector/easylogging/easylogging++.h"

//extern el::Logger *logger;
//extern auto console = nullptr;
extern std::shared_ptr<spdlog::logger> console;

//std::string simplifyFunctionName(std::string prettyFunction);
void simplifyFunctionName(const char *name, char *shorter, size_t maxLength);

//#define __METHOD_NAME__ simplifyFunctionName(__PRETTY_FUNCTION__)


//void printf(BasicWriter<Char> &w, BasicCStringRef<Char> format, ArgList args)
//{
//    internal::PrintfFormatter<Char>(args).format(w, format);
//}

struct Loggy {
    char buffer[128];

    static Loggy log(const char *name) {
        Loggy x{};
        x.setCaller(name);
        return x;
    }

    void setCaller(const char *name) {
        simplifyFunctionName(name, buffer, 128);
    }

    template<typename... Args>
    void println(const char* fmt, const Args &... args) {
        fputs(buffer, stdout);
        fmt::printf(fmt, args...);
        fputs("\n", stdout);
    }
};

#define PrintDbg        Loggy::log(__PRETTY_FUNCTION__).println
#define PrintInfo       Loggy::log(__PRETTY_FUNCTION__).println
#define PrintWarning    Loggy::log(__PRETTY_FUNCTION__).println
#define PrintError      Loggy::log(__PRETTY_FUNCTION__).println

#define PrintCpu        Loggy::log(__PRETTY_FUNCTION__).println
#define PrintMemory     Loggy::log(__PRETTY_FUNCTION__).println
#define PrintMemoryIO   Loggy::log(__PRETTY_FUNCTION__).println
#define PrintPpu        Loggy::log(__PRETTY_FUNCTION__).println


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
