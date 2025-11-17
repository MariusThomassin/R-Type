cmake_minimum_required(VERSION 3.15)
project(RType VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If user didn't set a build type for single-config generators, default to Release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Create two simple executables: server and client
add_executable(server server/main.cpp)
add_executable(client client/main.cpp)

# Put runtime binaries in a clear, cross-platform location inside the build dir.
# For multi-config generators (Visual Studio), create config subfolders.
# For single-config generators (Ninja, Make), place directly in bin/
if(CMAKE_CONFIGURATION_TYPES)
    # Multi-config generator (Visual Studio, Xcode)
    set(BIN_OUT ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(server client PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_OUT}/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_OUT}/Release
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BIN_OUT}/RelWithDebInfo
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${BIN_OUT}/MinSizeRel
    )
else()
    # Single-config generator (Ninja, Unix Makefiles)
    set(BIN_OUT ${CMAKE_BINARY_DIR}/bin)
    set_target_properties(server client PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_OUT}
    )
endif()

# Preserve original binary names used across the project
# (keeps compatibility with existing scripts/expectations: r-type_server / r-type_client)
set_target_properties(server PROPERTIES OUTPUT_NAME "r-type_server")
set_target_properties(client PROPERTIES OUTPUT_NAME "r-type_client")

# Compiler flags per toolchain to get reasonable warnings cross-platform
if(MSVC)
    target_compile_options(server PRIVATE /W4 /permissive-)
    target_compile_options(client PRIVATE /W4 /permissive-)
    # Prefer secure MSVC functions? (optional)
else()
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install rules (optional) to place executables in system paths when "make install" is used
install(TARGETS server client
        RUNTIME DESTINATION bin
)

message(STATUS "Project configured for C++${CMAKE_CXX_STANDARD}, binaries will be in: ${BIN_OUT}")
