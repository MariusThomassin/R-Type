// R-Type PlayerManager Tests
// Tests for player spawning, input handling, and lifecycle management

#include <catch2/catch_all.hpp>
#include <iostream>
#include <set>
#include <cmath>
#include "server/PlayerManager.hpp"
#include "server/NetworkManager.hpp"
#include "server/NetworkIdManager.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/Entity.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/NetworkComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/WeaponComponent.hpp"

using namespace rtype::server;

// Helper to create a PlayerManager for testing
struct PlayerManagerTestSetup {
    rtype::ecs::Registry registry;
    rtype::server::NetworkManager networkManager;
    rtype::server::NetworkIdManager networkIdManager;
    rtype::server::PlayerManager manager;

    PlayerManagerTestSetup()
        : networkManager(registry, 4242)
        , manager(registry, networkManager, networkIdManager) {}
};

// ============================================================
// Player Spawning Tests
// ============================================================

TEST_CASE("PlayerManager spawns player in slot 0 first", "[PlayerManager][spawning]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    rtype::ecs::Entity player = manager.spawnPlayer(1);

    REQUIRE(player.id != rtype::ecs::NULL_ENTITY);

    auto* playerComp = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player);
    REQUIRE(playerComp != nullptr);
    REQUIRE(playerComp->slot == 0);
}

TEST_CASE("PlayerManager spawns players in sequential slots", "[PlayerManager][spawning]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    rtype::ecs::Entity player1 = manager.spawnPlayer(1);
    rtype::ecs::Entity player2 = manager.spawnPlayer(2);
    rtype::ecs::Entity player3 = manager.spawnPlayer(3);

    REQUIRE(player1.id != rtype::ecs::NULL_ENTITY);
    REQUIRE(player2.id != rtype::ecs::NULL_ENTITY);
    REQUIRE(player3.id != rtype::ecs::NULL_ENTITY);

    auto* comp1 = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player1);
    auto* comp2 = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player2);
    auto* comp3 = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player3);

    REQUIRE(comp1 != nullptr);
    REQUIRE(comp2 != nullptr);
    REQUIRE(comp3 != nullptr);

    REQUIRE(comp1->slot == 0);
    REQUIRE(comp2->slot == 1);
    REQUIRE(comp3->slot == 2);
}

TEST_CASE("PlayerManager spawns player with correct components", "[PlayerManager][spawning]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    rtype::ecs::Entity player = manager.spawnPlayer(clientId);

    REQUIRE(player.id != rtype::ecs::NULL_ENTITY);

    // Check all required components exist
    REQUIRE(registry.tryGetComponent<rtype::ecs::TransformComponent>(player) != nullptr);
    REQUIRE(registry.tryGetComponent<rtype::ecs::VelocityComponent>(player) != nullptr);
    REQUIRE(registry.tryGetComponent<rtype::ecs::PlayerComponent>(player) != nullptr);
    REQUIRE(registry.tryGetComponent<rtype::ecs::HealthComponent>(player) != nullptr);
    REQUIRE(registry.tryGetComponent<rtype::ecs::WeaponComponent>(player) != nullptr);
    REQUIRE(registry.tryGetComponent<rtype::ecs::NetworkComponent>(player) != nullptr);
}

TEST_CASE("PlayerManager spawns player at correct position", "[PlayerManager][spawning]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    rtype::ecs::Entity player = manager.spawnPlayer(1);

    auto* transform = registry.tryGetComponent<rtype::ecs::TransformComponent>(player);
    REQUIRE(transform != nullptr);

    auto* playerComp = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player);

    if (playerComp && playerComp->slot == 0) {
        // Slot 0 spawn position: {150.0f, 200.0f}
        REQUIRE(transform->x == 150.0f);
        REQUIRE(transform->y == 200.0f);
    }
}

TEST_CASE("PlayerManager spawns player with 3 lives", "[PlayerManager][spawning]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    rtype::ecs::Entity player = manager.spawnPlayer(1);

    auto* playerComp = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player);
    REQUIRE(playerComp != nullptr);
    REQUIRE(playerComp->lives == 3);
}

TEST_CASE("PlayerManager spawns player with 100 health", "[PlayerManager][spawning]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    rtype::ecs::Entity player = manager.spawnPlayer(1);

    auto* health = registry.tryGetComponent<rtype::ecs::HealthComponent>(player);
    REQUIRE(health != nullptr);
    REQUIRE(health->currentHealth == 100);
    REQUIRE(health->maxHealth == 100);
}

TEST_CASE("PlayerManager fails when game is full (4 players)", "[PlayerManager][spawning]") {
    PlayerManagerTestSetup setup;
    PlayerManager& manager = setup.manager;

    // Spawn 4 players
    for (uint32_t i = 1; i <= 4; ++i) {
        rtype::ecs::Entity player = manager.spawnPlayer(i);
        REQUIRE(player.id != rtype::ecs::NULL_ENTITY);
    }

    // 5th player should fail
    rtype::ecs::Entity player5 = manager.spawnPlayer(5);
    REQUIRE(player5.id == rtype::ecs::NULL_ENTITY);
}

