#pragma once

#include "halfvector/easylogging/easylogging++.h"
#include "boost/format.hpp"

extern el::Logger *log;

#define PrintDbg        LOG(DEBUG) << boost::format
#define PrintInfo       LOG(INFO) << boost::format
#define PrintWarning      LOG(WARNING) << boost::format
#define PrintError      LOG(ERROR) << boost::format

static void PrintBitState(char byte, char mask, const char *label) {
    PrintInfo("%s: %s") % label % ((byte & mask) ? "True" : "False");
}

static void PrintFlagState(char byte, char mask, const char *label, const char *onString, const char *offString) {
    PrintInfo("%s: %s") % label % ((byte & mask) ? onString : offString);
}
