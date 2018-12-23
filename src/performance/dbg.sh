#!/bin/sh

g++ -I ../../../../deps -I ~/.biicode/boost/1.57.0/ -std=c++11 -O2 -g -c Loops.cpp
gobjdump -d -M intel -S Loops.o > Loops.s
