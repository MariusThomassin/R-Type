#!/bin/bash
set -e

# --------------------------------------
# Paths (change these if your project moves)
# --------------------------------------
# Windows project path (adjust as necessary)
PROJECT_WIN_PATH="/mnt/c/Users/theag/OneDrive/Documents/GitHub/G-CPP-500-MPL-5-2-rtype-1"
TOOLCHAIN_FILE="$PROJECT_WIN_PATH/toolchain-windows.cmake"
DIST_DIR="$PROJECT_WIN_PATH/build-windows"

# --------------------------------------
# Build folder in WSL home
# --------------------------------------
BUILD_DIR="$HOME/rtype_build"

echo "Creating build directory: $BUILD_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# --------------------------------------
# Configure CMake
# --------------------------------------
echo "Configuring CMake for Windows cross-compile..."
cmake "$PROJECT_WIN_PATH" \
      -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
      -DCMAKE_BUILD_TYPE=Release \
      -G "Ninja"

# --------------------------------------
# Build project
# --------------------------------------
echo "Building project..."
ninja

# --------------------------------------
# Copy binaries to Windows folder
# --------------------------------------
echo "Copying binaries to Windows folder: $DIST_DIR"
mkdir -p "$DIST_DIR"
cp "$BUILD_DIR/bin/Release/"*.exe "$DIST_DIR/"

# --------------------------------------
# Copy MinGW runtime DLLs
# --------------------------------------
echo "Copying required MinGW DLLs..."
cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libgcc_s_seh-1.dll "$DIST_DIR/"
cp /usr/lib/gcc/x86_64-w64-mingw32/13-win32/libstdc++-6.dll "$DIST_DIR/"
cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll "$DIST_DIR/"

echo "Build complete!"
echo "Windows binaries are in: $DIST_DIR"
