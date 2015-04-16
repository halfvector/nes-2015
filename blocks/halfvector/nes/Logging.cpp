#include "Logging.h"

INITIALIZE_EASYLOGGINGPP;
el::Logger *logger = el::Loggers::getLogger("default");

std::string simplifyFunctionName(std::string prettyFunction) {
    size_t begin = prettyFunction.find(" ") + 1;

    // skip attributes, look for next token
    if(prettyFunction.substr(0, begin).find("static") != std::string::npos) {
        begin = prettyFunction.find(" ", begin) + 1;
    }

    size_t length = prettyFunction.rfind("(") - begin;

    return prettyFunction.substr(begin, length) + "(); ";
}
