# CMake Command

---

This document explains how to configure and build the project using **CMake**, on **Linux**, **Windows**, and **macOS**.

---

## 📦 Requirements

### Linux

- CMake 3.15 or higher
- GCC or Clang
- Make or Ninja
- *(Optional)* MinGW-w64 → for cross-compiling Windows `.exe` from Linux

Install MinGW cross-compiler:

```bash
sudo apt install mingw-w64
```

### macOS

- CMake 3.15 or higher
- Xcode Command Line Tools
- *(Optional)* Homebrew for package management

Install requirements:

```bash
xcode-select --install
brew install cmake
```

### Windows

- CMake 3.15 or higher
- Visual Studio 2022 or MinGW-w64

> ✔ MinGW can be used on Windows
> ✔ MinGW can also be used on Linux only with the cross-compiler and a toolchain file

---

## 🛠️ Build Scripts

### Linux

You can use the `linux_build.sh` script to build the linux binaries:

```bash
chmod +x Builder/Linux/linux_build.sh
./Builder/Linux/linux_build.sh
```

### macOS

You can use the `macos_build.sh` script to build universal binaries for macOS:

```bash
chmod +x Builder/macOS/macos_build.sh
./Builder/macOS/macos_build.sh
```

- Creates **Universal Binaries** that work on:
    - ✔ Intel Macs (x86_64)
    - ✔ Apple Silicon Macs (arm64)

### Windows (MinGW cross-compile or native MinGW)

You can use the `windows_build_mingz-w64.sh` script to build the windows binaries:

```bash
./Builder/Windows/windows_build_mingz-w64.sh
```

- Works on:
    - ✔ Linux (with MinGW cross-compiler)
    - ✔ Windows (with MinGW installed)

> ⚠️ You must install MinGW-w64 before running this script.
On Linux, this is a cross-compilation toolchain.

---

### 📂 Output Binaries

All platforms: `build/`

- Linux: `build/r-type_server`, `build/r-type_client`
- macOS: `build/r-type_server`, `build/r-type_client` (Universal)
- Windows: `build/r-type_server.exe`, `build/r-type_client.exe`

---

### 🧹 Cleaning the Build Directory

Custom build rules are available inside each build directory:

| Command                      | Effect                               |
| ---------------------------- | ------------------------------------ |
| `make -C build clean_bin`    | Removes only compiled binaries       |
| `make -C build fclean`       | Removes the *entire* build directory |
| `make -C build re`           | Full rebuild (fclean + all)          |

---

Use this guide to quickly compile the project on any platform!
