#!/bin/bash
set -e

BUILD_DIR="build"

echo "🔨 Building R-Type for macOS..."

# Create and enter build directory
mkdir -p "$BUILD_DIR"

# Configure
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15

# Build
cmake --build "$BUILD_DIR" -j$(sysctl -n hw.ncpu)

echo "✅ Build completed!"
echo "📦 Binaries are in: $BUILD_DIR/"
ls -lh "$BUILD_DIR"/r-type_*
