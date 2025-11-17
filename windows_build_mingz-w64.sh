#!/bin/bash


#---------------------------------------------------------------------------------------
# WARNING: This script assumes you have MinGW-w64 installed and available in your PATH.
# It also assumes you have CMake installed on windows and available in your PATH.
#---------------------------------------------------------------------------------------

mkdir build-windows
cd build-windows

cmake .. -G "MinGW Makefiles"

cmake --build . --config Release
