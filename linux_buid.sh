#!/bin/bash
mkdir -p build-linux
cd build-linux

# Simple CMake build without toolchain file
cmake ..
make -j$(nproc)