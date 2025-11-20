#--------------------------------------------------------------
# RType CMake configuration file
#--------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)
project(RType VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------------------------------------
# Default build type (if not using multi-config generator)
#--------------------------------------------------------------

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

#--------------------------------------------------------------
# Output directories
#--------------------------------------------------------------
# Put runtime binaries in a clear, cross-platform location inside the build dir.
# For multi-config generators (Visual Studio), create config subfolders.

set(BIN_OUT ${CMAKE_BINARY_DIR}/bin)

function(configure_output TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_OUT}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_OUT}/Debug
        RUNTIME_OUTPUT_DIRECTORY ${BIN_OUT}
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BIN_OUT}/RelWithDebInfo
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${BIN_OUT}/MinSizeRel
    )
endfunction()

#--------------------------------------------------------------
# Targets
#--------------------------------------------------------------
# Create two simple executables: server and client

add_executable(server server/main.cpp)
add_executable(client client/main.cpp)

# Preserve original binary names used across the project
set_target_properties(server PROPERTIES OUTPUT_NAME "r-type_server")
set_target_properties(client PROPERTIES OUTPUT_NAME "r-type_client")

# Apply output directory config
configure_output(server)
configure_output(client)

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
# clean target to remove only binaries
add_custom_target(clean_bin
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${BIN_OUT}
    COMMENT "Clean : removing binaries in ${BIN_OUT}"
)

# fclean target to remove build directory
add_custom_target(fclean
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}
    COMMENT "Full clean: removing build directory and binaries"
)

# re : fclean + all target to rebuild all
add_custom_target(re
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMENT "Rebuilding project..."
)

#--------------------------------------------------------------
# Summary
#--------------------------------------------------------------
message(STATUS "Project configured for C++${CMAKE_CXX_STANDARD}, binaries will be in: ${BIN_OUT}")
