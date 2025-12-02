/*
** R-Type Client
** Main game client with ECS architecture
** Uses fixed timestep for game logic, uncapped rendering
** Event-driven input via EventBus
*/

#include <raylib.h>

#include "shared/ecs/ECS.hpp"
#include "shared/ecs/EventBus.hpp"
#include "shared/ecs/events/Events.hpp"
#include "shared/ecs/SystemManager.hpp"
#include "shared/ecs/components/Components.hpp"
#include "shared/ecs/systems/Systems.hpp"

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
    auto players = registry.getEntitiesWith<PlayerComponent, TransformComponent>();

    constexpr float MARGIN = 30.0f;

    for (EntityId entity : players) {
        auto& transform = registry.getComponent<TransformComponent>(entity);

        if (transform.x < MARGIN) transform.x = MARGIN;
        if (transform.x > SCREEN_WIDTH - MARGIN) transform.x = SCREEN_WIDTH - MARGIN;
        if (transform.y < MARGIN) transform.y = MARGIN;
        if (transform.y > SCREEN_HEIGHT - MARGIN) transform.y = SCREEN_HEIGHT - MARGIN;
    }
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
    auto* renderSystem = systems.addSystem<RenderSystem>(SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* debugSystem = systems.addSystem<DebugSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    debugSystem->setTextures(renderSystem->getTextures());
    debugSystem->init();
    
    renderSystem->setOverlayCallback([debugSystem]() {
        debugSystem->draw();
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
            systems.getSystem<MovementSystem>()->update(FIXED_TIMESTEP);
            bulletSystem->update(FIXED_TIMESTEP);
            clampPlayerToScreen(registry);
            accumulator -= FIXED_TIMESTEP;
        }

        renderSystem->update(frameTime);
    }

    // ==================== Cleanup ====================
    CloseWindow();

    return 0;
}
