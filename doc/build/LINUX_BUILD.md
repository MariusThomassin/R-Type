# Building R-Type on Linux

## Prerequisites

### 1. Install Build Essentials

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++ cmake make
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake
```

### 2. Install CMake (if not already installed)

```bash
# Check CMake version
cmake --version

# If CMake is too old or not installed:
sudo apt install cmake  # Ubuntu/Debian
sudo dnf install cmake  # Fedora/RHEL
sudo pacman -S cmake    # Arch Linux
```

## Building the Project

### Using the build script

```bash
chmod +x Builder/Linux/linux_build.sh
./Builder/Linux/linux_build.sh
```

### Manual build

```bash
# Create build directory
mkdir -p build
cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Binaries are in the build/ directory
```

## Running the Application

```bash
# From project root
./build/r-type_server &
./build/r-type_client
```

## Troubleshooting

### Permission denied when running scripts

```bash
chmod +x Builder/Linux/linux_build.sh
```

### CMake not found

Install CMake using your package manager:

```bash
sudo apt install cmake      # Ubuntu/Debian
sudo dnf install cmake      # Fedora/RHEL
sudo pacman -S cmake        # Arch Linux
```

### Compiler not found

Install GCC or Clang:

```bash
# GCC
sudo apt install build-essential  # Ubuntu/Debian
sudo dnf install gcc-c++          # Fedora/RHEL
sudo pacman -S base-devel         # Arch Linux

# OR Clang (alternative)
sudo apt install clang            # Ubuntu/Debian
sudo dnf install clang            # Fedora/RHEL
sudo pacman -S clang              # Arch Linux
```

### Missing threads library

Make sure pthread is available (usually included with build-essential).

## Clean Build

To clean and rebuild:

```bash
# Remove build directory
rm -rf build

# Rebuild
./Builder/Linux/linux_build.sh
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

## Advanced Options

### Debug Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

### Custom Compiler

```bash
# Use Clang instead of GCC
cmake -S . -B build \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Specify number of build jobs

```bash
# Use 4 parallel jobs
cmake --build build -j4
```
