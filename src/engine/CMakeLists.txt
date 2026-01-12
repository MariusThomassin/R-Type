#--------------------------------------------------------------
# ECS Engine Library CMake configuration
#--------------------------------------------------------------

# Collect all .cpp files from the engine (non-raylib dependent)
set(ECS_ENGINE_SOURCES
    # Core ECS files
    ecs/core/Registry.cpp
    ecs/core/EventBus.cpp
    ecs/core/SystemManager.cpp
    ecs/core/Command.cpp
    ecs/core/Serialization.cpp
    
    # Core engine utilities
    core/ConfigManager.cpp
    core/AssetManager.cpp
    
    # ECS Systems
    ecs/systems/MovementSystem.cpp
    ecs/systems/LifetimeSystem.cpp
    
    # ECS Component utilities
    ecs/components/CollisionUtils.cpp
)

# Game utility sources - shared (no raylib dependency)
set(GAME_UTILS_SOURCES
    ${CMAKE_SOURCE_DIR}/src/game/systems/common/EasingFunctions.cpp
    ${CMAKE_SOURCE_DIR}/src/game/systems/common/GameMath.cpp
    ${CMAKE_SOURCE_DIR}/src/game/components/patterns/PatternFactory.cpp
    ${CMAKE_SOURCE_DIR}/src/game/components/bullets/TrajectoryFactory.cpp
)

# Create static library for the ECS engine
add_library(ecs_engine STATIC ${ECS_ENGINE_SOURCES} ${GAME_UTILS_SOURCES})
target_include_directories(ecs_engine PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(ecs_engine PUBLIC nlohmann_json::nlohmann_json)

# Compile with same flags
if(MSVC)
    target_compile_options(ecs_engine PRIVATE /W4 /permissive-)
elseif(MINGW OR UNIX)
    target_compile_options(ecs_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

#--------------------------------------------------------------
# Modern UI Library (raylib dependent)
#--------------------------------------------------------------
set(MODERN_UI_SOURCES
    ui/UIColor.cpp
    ui/UIManager.cpp
    ui/Widget.cpp
    ui/WindowManager.cpp
    ui/widgets/ButtonWidget.cpp
    ui/widgets/TextWidget.cpp
    ui/widgets/ProgressBarWidget.cpp
    ui/widgets/PanelWidget.cpp
    ui/widgets/WindowWidget.cpp
    ui/widgets/SliderWidget.cpp
    ui/widgets/SettingsWidget.cpp
    ui/widgets/MainMenuWidget.cpp
)

add_library(rtype_ui_library STATIC ${MODERN_UI_SOURCES})
target_include_directories(rtype_ui_library PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(rtype_ui_library PUBLIC raylib)
