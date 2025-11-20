#!/bin/bash
mkdir -p build-linux

# Simple CMake build without toolchain file
cmake -S . -B build-linux -DCMAKE_BUILD_TYPE=Release
make -C build-linux -j$(nproc)

echo -e "\n\nBuild completed. Binaries are located in the build-linux directory."
