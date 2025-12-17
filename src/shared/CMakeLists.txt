#--------------------------------------------------------------
# Shared library (network protocol)
# Used by both client and server
#--------------------------------------------------------------

add_library(rtype_shared STATIC
    network/Protocol.cpp
)

# Include directories
target_include_directories(rtype_shared PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# Link ASIO (header-only, no linking needed)
target_include_directories(rtype_shared PUBLIC ${CMAKE_BINARY_DIR}/_deps/asio-src/asio/include)

# Compiler flags
if(MSVC)
    target_compile_options(rtype_shared PRIVATE /W4)
    target_compile_definitions(rtype_shared PUBLIC _WIN32_WINNT=0x0601)  # Windows 7 minimum
    target_compile_definitions(rtype_shared PUBLIC ASIO_STANDALONE)       # Use standalone ASIO
elseif(MINGW)
    target_compile_options(rtype_shared PRIVATE -Wall -Wextra)
    target_compile_definitions(rtype_shared PUBLIC ASIO_STANDALONE)       # Use standalone ASIO
    target_compile_definitions(rtype_shared PUBLIC _WIN32_WINNT=0x0601)  # Windows 7 minimum
elseif(UNIX)
    target_compile_options(rtype_shared PRIVATE -Wall -Wextra)
    target_compile_definitions(rtype_shared PUBLIC ASIO_STANDALONE)       # Use standalone ASIO
endif()