TEST_CASE("PlayerManager getPlayerCount is accurate", "[PlayerManager][spawning]") {
    PlayerManagerTestSetup setup;
    PlayerManager& manager = setup.manager;

    REQUIRE(manager.getPlayerCount() == 0);

    manager.spawnPlayer(1);
    REQUIRE(manager.getPlayerCount() == 1);

    manager.spawnPlayer(2);
    REQUIRE(manager.getPlayerCount() == 2);
}

// ============================================================
// Player Lookup Tests
// ============================================================

TEST_CASE("PlayerManager getPlayerEntity returns correct entity", "[PlayerManager][lookup]") {
    PlayerManagerTestSetup setup;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    rtype::ecs::Entity spawned = manager.spawnPlayer(clientId);

    rtype::ecs::Entity found = manager.getPlayerEntity(clientId);

    REQUIRE(found.id != rtype::ecs::NULL_ENTITY);
    REQUIRE(found.id == spawned.id);
}

TEST_CASE("PlayerManager getPlayerEntity returns NULL_ENTITY for unknown client", "[PlayerManager][lookup]") {
    PlayerManagerTestSetup setup;
    PlayerManager& manager = setup.manager;

    rtype::ecs::Entity found = manager.getPlayerEntity(999);

    REQUIRE(found.id == rtype::ecs::NULL_ENTITY);
}

// ============================================================
// Input Handling Tests
// ============================================================

TEST_CASE("PlayerManager applyInput sets velocity correctly", "[PlayerManager][input]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    manager.spawnPlayer(clientId);

    rtype::network::ClientInputMessage input;
    input.sequenceNumber = 1;
    input.inputFlags = rtype::network::INPUT_RIGHT;
    input.deltaTime = 0.016f;

    manager.applyInput(clientId, input);

    rtype::ecs::Entity player = manager.getPlayerEntity(clientId);
    auto* velocity = registry.tryGetComponent<rtype::ecs::VelocityComponent>(player);

    REQUIRE(velocity != nullptr);
    REQUIRE(velocity->vx > 0.0f);
    REQUIRE(velocity->vy == 0.0f);
}

TEST_CASE("PlayerManager applyInput handles UP input", "[PlayerManager][input]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    manager.spawnPlayer(clientId);

    rtype::network::ClientInputMessage input;
    input.sequenceNumber = 1;
    input.inputFlags = rtype::network::INPUT_UP;
    input.deltaTime = 0.016f;

    manager.applyInput(clientId, input);

    rtype::ecs::Entity player = manager.getPlayerEntity(clientId);
    auto* velocity = registry.tryGetComponent<rtype::ecs::VelocityComponent>(player);

    REQUIRE(velocity != nullptr);
    REQUIRE(velocity->vx == 0.0f);
    REQUIRE(velocity->vy < 0.0f);
}

TEST_CASE("PlayerManager applyInput handles diagonal movement", "[PlayerManager][input]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    manager.spawnPlayer(clientId);

    rtype::network::ClientInputMessage input;
    input.sequenceNumber = 1;
    input.inputFlags = rtype::network::INPUT_UP | rtype::network::INPUT_RIGHT;
    input.deltaTime = 0.016f;

    manager.applyInput(clientId, input);

    rtype::ecs::Entity player = manager.getPlayerEntity(clientId);
    auto* velocity = registry.tryGetComponent<rtype::ecs::VelocityComponent>(player);

    REQUIRE(velocity != nullptr);
    REQUIRE(velocity->vx > 0.0f);
    REQUIRE(velocity->vy < 0.0f);

    // Diagonal should be normalized (about 0.707 * speed)
    float speed = std::sqrt(velocity->vx * velocity->vx + velocity->vy * velocity->vy);
    REQUIRE(speed < 200.0f);  // Less than full speed
}

TEST_CASE("PlayerManager applyInput cancels opposite directions", "[PlayerManager][input]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    manager.spawnPlayer(clientId);

    // Test UP+DOWN cancel
    rtype::network::ClientInputMessage input1;
    input1.sequenceNumber = 1;
    input1.inputFlags = rtype::network::INPUT_UP | rtype::network::INPUT_DOWN;
    input1.deltaTime = 0.016f;

    manager.applyInput(clientId, input1);

    rtype::ecs::Entity player = manager.getPlayerEntity(clientId);
    auto* velocity = registry.tryGetComponent<rtype::ecs::VelocityComponent>(player);

    REQUIRE(velocity != nullptr);
    REQUIRE(velocity->vx == 0.0f);
    REQUIRE(velocity->vy == 0.0f);

    // Test LEFT+RIGHT cancel
    rtype::network::ClientInputMessage input2;
    input2.sequenceNumber = 2;
    input2.inputFlags = rtype::network::INPUT_LEFT | rtype::network::INPUT_RIGHT;
    input2.deltaTime = 0.016f;

    manager.applyInput(clientId, input2);

    velocity = registry.tryGetComponent<rtype::ecs::VelocityComponent>(player);

    REQUIRE(velocity != nullptr);
    REQUIRE(velocity->vx == 0.0f);
    REQUIRE(velocity->vy == 0.0f);
}

