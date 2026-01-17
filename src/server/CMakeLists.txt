#--------------------------------------------------------------
# Server CMake configuration
#--------------------------------------------------------------

# Server core sources (for library)
set(SERVER_CORE_SOURCES
    GameServer.cpp
    NetworkManager.cpp
    PlayerManager.cpp
    RoomManager.cpp
)

# Game systems sources (shared between client and server, no Raylib dependency)
set(GAME_SYSTEMS_SOURCES
    # BulletSystem.cpp and PatternSystem.cpp removed - not needed on server
    # Both use SpritesheetComponent which causes linking errors (RenderUtils dependency)
    # Server spawns projectiles manually in GameServer::spawnDemoProjectiles()
    ${CMAKE_SOURCE_DIR}/src/game/systems/CollisionSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/game/systems/common/ColorPalette.cpp
    # Note: TrajectorySystem and SpinSystem are header-only
)

# Create a static library for server core (so client can embed LocalServer)
add_library(server_core STATIC ${SERVER_CORE_SOURCES} ${GAME_SYSTEMS_SOURCES})

target_include_directories(server_core PUBLIC ${CMAKE_SOURCE_DIR})
target_include_directories(server_core PUBLIC ${CMAKE_SOURCE_DIR}/src/server)

# Add Raylib include directory (for headers only, not linking)
# This allows compilation of game systems that reference Raylib types
# but the server won't actually use any Raylib functions
target_include_directories(server_core PUBLIC ${CMAKE_BINARY_DIR}/_deps/raylib-src/src)

target_link_libraries(server_core PUBLIC ecs_engine)
target_link_libraries(server_core PUBLIC rtype_shared)
target_link_libraries(server_core PUBLIC asio_lib)

# Compiler flags for library
if(MSVC)
    target_compile_options(server_core PRIVATE /W4 /permissive-)
    target_compile_definitions(server_core PRIVATE NOGDI NOUSER)
elseif(MINGW)
    target_compile_options(server_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_definitions(server_core PRIVATE NOGDI NOUSER)
elseif(UNIX)
    target_compile_options(server_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Server executable sources
set(SERVER_EXE_SOURCES
    main.cpp
    Server.cpp
)

# Server executable
add_executable(server ${SERVER_EXE_SOURCES})

# Include paths
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/src/server)

# Link server core library
target_link_libraries(server PRIVATE server_core)

# Preserve original binary name
set_target_properties(server PROPERTIES OUTPUT_NAME "r-type_server")

# Compiler flags and defines
if(MSVC)
    target_compile_options(server PRIVATE /W4 /permissive-)
    target_compile_definitions(server PRIVATE NOGDI NOUSER)
elseif(MINGW)
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_definitions(server PRIVATE NOGDI NOUSER)
    target_link_libraries(server PRIVATE ws2_32)
elseif(UNIX)
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
    find_package(Threads REQUIRED)
    target_link_libraries(server PRIVATE Threads::Threads)
endif()

# Install rules
install(TARGETS server RUNTIME DESTINATION bin)
