#include "Logging.h"

// initialize statics
Loggy::Type Loggy::Enabled = Loggy::DEBUG;

// takes a __PRETTY_FUNCTION__ long method signature
void simplifyFunctionName(const char *name, char *shorter, size_t maxLength) {
    size_t nameLength = strlen(name);
    const char *start = std::find(name, name + nameLength, ' ');
    const char *end = std::find(name, name + nameLength, '(');

    // if next token is an attribute, skip it
    if (!strcmp(start, "static")) {
        start += 7;
    }

    const char *prefix = "\u001b[33m";
    const char *postfix = "\u001b[0;97m";

    snprintf(shorter, maxLength, "%s%.*s%s(); ", prefix, int(end - start), start, postfix);
}
