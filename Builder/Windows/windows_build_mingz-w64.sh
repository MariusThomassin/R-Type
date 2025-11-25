#!/bin/bash
set -e

#---------------------------------------------------------------------------------------
# WARNING: This script assumes you have MinGW-w64 installed and available in your PATH.
# It also assumes you have CMake installed and available in your PATH.
#---------------------------------------------------------------------------------------

BUILD_DIR="build"

echo "🔨 Building R-Type for Windows (MinGW)..."

# Create and enter build directory
mkdir -p "$BUILD_DIR"

# Configure with MinGW toolchain
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build "$BUILD_DIR" -j$(nproc)

echo "✅ Build completed!"
echo "📦 Binaries are in: $BUILD_DIR/"
ls -lh "$BUILD_DIR"/r-type_*.exe
