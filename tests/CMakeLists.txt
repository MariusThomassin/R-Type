#--------------------------------------------------------------
# Tests CMake configuration
#--------------------------------------------------------------

# Test executable
add_executable(rtype_tests
    main.cpp
    network/ProtocolTests.cpp
    server/NetworkIdManagerTests.cpp
    server/PlayerManagerTests.cpp
    integration/FullGameLoopTests.cpp
    # Server source files (needed for tests)
    ${CMAKE_SOURCE_DIR}/src/server/NetworkManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/PlayerManager.cpp
    ${CMAKE_SOURCE_DIR}/src/server/RoomManager.cpp
)

# Include directories
target_include_directories(rtype_tests PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(rtype_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/engine)
target_include_directories(rtype_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/server)
target_include_directories(rtype_tests PRIVATE ${CMAKE_SOURCE_DIR}/src/game)

# Link libraries
target_link_libraries(rtype_tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(rtype_tests PRIVATE ecs_engine)
target_link_libraries(rtype_tests PRIVATE rtype_shared)
target_link_libraries(rtype_tests PRIVATE asio_lib)

# Compiler flags
if(MSVC)
    target_compile_options(rtype_tests PRIVATE /W4 /permissive-)
elseif(MINGW)
    target_compile_options(rtype_tests PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(rtype_tests PRIVATE ws2_32)
elseif(UNIX)
    target_compile_options(rtype_tests PRIVATE -Wall -Wextra -Wpedantic)
    find_package(Threads REQUIRED)
    target_link_libraries(rtype_tests PRIVATE Threads::Threads)
endif()

# Discover and register tests with CTest
include(CTest)
include(Catch)
catch_discover_tests(rtype_tests)
