#--------------------------------------------------------------
# RType CMake configuration file
#--------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)
project(RType VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------------------------------------
# Dependencies (FetchContent)
#--------------------------------------------------------------
include(FetchContent)

# Don't build raylib examples
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

# Prevent raylib from installing its own files
set(RAYLIB_INSTALL OFF CACHE BOOL "" FORCE)

# Fix CMake policy issues with newer CMake versions
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

# Set FetchContent properties to avoid hanging
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)

# Try to find system raylib first, then fetch if not found
find_package(raylib QUIET)
if(NOT raylib_FOUND)
    message(STATUS "System raylib not found, downloading...")
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
        TIMEOUT 60
    )
    FetchContent_MakeAvailable(raylib)
endif()

# For Asio, use header-only download
FetchContent_Declare(
    asio
    URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-28-0.tar.gz  
    TIMEOUT 60
)
FetchContent_MakeAvailable(asio)

# Create an interface library for Asio
add_library(asio_lib INTERFACE)
target_include_directories(asio_lib INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio_lib INTERFACE ASIO_STANDALONE)

# Catch2 for testing (header-only library)
FetchContent_Declare(
    catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.0.tar.gz
    TIMEOUT 60
)
FetchContent_MakeAvailable(catch2)

# nlohmann/json for configuration and serialization
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    TIMEOUT 60
)
FetchContent_MakeAvailable(nlohmann_json)

#--------------------------------------------------------------
# Default build type (if not using multi-config generator)
#--------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

#--------------------------------------------------------------
# Output directories
#--------------------------------------------------------------
# All binaries go to build/ at the project root

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# For multi-config generators
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR})
endforeach()

#--------------------------------------------------------------
# Add subdirectories
#--------------------------------------------------------------
# Build engine library first (contains ecs_engine and rtype_ui_library)
add_subdirectory(src/engine)

# Build shared library (network protocol)
add_subdirectory(src/shared)

# Then build server and client executables
add_subdirectory(src/server)
add_subdirectory(src/client)

# Build Flappy Bird game (standalone game using the engine)
add_subdirectory(src/flappybird)

#--------------------------------------------------------------
# Testing
#--------------------------------------------------------------
enable_testing()
add_subdirectory(tests)

#--------------------------------------------------------------
# Custom Rules
#--------------------------------------------------------------

add_custom_target(clean_bin
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/r-type_server* ${CMAKE_BINARY_DIR}/r-type_client*
    COMMENT "Clean: removing binaries"
)

# Safety function to prevent accidental source deletion
if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(WARNING "In-source build detected! fclean will be restricted for safety.")
    add_custom_target(fclean
        COMMAND ${CMAKE_COMMAND} -E echo "WARNING: In-source build detected"
        COMMAND ${CMAKE_COMMAND} -E echo "For safety, fclean only removes specific build artifacts"
        COMMAND ${CMAKE_COMMAND} -E remove -f CMakeCache.txt
        COMMAND ${CMAKE_COMMAND} -E remove_directory CMakeFiles
        COMMAND ${CMAKE_COMMAND} -E remove -f Makefile
        COMMAND ${CMAKE_COMMAND} -E remove -f cmake_install.cmake
        COMMAND ${CMAKE_COMMAND} -E remove -f r-type_client r-type_server
        COMMAND ${CMAKE_COMMAND} -E remove -f libecs_engine.a librtype_ui_library.a
        COMMENT "Safe clean: removing only build artifacts (in-source build protection)"
    )
else()
    add_custom_target(fclean
        COMMAND ${CMAKE_COMMAND} -E echo "Cleaning build directory: ${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/CMakeCache.txt
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/_deps
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/Makefile
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/cmake_install.cmake
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/r-type_client ${CMAKE_BINARY_DIR}/r-type_server
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/libecs_engine.a ${CMAKE_BINARY_DIR}/librtype_ui_library.a
        COMMAND ${CMAKE_COMMAND} -E echo "Build directory cleaned successfully"
        COMMENT "Full clean: removing build directory contents safely"
    )
endif()

add_custom_target(re
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMENT "Rebuilding project..."
)

#--------------------------------------------------------------
# Tests
#--------------------------------------------------------------

# Add a custom 'run_tests' target that builds and runs tests
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS rtype_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests..."
)

#--------------------------------------------------------------
# Installation / Distribution
#--------------------------------------------------------------

# Set default install prefix to a 'dist' folder in build directory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dist" CACHE PATH "Install path prefix" FORCE)
endif()

# Install binaries
install(TARGETS server client
    RUNTIME DESTINATION .
    COMPONENT Runtime
)

# Install assets
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/
    DESTINATION assets
    COMPONENT Assets
    FILES_MATCHING 
    PATTERN "*.png"
    PATTERN "*.gif"
    PATTERN "*.jpg"
    PATTERN "*.wav"
    PATTERN "*.ogg"
    PATTERN "*.mp3"
)

# Install config files
install(DIRECTORY ${CMAKE_SOURCE_DIR}/config/
    DESTINATION config
    COMPONENT Config
    FILES_MATCHING
    PATTERN "*.json"
)

# Create a convenience target for distribution
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_BINARY_DIR}/dist"
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix "${CMAKE_BINARY_DIR}/dist" --component Runtime
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix "${CMAKE_BINARY_DIR}/dist" --component Assets
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --prefix "${CMAKE_BINARY_DIR}/dist" --component Config
    COMMENT "Creating distribution package in ${CMAKE_BINARY_DIR}/dist/"
)

# CPack configuration for creating installers/archives
set(CPACK_PACKAGE_NAME "R-Type")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "R-Type Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "R-Type Multiplayer Game")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Archive generators (ZIP for Windows, TGZ for Linux/Mac)
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_GENERATOR "ZIP;TGZ")

# Explicitly specify which projects/components to install
# Format: "build_directory;project_name;component;subdirectory"
set(CPACK_INSTALL_CMAKE_PROJECTS
    "${CMAKE_BINARY_DIR};RType;Runtime;/"
    "${CMAKE_BINARY_DIR};RType;Assets;/"
    "${CMAKE_BINARY_DIR};RType;Config;/"
)

include(CPack)

#--------------------------------------------------------------
# Summary
#--------------------------------------------------------------

message(STATUS "==================================================")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary Dir: ${CMAKE_BINARY_DIR}")
message(STATUS "Binary Output: ${CMAKE_BINARY_DIR}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "==================================================")
message(STATUS "")
message(STATUS "Distribution targets:")
message(STATUS "  make dist     - Build and create dist/ folder")
message(STATUS "  make package  - Create distributable archive")
message(STATUS "==================================================")

