#--------------------------------------------------------------
# Server CMake configuration
#--------------------------------------------------------------

# Server sources
set(SERVER_SOURCES
    main.cpp
    Server.cpp
)

# Server executable
add_executable(server ${SERVER_SOURCES})

# Include paths
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/src/server)

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
