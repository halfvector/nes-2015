/**
 * Print a simple "Hello world!"
 *
 * @file main.cpp
 * @section LICENSE

    This code is under MIT License, http://opensource.org/licenses/MIT
 */

#include <iostream>
#include "easylogging++.h"

// init logging
INITIALIZE_EASYLOGGINGPP

int main() {
  LOG(INFO) << "Hello There!";
}
