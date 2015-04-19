#pragma once

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include "halfvector/easylogging/easylogging++.h"

extern el::Logger *logger;

std::string simplifyFunctionName(std::string prettyFunction);

#define __METHOD_NAME__ simplifyFunctionName(__PRETTY_FUNCTION__)

#define PrintDbg        LOG(DEBUG) << __METHOD_NAME__ << boost::format
#define PrintInfo       LOG(INFO) << __METHOD_NAME__ << boost::format
#define PrintWarning    LOG(WARNING) << __METHOD_NAME__ << boost::format
#define PrintError      LOG(ERROR) << __METHOD_NAME__ << boost::format

#define PrintCpu        LOG(DEBUG) << __METHOD_NAME__ << boost::format
#define PrintMemory     LOG(DEBUG) << __METHOD_NAME__ << boost::format
#define PrintPpu        LOG(DEBUG) << __METHOD_NAME__ << boost::format

#if 1
#define PrintMemory     boost::format
#endif

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
    throw new std::runtime_error(result);
}

static void PrintBitState(char byte, char mask, const char *label) {
    PrintInfo("%s: %s") % label % ((byte & mask) ? "True" : "False");
}

static void PrintFlagState(char byte, char mask, const char *label, const char *onString, const char *offString) {
    PrintInfo("%s: %s") % label % ((byte & mask) ? onString : offString);
}
