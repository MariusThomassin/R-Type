/*
** R-Type Client
** Main game client with ECS architecture
** Uses fixed timestep for game logic, uncapped rendering
** Event-driven input via EventBus
*/

#include <raylib.h>

#include "engine/ecs/ECS.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/Events.hpp"
#include "engine/ecs/core/SystemManager.hpp"
#include "game/Components.hpp"
#include "game/Systems.hpp"

using namespace rtype::ecs;
using namespace rtype::ecs::events;

using rtype::ecs::BulletType;
using rtype::ecs::BulletColor;

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;  // 60 Hz game logic

/**
 * @brief Create the player entity with all necessary components
 */
Entity createPlayer(Registry& registry, int playerId) {
    Entity player = registry.createEntity();

    registry.addComponent(player, TransformComponent(
        150.0f,                           // x - left side
        SCREEN_HEIGHT / 2.0f,             // y - center
        0.0f,                             // rotation
        1.5f, 1.5f                        // scale
    ));

    registry.addComponent(player, VelocityComponent(0.0f, 0.0f, 400.0f));

    PlayerShipComponent ship(PlayerShipComponent::ShipStyle::Classic);
    ship.layer = 10;
    registry.addComponent(player, ship);

    registry.addComponent(player, PlayerComponent(playerId, 3));

    WeaponComponent weapon(0.15f, 10);
    weapon.projectileSpeed = 800.0f;
    registry.addComponent(player, weapon);

    return player;
}

/**
 * @brief Create the scrolling background entity
 */
Entity createBackground(Registry& registry, int screenWidth, int screenHeight) {
    Entity bg = registry.createEntity();
    
    registry.addComponent(bg, TransformComponent(0, 0));
    
    BackgroundComponent background(screenWidth, screenHeight, 200, 100.0f);
    background.layer = 0;  // Render behind everything
    registry.addComponent(bg, background);
    
    return bg;
}

/**
 * @brief Clamp player position to screen bounds
 */
void clampPlayerToScreen(Registry& registry) {
    constexpr float MARGIN = 30.0f;

    registry.forEach<PlayerComponent, TransformComponent>(
        [&registry](EntityId entity) {
            auto& transform = registry.getComponent<TransformComponent>(entity);

            if (transform.x < MARGIN) transform.x = MARGIN;
            if (transform.x > SCREEN_WIDTH - MARGIN) transform.x = SCREEN_WIDTH - MARGIN;
            if (transform.y < MARGIN) transform.y = MARGIN;
            if (transform.y > SCREEN_HEIGHT - MARGIN) transform.y = SCREEN_HEIGHT - MARGIN;
        }
    );
}

