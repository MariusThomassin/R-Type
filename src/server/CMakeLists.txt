#--------------------------------------------------------------
# Server CMake configuration
#--------------------------------------------------------------

# Server sources
set(SERVER_SOURCES
    main.cpp
    Server.cpp
    GameServer.cpp
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

# Server executable
add_executable(server ${SERVER_SOURCES} ${GAME_SYSTEMS_SOURCES})

# Include paths
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/src/server)

# Add Raylib include directory (for headers only, not linking)
# This allows compilation of game systems that reference Raylib types
# but the server won't actually use any Raylib functions
target_include_directories(server PRIVATE ${CMAKE_BINARY_DIR}/_deps/raylib-src/src)

# Link ECS engine library
target_link_libraries(server PRIVATE ecs_engine)

# Link Asio to server
target_link_libraries(server PRIVATE asio_lib)

# Preserve original binary name
set_target_properties(server PROPERTIES OUTPUT_NAME "r-type_server")

# Compiler flags
if(MSVC)
    target_compile_options(server PRIVATE /W4 /permissive-)
elseif(MINGW)
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(server PRIVATE ws2_32)
elseif(UNIX)
    target_compile_options(server PRIVATE -Wall -Wextra -Wpedantic)
    find_package(Threads REQUIRED)
    target_link_libraries(server PRIVATE Threads::Threads)
endif()

# Install rules
install(TARGETS server RUNTIME DESTINATION bin)
