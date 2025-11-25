# R-Type

A modern C++ implementation of the classic R-Type game using an Entity Component System (ECS) architecture.

## 📁 Project Structure

```
.
├── doc/                    # Documentation
│   ├── ECS_Architecture.puml
│   └── build/             # Build guides
│       ├── CMakeCommand.md
│       └── WINDOWS_BUILD.md
├── src/                   # Source code
│   ├── client/            # Client implementation
│   └── server/            # Server implementation
├── Builder/               # Build scripts
│   ├── Linux/
│   └── Windows/
├── cmake/                 # CMake configurations
├── sprites/               # Game assets
└── build/                 # Build output (generated)
```

## 🚀 Quick Start

### Linux
```bash
./Builder/Linux/linux_build.sh
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
- **[Windows Setup](doc/build/WINDOWS_BUILD.md)** - Windows-specific setup guide

## 🛠️ Requirements

- **CMake** 3.15 or higher
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **MinGW-w64** (optional, for Windows cross-compilation)

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
- Cross-platform support (Linux, Windows)

## 📝 License

[Your license here]

## 👥 Contributors

[Your team members here]
