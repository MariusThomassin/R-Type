/*
** R-Type Client
** Main game client with ECS architecture
*/

#include <raylib.h>

#include "shared/ecs/ECS.hpp"
#include "shared/ecs/SystemManager.hpp"
#include "shared/ecs/components/Components.hpp"
#include "shared/ecs/systems/Systems.hpp"

using namespace rtype::ecs;

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int TARGET_FPS = 60;

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

    SpriteComponent sprite("player_ship");
    sprite.layer = 10;
    sprite.tintR = 100;
    sprite.tintG = 150;
    sprite.tintB = 255;
    registry.addComponent(player, sprite);

    registry.addComponent(player, PlayerComponent(playerId, 3));

    WeaponComponent weapon(0.15f, 10);
    weapon.projectileSpeed = 800.0f;
    registry.addComponent(player, weapon);

    return player;
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
    SetTargetFPS(TARGET_FPS);

    // ==================== ECS Setup ====================
    Registry registry;
    SystemManager systems(&registry);

    auto* inputSystem = systems.addSystem<InputSystem>(350.0f);
    systems.addSystem<MovementSystem>();
    auto* bulletSystem = systems.addSystem<BulletSystem>();
    systems.addSystem<RenderSystem>(SCREEN_WIDTH, SCREEN_HEIGHT);

    inputSystem->setShootCallback([bulletSystem](EntityId shooter) {
        bulletSystem->spawnProjectile(shooter);
    });

    // ==================== Create Game Entities ====================
    Entity player = createPlayer(registry, 1);
    (void)player;

    // ==================== Game Loop ====================
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        systems.updateAll(dt);

        clampPlayerToScreen(registry);
    }

    // ==================== Cleanup ====================
    CloseWindow();

    return 0;
}
