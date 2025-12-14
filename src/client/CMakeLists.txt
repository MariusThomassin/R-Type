#--------------------------------------------------------------
# Client CMake configuration
#--------------------------------------------------------------

# Game utility sources - client only (raylib dependent)
set(GAME_CLIENT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/game/systems/common/ColorPalette.cpp
    ${CMAKE_SOURCE_DIR}/src/game/systems/StressTestSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/game/systems/PatternSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/game/systems/DebugSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/game/systems/BulletSystem.cpp
    ${CMAKE_SOURCE_DIR}/src/game/systems/InputSystem.cpp
    # Graphics utilities
    ${CMAKE_SOURCE_DIR}/src/engine/graphics/RenderUtils.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/graphics/RaylibRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/graphics/SpriteRenderers.cpp
)

# Client sources
set(CLIENT_SOURCES
    main.cpp
    NetworkClient.cpp
)

# Client executable
add_executable(client ${CLIENT_SOURCES} ${GAME_CLIENT_SOURCES})

# Include paths
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR})

# Link libraries
target_link_libraries(client PRIVATE ecs_engine)
target_link_libraries(client PRIVATE rtype_shared)
target_link_libraries(client PRIVATE asio_lib)
target_link_libraries(client PRIVATE raylib)
target_link_libraries(client PRIVATE rtype_ui_library)

# Preserve original binary name
set_target_properties(client PROPERTIES OUTPUT_NAME "r-type_client")

# Compiler flags and defines
if(MSVC)
    target_compile_options(client PRIVATE /W4 /permissive-)
    target_compile_definitions(client PRIVATE NOGDI NOUSER)  # Avoid Windows GDI conflicts with Raylib
elseif(MINGW)
    target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_definitions(client PRIVATE NOGDI NOUSER)  # Avoid Windows GDI conflicts with Raylib
    target_link_libraries(client PRIVATE ws2_32)
elseif(UNIX)
    target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic)
    find_package(Threads REQUIRED)
    target_link_libraries(client PRIVATE Threads::Threads)
endif()

# Install rules
install(TARGETS client RUNTIME DESTINATION bin)
