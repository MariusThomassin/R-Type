#--------------------------------------------------------------
# Flappy Bird CMake configuration
#--------------------------------------------------------------

# Flappy Bird sources
set(FLAPPYBIRD_SOURCES
    main.cpp
    systems/FlappyBirdSystem.cpp
)

# Flappy Bird executable
add_executable(flappybird ${FLAPPYBIRD_SOURCES})

# Include paths
target_include_directories(flappybird PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link libraries
target_link_libraries(flappybird PRIVATE ecs_engine)
target_link_libraries(flappybird PRIVATE raylib)

# Output name
set_target_properties(flappybird PROPERTIES OUTPUT_NAME "r-type_flappy")

# Compiler flags and defines
if(MSVC)
    target_compile_options(flappybird PRIVATE /W4 /permissive-)
    target_compile_definitions(flappybird PRIVATE NOGDI NOUSER)
elseif(MINGW)
    target_compile_options(flappybird PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_definitions(flappybird PRIVATE NOGDI NOUSER)
    target_link_libraries(flappybird PRIVATE ws2_32)
elseif(UNIX)
    target_compile_options(flappybird PRIVATE -Wall -Wextra -Wpedantic)
    find_package(Threads REQUIRED)
    target_link_libraries(flappybird PRIVATE Threads::Threads)
endif()
