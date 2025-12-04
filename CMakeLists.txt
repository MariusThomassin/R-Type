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
    GIT_TAG 5.0
    GIT_SHALLOW TRUE
)

# Don't build raylib examples
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

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
# ECS Engine Library (compiled sources)
#--------------------------------------------------------------

# Collect all .cpp files from the engine (non-raylib dependent)
set(ECS_ENGINE_SOURCES
    src/engine/ecs/core/src/Registry.cpp
)

# Game utility sources - shared (no raylib dependency)
set(GAME_UTILS_SOURCES
    src/game/systems/common/EasingFunctions.cpp
    src/game/systems/common/GameMath.cpp
    src/game/components/patterns/PatternFactory.cpp
    src/game/components/bullets/TrajectoryFactory.cpp
)

# Game utility sources - client only (raylib dependent)
set(GAME_CLIENT_SOURCES
    src/game/systems/common/ColorPalette.cpp
    # UI Library (raylib dependent)
    src/engine/ui/UIColor.cpp
    src/engine/ui/Widget.cpp
    src/engine/ui/UIManager.cpp
    src/engine/ui/widgets/TextWidget.cpp
    src/engine/ui/widgets/ButtonWidget.cpp
)

# Create static library for the ECS engine
add_library(ecs_engine STATIC ${ECS_ENGINE_SOURCES} ${GAME_UTILS_SOURCES})
target_include_directories(ecs_engine PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Compile with same flags
if(MSVC)
    target_compile_options(ecs_engine PRIVATE /W4 /permissive-)
elseif(MINGW OR UNIX)
    target_compile_options(ecs_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

#--------------------------------------------------------------
# Targets
#--------------------------------------------------------------

# Server sources
set(SERVER_SOURCES
    src/server/main.cpp
    src/server/Server.cpp
)

add_executable(server ${SERVER_SOURCES})
add_executable(client src/client/main.cpp ${GAME_CLIENT_SOURCES})

#--------------------------------------------------------------
# Modern UI Library
#--------------------------------------------------------------
# Collect all modern UI source files
file(GLOB_RECURSE MODERN_UI_SOURCES
    src/engine/ui/UIColor.cpp
    src/engine/ui/UIManager.cpp
    src/engine/ui/Widget.cpp
    src/engine/ui/widgets/ButtonWidget.cpp
    src/engine/ui/widgets/Label.cpp
    src/engine/ui/widgets/TextWidget.cpp
    src/engine/ui/widgets/ProgressBarWidget.cpp
    src/engine/ui/widgets/PanelWidget.cpp
)

add_library(rtype_ui_library STATIC ${MODERN_UI_SOURCES})
target_include_directories(rtype_ui_library PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(rtype_ui_library PUBLIC raylib)

# Include paths
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR})

# Link ECS engine library
target_link_libraries(server PRIVATE ecs_engine)
target_link_libraries(client PRIVATE ecs_engine)

# Link raylib to client
target_link_libraries(client PRIVATE raylib)

# Link modern UI library to client
target_link_libraries(client PRIVATE rtype_ui_library)

# Preserve original binary names
set_target_properties(server PROPERTIES OUTPUT_NAME "r-type_server")
set_target_properties(client PROPERTIES OUTPUT_NAME "r-type_client")

# Include directories
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/src/server)

# Link Asio to server
target_link_libraries(server PRIVATE asio_lib)

#--------------------------------------------------------------
# Warnings and platform-specific libraries
#--------------------------------------------------------------
# Compiler flags per toolchain to get reasonable warnings cross-platform

if(MSVC)
    # Windows MSVC
    target_compile_options(server PRIVATE /W4 /permissive-)
    target_compile_options(client PRIVATE /W4 /permissive-)
elseif(MINGW)
    # Windows cross compile (mingw): enable normal warnings
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic)

    # WinSock2 is required on Windows networking
    target_link_libraries(server PRIVATE ws2_32)
    target_link_libraries(client PRIVATE ws2_32)
elseif(UNIX)
    # Linux / macOS builds
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic)

    # Threads if needed
    find_package(Threads REQUIRED)
    target_link_libraries(server PRIVATE Threads::Threads)
    target_link_libraries(client PRIVATE Threads::Threads)
endif()

#--------------------------------------------------------------
# Install rules (optional)
#--------------------------------------------------------------
# to place executables in system paths when "make install" is used
install(TARGETS server client
        RUNTIME DESTINATION bin
)

#--------------------------------------------------------------
# Custom Rules
#--------------------------------------------------------------

add_custom_target(clean_bin
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/r-type_server* ${CMAKE_BINARY_DIR}/r-type_client*
    COMMENT "Clean: removing binaries"
)

add_custom_target(fclean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}
    COMMENT "Full clean: removing build directory"
)

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
message(STATUS "Binary Output: ${CMAKE_BINARY_DIR}")
message(STATUS "==================================================")

