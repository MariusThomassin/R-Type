# CMake Command

---

This document explains how to configure and build the project using **CMake**, both on **Linux** and **Windows** (including cross-compilation using MinGW).

---

## 📦 Requirements

### Linux

- CMake 3.15 or higher
- GCC or Clang
- Make or Ninja
- *(Optional)* MinGW-w64 → for cross-compiling Windows `.exe` from Linux

Install MinGW cross-compiler:

```
sudo apt install mingw-w64
```


### Windows

- CMake 3.15 or higher
- Visual Studio 2022 or MinGW-w64

> ✔ MinGW can be used on Windows
> ✔ MinGW can also be used on Linux only with the cross-compiler and a toolchain file

---

## 🛠️ Build Scripts

### Linux

You can use the `linux_build.sh` script to build the linux binaries :

```
./Builder/Linux/linux_build.sh
```

### Windows (MinGW cross-compile or native MinGW)

You can use the `windows_build_mingz-w64.sh` script to build the windows binaries :

```
./Builder/Windows/windows_build_mingz-w64.sh
```

- Works on:
    - ✔ Linux (with MinGW cross-compiler)
    - ✔ Windows (with MinGW installed)

> ⚠️ You must install MinGW-w64 before running this script.
On Linux, this is a cross-compilation toolchain.

---

### 📂 Output Binaries

Linux : `build-linux/`
Windows : `build-windows/`

---

### 🧹 Cleaning the Build Directory

Custom build rules are available inside each build directory :

| Command                           | Effect                               |
| --------------------------------- | ------------------------------------ |
| `make -C build-windows clean_bin` | Removes only compiled binaries       |
| `make -C build-windows fclean`    | Removes the *entire* build directory |
| `make -C build-windows re`        | Full rebuild (fclean + all)          |

---

Use this guide to quickly compile the project on any platform!
