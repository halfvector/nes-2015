#!/bin/sh

(cd cmake-build-debug && conan install .. -s build_type=Debug --build sdl2_ttf)

