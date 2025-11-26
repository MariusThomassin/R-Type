# R-Type

A modern C++ implementation of the classic R-Type game using an Entity Component System (ECS) architecture.

## 📁 Project Structure

```
.
├── doc/                    # Documentation
│   ├── ECS_Architecture.puml
│   └── build/             # Build guides
│       ├── CMakeCommand.md
│       ├── LINUX_BUILD.md
│       ├── WINDOWS_BUILD.md
│       └── MACOS_BUILD.md
├── src/                   # Source code
│   ├── client/            # Client implementation
│   └── server/            # Server implementation
├── Builder/               # Build scripts
│   ├── Linux/
│   ├── Windows/
│   └── macOS/
├── cmake/                 # CMake configurations
├── sprites/               # Game assets
└── build/                 # Build output (generated)
```

## 🚀 Quick Start

### Linux
```bash
chmod +x Builder/Linux/linux_build.sh
./Builder/Linux/linux_build.sh
./build/r-type_server &
./build/r-type_client
```

### macOS
```bash
chmod +x Builder/macOS/macos_build.sh
./Builder/macOS/macos_build.sh
./build/r-type_server &
./build/r-type_client
```

### Windows (MinGW)
```bash
./Builder/Windows/windows_build_mingw-w64.sh
./build/r-type_server.exe
./build/r-type_client.exe
```

### Manual Build
```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run
./build/r-type_server &
./build/r-type_client
```

## 📖 Documentation

- **[ECS Architecture](doc/ECS_Architecture.puml)** - System design and component structure
- **[Build Guide](doc/build/CMakeCommand.md)** - Detailed build instructions
- **[Linux Setup](doc/build/LINUX_BUILD.md)** - Linux-specific setup guide
- **[Windows Setup](doc/build/WINDOWS_BUILD.md)** - Windows-specific setup guide
- **[macOS Setup](doc/build/MACOS_BUILD.md)** - macOS-specific setup guide

## 🛠️ Requirements

### All Platforms
- **CMake** 3.15 or higher
- **C++17** compatible compiler

### Linux
- GCC 7+ or Clang 5+
- make or ninja

### macOS
- Xcode Command Line Tools
- CMake (via Homebrew)

### Windows
- MinGW-w64 or MSVC 2017+

## 🏗️ Build Options

```bash
# Clean binaries only
cmake --build build --target clean_bin

# Full clean (removes build directory)
cmake --build build --target fclean

# Rebuild from scratch
cmake --build build --target re
```

## 🎮 Features

- Entity Component System (ECS) architecture
- Client-Server networking
- Cross-platform support (Linux, Windows, macOS)
- Universal Binary support for macOS (Intel + Apple Silicon)
- Client-Server networking
- Cross-platform support (Linux, Windows)

## 📝 License

[Your license here]

## 👥 Contributors

[Your team members here]
