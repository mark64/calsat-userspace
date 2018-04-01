#!/usr/bin/env sh
mkdir -p build
cd build
cmake ..
make -j4 all
make test
make coverage
