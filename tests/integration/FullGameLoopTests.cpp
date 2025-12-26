// R-Type Integration Tests
// Full game loop and component interaction tests

#include <catch2/catch_all.hpp>
#include <iostream>
#include <set>
#include "server/NetworkIdManager.hpp"
#include "server/PlayerManager.hpp"
#include "server/NetworkManager.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/Entity.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "engine/ecs/components/NetworkComponent.hpp"
#include "game/components/PlayerComponent.hpp"

using namespace rtype::server;

// Helper to create test setup
struct IntegrationTestSetup {
    rtype::ecs::Registry registry;
    rtype::server::NetworkManager networkManager;
    rtype::server::NetworkIdManager networkIdManager;
    rtype::server::PlayerManager playerManager;

    IntegrationTestSetup()
        : networkManager(registry, 4242)
        , playerManager(registry, networkManager, networkIdManager) {}
};

// ============================================================
// Basic Player Lifecycle Integration
// ============================================================

TEST_CASE("Player lifecycle: spawn → move → remove", "[integration][lifecycle]") {
    IntegrationTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& playerManager = setup.playerManager;

    // Spawn player
    uint32_t clientId = 123;
    rtype::ecs::Entity player = playerManager.spawnPlayer(clientId);

    REQUIRE(player.id != rtype::ecs::NULL_ENTITY);
    REQUIRE(playerManager.getPlayerCount() == 1);

    // Apply input (move player)
    rtype::network::ClientInputMessage input;
    input.sequenceNumber = 1;
    input.inputFlags = rtype::network::INPUT_RIGHT;
    input.deltaTime = 0.016f;
    playerManager.applyInput(clientId, input);

    // Verify player moved
    auto* velocity = registry.tryGetComponent<rtype::ecs::VelocityComponent>(player);
    REQUIRE(velocity != nullptr);
    REQUIRE(velocity->vx > 0.0f);

    // Remove player
    playerManager.removePlayer(clientId);

    REQUIRE(playerManager.getPlayerCount() == 0);
    rtype::ecs::Entity found = playerManager.getPlayerEntity(clientId);
    REQUIRE(found.id == rtype::ecs::NULL_ENTITY);
}

// ============================================================
// Multi-Player Integration
// ============================================================

TEST_CASE("Multiple players spawn in different slots", "[integration][multiplayer]") {
    IntegrationTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& playerManager = setup.playerManager;

    uint32_t clientIds[] = {100, 200, 300, 400};
    rtype::ecs::Entity players[4];

    for (int i = 0; i < 4; ++i) {
        players[i] = playerManager.spawnPlayer(clientIds[i]);
        REQUIRE(players[i].id != rtype::ecs::NULL_ENTITY);
    }

    REQUIRE(playerManager.getPlayerCount() == 4);

    // Verify each player has unique slot
    for (int i = 0; i < 4; ++i) {
        auto* playerComp = registry.tryGetComponent<rtype::ecs::PlayerComponent>(players[i]);
        REQUIRE(playerComp != nullptr);
        REQUIRE(playerComp->slot == i);
    }

    // Verify each player has unique network ID
    std::set<uint32_t> networkIds;
    for (int i = 0; i < 4; ++i) {
        auto* networkComp = registry.tryGetComponent<rtype::ecs::NetworkComponent>(players[i]);
        REQUIRE(networkComp != nullptr);
        REQUIRE(networkIds.insert(networkComp->networkId).second == true);
    }
}

// ============================================================
// Component Integration
// ============================================================

TEST_CASE("Player has all required components after spawn", "[integration][components]") {
    IntegrationTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& playerManager = setup.playerManager;

    uint32_t clientId = 123;
    rtype::ecs::Entity player = playerManager.spawnPlayer(clientId);

    REQUIRE(player.id != rtype::ecs::NULL_ENTITY);

    // Verify all expected components exist
    auto* transform = registry.tryGetComponent<rtype::ecs::TransformComponent>(player);
    auto* velocity = registry.tryGetComponent<rtype::ecs::VelocityComponent>(player);
    auto* playerComp = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player);
    auto* health = registry.tryGetComponent<rtype::ecs::HealthComponent>(player);
    auto* network = registry.tryGetComponent<rtype::ecs::NetworkComponent>(player);

    REQUIRE(transform != nullptr);
    REQUIRE(velocity != nullptr);
    REQUIRE(playerComp != nullptr);
    REQUIRE(health != nullptr);
    REQUIRE(network != nullptr);

    // Verify component values
    REQUIRE(transform->x > 0.0f);
    REQUIRE(transform->y > 0.0f);
    REQUIRE(playerComp->lives == 3);
    REQUIRE(health->currentHealth == 100);
    REQUIRE(network->networkId > 0);
}

// ============================================================
// Network ID Manager Integration
// ============================================================

TEST_CASE("NetworkIdManager integration with PlayerManager", "[integration][network]") {
    IntegrationTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    NetworkIdManager& networkIdManager = setup.networkIdManager;
    PlayerManager& playerManager = setup.playerManager;

    uint32_t clientId = 123;
    rtype::ecs::Entity player = playerManager.spawnPlayer(clientId);

    REQUIRE(player.id != rtype::ecs::NULL_ENTITY);

    auto* networkComp = registry.tryGetComponent<rtype::ecs::NetworkComponent>(player);
    REQUIRE(networkComp != nullptr);

    // Verify NetworkIdManager has this entity
    uint32_t networkId = networkIdManager.getNetworkId(player);
    REQUIRE(networkId == networkComp->networkId);

    // Verify reverse mapping
    rtype::ecs::Entity found = networkIdManager.getEntity(networkId);
    REQUIRE(found.id == player.id);
}

