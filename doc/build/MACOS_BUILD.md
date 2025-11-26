# Building R-Type on macOS

## Prerequisites

### 1. Install Xcode Command Line Tools

```bash
xcode-select --install
```

### 2. Install Homebrew (if not already installed)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. Install CMake

```bash
brew install cmake
```

## Building the Project

### Using the build script

```bash
chmod +x Builder/macOS/macos_build.sh
./Builder/macOS/macos_build.sh
```

### Manual build

```bash
# Create build directory
mkdir -p build
cd build

# Configure (Universal Binary for Intel and Apple Silicon)
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15

# Build
cmake --build . -j$(sysctl -n hw.ncpu)

# Binaries are in the build/ directory
```

## Running the Application

```bash
# From project root
./build/r-type_server &
./build/r-type_client
```

## Architecture Support

The build script creates **Universal Binaries** that work on:
- ✅ Intel Macs (x86_64)
- ✅ Apple Silicon Macs (arm64 / M1/M2/M3)

## Troubleshooting

### Permission denied when running scripts

```bash
chmod +x Builder/macOS/macos_build.sh
```

### CMake not found

Make sure CMake is in your PATH:

```bash
brew install cmake
```

### Linker errors

Make sure Xcode Command Line Tools are installed:

```bash
xcode-select --install
```

## Clean Build

To clean and rebuild:

```bash
# Remove build directory
rm -rf build

# Rebuild
./Builder/macOS/macos_build.sh
```

Or use CMake targets:

```bash
# Clean binaries only
cmake --build build --target clean_bin

# Full clean
cmake --build build --target fclean

# Rebuild from scratch
cmake --build build --target re
```
