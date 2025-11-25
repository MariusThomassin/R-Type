#!/bin/bash
set -e

BUILD_DIR="build"

echo "🔨 Building R-Type for Linux..."

# Create and enter build directory
mkdir -p "$BUILD_DIR"

# Configure
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build "$BUILD_DIR" -j$(nproc)

echo "✅ Build completed!"
echo "📦 Binaries are in: $BUILD_DIR/"
ls -lh "$BUILD_DIR"/r-type_*