// ============================================================
// Health and Lives Integration
// ============================================================

TEST_CASE("Player death and lives integration", "[integration][death-system]") {
    IntegrationTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& playerManager = setup.playerManager;

    uint32_t clientId = 123;
    rtype::ecs::Entity player = playerManager.spawnPlayer(clientId);

    auto* playerComp = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player);
    auto* health = registry.tryGetComponent<rtype::ecs::HealthComponent>(player);

    REQUIRE(playerComp != nullptr);
    REQUIRE(health != nullptr);

    REQUIRE(playerComp->lives == 3);
    REQUIRE(health->currentHealth == 100);

    // Simulate death (would normally happen via collision system)
    health->currentHealth = 0;
    REQUIRE(health->isDead());

    // Lose a life
    bool hasRemainingLives = playerComp->loseLife();

    REQUIRE(hasRemainingLives == true);
    REQUIRE(playerComp->lives == 2);
}

TEST_CASE("Player with 0 lives cannot respawn", "[integration][death-system]") {
    IntegrationTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& playerManager = setup.playerManager;

    uint32_t clientId = 123;
    rtype::ecs::Entity player = playerManager.spawnPlayer(clientId);

    auto* playerComp = registry.tryGetComponent<rtype::ecs::PlayerComponent>(player);
    REQUIRE(playerComp != nullptr);

    // Lose all lives
    REQUIRE(playerComp->loseLife() == true);  // 2 lives left
    REQUIRE(playerComp->loseLife() == true);  // 1 life left
    REQUIRE(playerComp->loseLife() == true);  // 0 lives left
    REQUIRE(playerComp->lives == 0);

    // Should have no lives remaining
    REQUIRE(playerComp->loseLife() == false);
}

// ============================================================
// Position Updates Integration
// ============================================================

TEST_CASE("Player position clamps during update", "[integration][update]") {
    IntegrationTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& playerManager = setup.playerManager;

    uint32_t clientId = 123;
    playerManager.spawnPlayer(clientId);

    rtype::ecs::Entity player = playerManager.getPlayerEntity(clientId);
    auto* transform = registry.tryGetComponent<rtype::ecs::TransformComponent>(player);

    REQUIRE(transform != nullptr);

    // Move player beyond bounds
    transform->x = -1000.0f;
    transform->y = -1000.0f;

    // Update should clamp position
    playerManager.update(0.016f);

    REQUIRE(transform->x >= 30.0f);   // MARGIN
    REQUIRE(transform->y >= 30.0f);   // MARGIN
    REQUIRE(transform->x <= 1250.0f); // SCREEN_WIDTH - MARGIN
    REQUIRE(transform->y <= 690.0f);  // SCREEN_HEIGHT - MARGIN
}

// ============================================================
// Full Game Flow Simulation
// ============================================================

TEST_CASE("Simple 2-player game flow", "[integration][full-game]") {
    IntegrationTestSetup setup;
    rtype::ecs::Registry& registry = setup.registry;
    PlayerManager& playerManager = setup.playerManager;

    // Player 1 joins
    uint32_t clientId1 = 100;
    rtype::ecs::Entity player1 = playerManager.spawnPlayer(clientId1);
    REQUIRE(player1.id != rtype::ecs::NULL_ENTITY);

    // Player 2 joins
    uint32_t clientId2 = 200;
    rtype::ecs::Entity player2 = playerManager.spawnPlayer(clientId2);
    REQUIRE(player2.id != rtype::ecs::NULL_ENTITY);

    REQUIRE(playerManager.getPlayerCount() == 2);

    // Both players move
    rtype::network::ClientInputMessage input1;
    input1.sequenceNumber = 1;
    input1.inputFlags = rtype::network::INPUT_RIGHT;
    input1.deltaTime = 0.016f;
    playerManager.applyInput(clientId1, input1);

    rtype::network::ClientInputMessage input2;
    input2.sequenceNumber = 1;
    input2.inputFlags = rtype::network::INPUT_UP;
    input2.deltaTime = 0.016f;
    playerManager.applyInput(clientId2, input2);

    // Update
    playerManager.update(0.016f);

    // Verify positions
    auto* transform1 = registry.tryGetComponent<rtype::ecs::TransformComponent>(player1);
    auto* transform2 = registry.tryGetComponent<rtype::ecs::TransformComponent>(player2);

    REQUIRE(transform1 != nullptr);
    REQUIRE(transform2 != nullptr);

    // Players should have different Y positions (different slots)
    REQUIRE(transform1->y != transform2->y);
}

// ============================================================
// Stress Test
// ============================================================

TEST_CASE("Spawn and remove multiple players in sequence", "[integration][stress]") {
    IntegrationTestSetup setup;
    PlayerManager& playerManager = setup.playerManager;

    for (int i = 0; i < 10; ++i) {
        uint32_t clientId = 1000 + i;
        rtype::ecs::Entity player = playerManager.spawnPlayer(clientId);

        if (i < 4) {
            // First 4 should succeed
            REQUIRE(player.id != rtype::ecs::NULL_ENTITY);
        } else {
            // Rest should fail (game full)
            REQUIRE(player.id == rtype::ecs::NULL_ENTITY);
        }
    }

    REQUIRE(playerManager.getPlayerCount() == 4);

    // Remove all players
    playerManager.removePlayer(1000);
    playerManager.removePlayer(1001);
    playerManager.removePlayer(1002);
    playerManager.removePlayer(1003);

    REQUIRE(playerManager.getPlayerCount() == 0);

    // Can spawn again
    rtype::ecs::Entity player = playerManager.spawnPlayer(2000);
    REQUIRE(player.id != rtype::ecs::NULL_ENTITY);
}
