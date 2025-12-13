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

# Raylib (fetched from GitHub)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# Don't build raylib examples
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

# Fix CMake policy issues with newer CMake versions
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

FetchContent_MakeAvailable(raylib)

# Asio (standalone, header-only networking library)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-29-0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(asio)

# Create an interface library for Asio
add_library(asio_lib INTERFACE)
target_include_directories(asio_lib INTERFACE ${asio_SOURCE_DIR}/asio/include)
target_compile_definitions(asio_lib INTERFACE ASIO_STANDALONE)

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

# Then build server and client executables
add_subdirectory(src/server)
add_subdirectory(src/client)

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
# Summary
#--------------------------------------------------------------

message(STATUS "==================================================")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary Dir: ${CMAKE_BINARY_DIR}")
message(STATUS "Binary Output: ${CMAKE_BINARY_DIR}")
message(STATUS "==================================================")

