#!/bin/bash


#---------------------------------------------------------------------------------------
# WARNING: This script assumes you have MinGW-w64 installed and available in your PATH.
# It also assumes you have CMake installed on windows and available in your PATH.
#---------------------------------------------------------------------------------------
# Note: You may need to adjust the generator name ("MinGW Makefiles") based on your MinGW-w64 installation.

mkdir -p build-windows

cmake -S . -B build-windows -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-windows -j$(nproc)

echo -e "\n\nBuild completed. Binaries are located in the build-windows directory."