int main() {
    // ==================== Raylib Initialization ====================
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "R-Type");

    // ==================== Event Bus ====================
    EventBus eventBus;

    // ==================== Input Manager ====================
    InputManager inputManager(eventBus);

    // ==================== ECS Setup ====================
    Registry registry;
    SystemManager systems(&registry);

    auto* inputSystem = systems.addSystem<InputSystem>(eventBus, 350.0f);
    systems.addSystem<MovementSystem>();
    auto* bulletSystem = systems.addSystem<BulletSystem>(eventBus);
    bulletSystem->setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* patternSystem = systems.addSystem<PatternSystem>();
    patternSystem->setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    systems.addSystem<TrajectorySystem>();
    systems.addSystem<SpinSystem>();
    auto* showoffSystem = systems.addSystem<ShowoffSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* stressTestSystem = systems.addSystem<StressTestSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* renderSystem = systems.addSystem<RenderSystem>(SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* debugSystem = systems.addSystem<DebugSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    debugSystem->setTextures(renderSystem->getTextures());
    debugSystem->init();
    
    renderSystem->setOverlayCallback([debugSystem, showoffSystem, stressTestSystem]() {
        debugSystem->draw();
        
        // Draw showoff mode indicator
        if (showoffSystem->isActive()) {
            DrawRectangle(0, 0, 400, 60, Fade(BLACK, 0.7f));
            DrawText("SHOWOFF MODE", 10, 10, 20, YELLOW);
            DrawText(showoffSystem->getCurrentPatternName().c_str(), 10, 35, 16, WHITE);
            
            // Progress bar
            float progress = showoffSystem->getPhaseProgress();
            DrawRectangle(200, 12, 180, 16, DARKGRAY);
            DrawRectangle(200, 12, static_cast<int>(180 * progress), 16, YELLOW);
            
            // Phase counter
            char phaseText[32];
            snprintf(phaseText, sizeof(phaseText), "%d/%d", 
                     showoffSystem->getCurrentPhase() + 1, 
                     showoffSystem->getTotalPhases());
            DrawText(phaseText, 200, 35, 16, GRAY);
        }
        
        // Draw stress test mode indicator
        if (stressTestSystem->isActive()) {
            DrawRectangle(0, 0, 450, 100, Fade(RED, 0.85f));
            DrawText("STRESS TEST MODE", 10, 8, 20, WHITE);
            
            // Phase progress bar
            char phaseLabel[64];
            snprintf(phaseLabel, sizeof(phaseLabel), "Phase %d/%d (Intensity %d)", 
                     stressTestSystem->getCurrentPhase(),
                     stressTestSystem->getTotalPhases(),
                     stressTestSystem->getIntensity());
            DrawText(phaseLabel, 10, 32, 14, YELLOW);
            
            float progress = stressTestSystem->getPhaseProgress();
            DrawRectangle(10, 50, 430, 12, DARKGRAY);
            DrawRectangle(10, 50, static_cast<int>(430 * progress), 12, ORANGE);
            
            char statsText[128];
            snprintf(statsText, sizeof(statsText), "Entities: %d | Waves: %d | FPS: %d | Time: %.0fs/100s",
                     stressTestSystem->getBulletCount(),
                     stressTestSystem->getWaveCount(),
                     GetFPS(),
                     stressTestSystem->getTotalTime());
            DrawText(statsText, 10, 68, 14, LIGHTGRAY);
            
            // Phase time remaining
            float timeLeft = 20.0f - stressTestSystem->getPhaseTime();
            snprintf(statsText, sizeof(statsText), "Next phase in: %.1fs", timeLeft > 0 ? timeLeft : 0);
            DrawText(statsText, 10, 84, 12, GRAY);
        }
        
        // Draw completion message
        if (stressTestSystem->isComplete() && !stressTestSystem->getReportFilename().empty()) {
            DrawRectangle(0, 0, 500, 60, Fade(GREEN, 0.9f));
            DrawText("STRESS TEST COMPLETE!", 10, 10, 20, WHITE);
            char reportMsg[256];
            snprintf(reportMsg, sizeof(reportMsg), "Report saved: %s", 
                     stressTestSystem->getReportFilename().c_str());
            DrawText(reportMsg, 10, 35, 14, YELLOW);
        }
    });

    // ==================== Create Game Entities ====================
    Entity background = createBackground(registry, SCREEN_WIDTH, SCREEN_HEIGHT);
    (void)background;
    
    Entity player = createPlayer(registry, 1);
    (void)player;

    // ==================== Game Loop (Fixed Timestep) ====================
    float accumulator = 0.0f;

    while (!WindowShouldClose()) {
        float frameTime = GetFrameTime();
        accumulator += frameTime;

        inputManager.pollInput();

        while (accumulator >= FIXED_TIMESTEP) {
            inputSystem->update(FIXED_TIMESTEP);
            debugSystem->update(FIXED_TIMESTEP);
            showoffSystem->update(FIXED_TIMESTEP);
            stressTestSystem->update(FIXED_TIMESTEP);
            patternSystem->update(FIXED_TIMESTEP);
            systems.getSystem<TrajectorySystem>()->update(FIXED_TIMESTEP);
            systems.getSystem<SpinSystem>()->update(FIXED_TIMESTEP);
            systems.getSystem<MovementSystem>()->update(FIXED_TIMESTEP);
            bulletSystem->update(FIXED_TIMESTEP);
            clampPlayerToScreen(registry);
            accumulator -= FIXED_TIMESTEP;
        }

        // Sync mode states to debug menu
        debugSystem->updateShowoffState(
            showoffSystem->isActive(),
            showoffSystem->getCurrentPatternName(),
            showoffSystem->getCurrentPhase(),
            showoffSystem->getTotalPhases(),
            showoffSystem->getPhaseProgress()
        );
        debugSystem->updateStressTestState(
            stressTestSystem->isActive(),
            stressTestSystem->isComplete(),
            stressTestSystem->getIntensity(),
            stressTestSystem->getPhaseProgress(),
            stressTestSystem->getReportFilename()
        );

        renderSystem->update(frameTime);
    }

    // ==================== Cleanup ====================
    CloseWindow();

    return 0;
}