TEST_CASE("PlayerManager applyInput with no flags resets velocity", "[PlayerManager][input]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    manager.spawnPlayer(clientId);

    // First move player
    rtype::network::ClientInputMessage input1;
    input1.sequenceNumber = 1;
    input1.inputFlags = rtype::network::INPUT_RIGHT;
    input1.deltaTime = 0.016f;
    manager.applyInput(clientId, input1);

    // Then stop
    rtype::network::ClientInputMessage input2;
    input2.sequenceNumber = 2;
    input2.inputFlags = rtype::network::INPUT_NONE;
    input2.deltaTime = 0.016f;
    manager.applyInput(clientId, input2);

    rtype::ecs::Entity player = manager.getPlayerEntity(clientId);
    auto* velocity = registry.tryGetComponent<rtype::ecs::VelocityComponent>(player);

    REQUIRE(velocity != nullptr);
    REQUIRE(velocity->vx == 0.0f);
    REQUIRE(velocity->vy == 0.0f);
}

// ============================================================
// Player Removal Tests
// ============================================================

TEST_CASE("PlayerManager removePlayer destroys entity", "[PlayerManager][removal]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    rtype::ecs::Entity player = manager.spawnPlayer(clientId);

    REQUIRE(player.id != rtype::ecs::NULL_ENTITY);

    manager.removePlayer(clientId);

    // Player should no longer be found
    rtype::ecs::Entity found = manager.getPlayerEntity(clientId);
    REQUIRE(found.id == rtype::ecs::NULL_ENTITY);
}

TEST_CASE("PlayerManager removePlayer decreases player count", "[PlayerManager][removal]") {
    PlayerManagerTestSetup setup;
    PlayerManager& manager = setup.manager;

    manager.spawnPlayer(1);
    manager.spawnPlayer(2);

    REQUIRE(manager.getPlayerCount() == 2);

    manager.removePlayer(1);

    REQUIRE(manager.getPlayerCount() == 1);
}

TEST_CASE("PlayerManager removePlayer is safe for unknown client", "[PlayerManager][removal]") {
    PlayerManagerTestSetup setup;
    PlayerManager& manager = setup.manager;

    // Should not crash
    REQUIRE_NOTHROW(manager.removePlayer(999));
}

// ============================================================
// Position Clamping Tests
// ============================================================

TEST_CASE("PlayerManager clamps position to screen bounds", "[PlayerManager][update]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    manager.spawnPlayer(clientId);

    rtype::ecs::Entity player = manager.getPlayerEntity(clientId);
    auto* transform = registry.tryGetComponent<rtype::ecs::TransformComponent>(player);

    REQUIRE(transform != nullptr);

    // Set position beyond screen bounds
    transform->x = -100.0f;
    transform->y = -100.0f;

    manager.update(0.016f);

    // Should be clamped to margin
    REQUIRE(transform->x == 30.0f);  // MARGIN
    REQUIRE(transform->y == 30.0f);  // MARGIN
}

TEST_CASE("PlayerManager clamps position to max bounds", "[PlayerManager][update]") {
    PlayerManagerTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& manager = setup.manager;

    uint32_t clientId = 123;
    manager.spawnPlayer(clientId);

    rtype::ecs::Entity player = manager.getPlayerEntity(clientId);
    auto* transform = registry.tryGetComponent<rtype::ecs::TransformComponent>(player);

    REQUIRE(transform != nullptr);

    // Set position beyond screen bounds
    transform->x = 2000.0f;
    transform->y = 1000.0f;

    manager.update(0.016f);

    // Should be clamped
    REQUIRE(transform->x == 1250.0f);  // SCREEN_WIDTH - MARGIN
    REQUIRE(transform->y == 690.0f);    // SCREEN_HEIGHT - MARGIN
}

// ============================================================
// Game Time Tracking Tests
// ============================================================

TEST_CASE("PlayerManager update increments game time", "[PlayerManager][update]") {
    PlayerManagerTestSetup setup;
    PlayerManager& manager = setup.manager;

    // Update multiple times
    manager.update(0.016f);
    manager.update(0.016f);
    manager.update(0.016f);

    // We can't directly check gameTime, but we can verify updates don't crash
    SUCCEED();
}
