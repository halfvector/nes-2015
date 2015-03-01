#pragma once

#include "halfvector/easylogging/easylogging++.h"
#include "boost/format.hpp"

extern el::Logger *log;

#define PrintDbg        LOG(DEBUG) << boost::format
#define PrintInfo       LOG(INFO) << boost::format
#define PrintWarning    LOG(WARNING) << boost::format
#define PrintError      LOG(ERROR) << boost::format
#define PrintMemory     LOG(DEBUG) << boost::format

static std::string boostFormatWrapper(boost::format &f) {
    return boost::str(f);
}

template<class T, class... Args>
static std::string boostFormatWrapper(boost::format &f, T &&t, Args &&... args) {
    return boostFormatWrapper(f % std::forward<T>(t), std::forward<Args>(args)...);
}

template<typename... Arguments>
static void ThrowException(std::string const &fmt, Arguments &&... args) {
    boost::format f(fmt);
    auto result = boostFormatWrapper(f, std::forward<Arguments>(args)...);
    throw new std::runtime_error(result);
}

static void PrintBitState(char byte, char mask, const char *label) {
    PrintInfo("%s: %s") % label % ((byte & mask) ? "True" : "False");
}

static void PrintFlagState(char byte, char mask, const char *label, const char *onString, const char *offString) {
    PrintInfo("%s: %s") % label % ((byte & mask) ? onString : offString);
}
