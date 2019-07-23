#pragma once

#include <iostream>
#ifdef _WIN32
	#include <cassert> // assert()
	#include <stdarg.h> // va_start()
	#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

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

#define LOG_DBG Loggy::Enabled <= Loggy::DEBUG
#define LOG_INFO Loggy::Enabled <= Loggy::INFO
#define LOG_WARNING Loggy::Enabled <= Loggy::WARNING
#define LOG_ERROR Loggy::Enabled <= Loggy::ERROR

#define PrintDbg               if(LOG_DBG) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintInfo              if(LOG_INFO) Loggy::log(__PRETTY_FUNCTION__, Loggy::INFO).println
#define PrintWarning           if(LOG_WARNING) Loggy::log(__PRETTY_FUNCTION__, Loggy::WARNING).println
#define PrintError             if(LOG_ERROR) Loggy::log(__PRETTY_FUNCTION__, Loggy::ERROR).println
#define PrintCpu               if(LOG_DBG) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintMemory            if(LOG_DBG) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintMemoryIO          if(LOG_DBG) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintPpu               if(LOG_DBG) Loggy::log(__PRETTY_FUNCTION__, Loggy::DEBUG).println
#define PrintApu               if(LOG_INFO) Loggy::log(__PRETTY_FUNCTION__, Loggy::INFO).println
#define PrintUnimplementedIO   if(LOG_INFO) Loggy::log(__PRETTY_FUNCTION__, Loggy::INFO).println


//static std::string boostFormatWrapper(boost::format &f) {
//    return boost::str(f);
//}
//
//template<class T, class... Ts>
//static std::string boostFormatWrapper(boost::format &f, T &&t, Ts &&... args) {
//    return boostFormatWrapper(f % std::forward<T>(t), std::forward<Ts>(args)...);
//}

template<typename... Ts>
static void ThrowException(const char *fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start (args, fmt);
    vsprintf(buffer, fmt, args);
    va_end (args);

    throw std::runtime_error(buffer);
}

static void PrintBitState(char byte, char mask, const char *label) {
    PrintInfo("%s: %s", label, ((byte & mask) ? "True" : "False"));
}

static void PrintFlagState(char byte, char mask, const char *label, const char *onString, const char *offString) {
    PrintInfo("%s: %s", label, ((byte & mask) ? onString : offString));
}
