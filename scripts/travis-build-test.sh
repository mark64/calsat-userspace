#!/usr/bin/env sh
mkdir -p build
cd build
cmake ..
make all test coverage
