/*
** R-Type Server - PlayerManager Implementation
*/

#include "PlayerManager.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/NetworkComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/WeaponConstants.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/bullets/TrajectoryComponent.hpp"
#include <iostream>
#include <algorithm>

namespace rtype::server {

    PlayerManager::PlayerManager(ecs::Registry& registry,
                                 NetworkManager& networkManager,
                                 NetworkIdManager& networkIdManager)
        : m_registry(registry)
        , m_networkManager(networkManager)
        , m_networkIdManager(networkIdManager)
    {
        std::cout << "[PlayerManager] Initialized" << std::endl;
    }

    ecs::Entity PlayerManager::spawnPlayer(uint32_t clientId) {
        // Check if already has a player
        if (m_clientIdToPlayerEntity.find(clientId) != m_clientIdToPlayerEntity.end()) {
            std::cout << "[PlayerManager] Client " << clientId << " already has a player" << std::endl;
            return m_clientIdToPlayerEntity[clientId];
        }

        // Check if game is full
        if (m_clientIdToPlayerEntity.size() >= MAX_PLAYERS) {
            std::cout << "[PlayerManager] Cannot spawn player: game full (" << MAX_PLAYERS << "/" << MAX_PLAYERS << ")" << std::endl;
            return ecs::Entity(ecs::NULL_ENTITY);
        }

        // Find available slot
        uint8_t slot = findAvailableSlot();
        if (slot == 255) {
            std::cout << "[PlayerManager] No available slots" << std::endl;
            return ecs::Entity(ecs::NULL_ENTITY);
        }

        // Get spawn position
        Vec2 spawnPos = getSpawnPosition(slot);

        // Create player entity
        ecs::Entity player = m_registry.createEntity();

        // Add components
        m_registry.addComponent(player, ecs::TransformComponent(spawnPos.x, spawnPos.y, 0.0f));
        m_registry.addComponent(player, ecs::VelocityComponent(0.0f, 0.0f, PLAYER_SPEED));

        ecs::PlayerComponent playerComp(slot, 3);  // slot, 3 lives
        playerComp.networkClientId = clientId;     // Store network client ID
        m_registry.addComponent(player, playerComp);

        m_registry.addComponent(player, ecs::HealthComponent(100, 100)); // 100 HP

        // Weapon component (for shooting missiles)
        ecs::WeaponComponent weapon(ecs::WeaponConstants::DEFAULT_FIRE_RATE, ecs::WeaponConstants::DEFAULT_DAMAGE);
        weapon.projectileSpeed = ecs::WeaponConstants::DEFAULT_PROJECTILE_SPEED;
        m_registry.addComponent(player, weapon);

        // Collider (approximate player ship size)
        ecs::ColliderComponent collider;
        collider.width = 32.0f;
        collider.height = 24.0f;
        collider.layer = ecs::CollisionLayer::Player;
        collider.mask = static_cast<ecs::CollisionLayer>(
            static_cast<uint32_t>(ecs::CollisionLayer::EnemyShot) |
            static_cast<uint32_t>(ecs::CollisionLayer::Enemy)
        );
        m_registry.addComponent(player, collider);

        // Allocate network ID
        uint32_t networkId = m_networkIdManager.allocate(player);
        m_registry.addComponent(player, ecs::NetworkComponent(networkId, false));

        // Store mapping
        m_clientIdToPlayerEntity[clientId] = player;

        std::cout << "[PlayerManager] Spawned player for client " << clientId
                  << " (slot=" << (int)slot << ", networkId=" << networkId << ")" << std::endl;

        // Broadcast to all clients
        network::PlayerSpawnMessage spawnMsg;
        spawnMsg.networkId = networkId;
        spawnMsg.clientId = clientId;
        spawnMsg.playerSlot = slot;
        spawnMsg.x = spawnPos.x;
        spawnMsg.y = spawnPos.y;
        spawnMsg.health = 100.0f;

        std::cout << "[PlayerManager] About to serialize and broadcast PLAYER_SPAWN..." << std::endl;
        auto buffer = network::serializeMessage(network::MessageType::PLAYER_SPAWN, spawnMsg);
        std::cout << "[PlayerManager] Serialized PLAYER_SPAWN, buffer size: " << buffer.size() << std::endl;
        m_networkManager.broadcast(buffer);
        std::cout << "[PlayerManager] Broadcasted PLAYER_SPAWN" << std::endl;

        return player;
    }

    void PlayerManager::applyInput(uint32_t clientId, const network::ClientInputMessage& input) {
        // Debug log for SHOOT input
        if (input.inputFlags & network::INPUT_SHOOT) {
            std::cout << "[PlayerManager] Received INPUT_SHOOT from client " << clientId
                      << " (flags=0x" << std::hex << (int)input.inputFlags << std::dec << ")" << std::endl;
        }

        // Find player entity
        auto it = m_clientIdToPlayerEntity.find(clientId);
        if (it == m_clientIdToPlayerEntity.end()) {
            std::cout << "[PlayerManager] WARNING: Player not found for client " << clientId << std::endl;
            return;  // Player not found
        }

        ecs::Entity player = it->second;

        // Get velocity component
        auto* velocity = m_registry.tryGetComponent<ecs::VelocityComponent>(player);
        if (!velocity) {
            return;
        }

        // Reset velocity
        velocity->vx = 0.0f;
        velocity->vy = 0.0f;

        // Apply input flags
        if (input.inputFlags & network::INPUT_UP) {
            velocity->vy = -PLAYER_SPEED;
        }
        if (input.inputFlags & network::INPUT_DOWN) {
            velocity->vy = PLAYER_SPEED;
        }
        if (input.inputFlags & network::INPUT_LEFT) {
            velocity->vx = -PLAYER_SPEED;
        }
        if (input.inputFlags & network::INPUT_RIGHT) {
            velocity->vx = PLAYER_SPEED;
        }

        // Handle simultaneous opposite inputs (cancel out)
        if ((input.inputFlags & network::INPUT_UP) && (input.inputFlags & network::INPUT_DOWN)) {
            velocity->vy = 0.0f;
        }
        if ((input.inputFlags & network::INPUT_LEFT) && (input.inputFlags & network::INPUT_RIGHT)) {
            velocity->vx = 0.0f;
        }

        // Diagonal movement normalization (prevent faster diagonal speed)
        if (velocity->vx != 0.0f && velocity->vy != 0.0f) {
            float factor = 0.7071f;  // 1/sqrt(2)
            velocity->vx *= factor;
            velocity->vy *= factor;
        }

        // Handle shooting (INPUT_SHOOT flag = 0x10)
        if (input.inputFlags & network::INPUT_SHOOT) {
            auto* weapon = m_registry.tryGetComponent<ecs::WeaponComponent>(player);
            auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(player);

            if (weapon && transform) {
                // Check weapon cooldown using game time
                if (weapon->isReady(m_gameTime)) {
                    weapon->lastFiredTime = m_gameTime;

                    // Spawn projectile
                    spawnPlayerProjectile(player, clientId, *transform, *weapon);
                }
            }
        }
    }

    void PlayerManager::removePlayer(uint32_t clientId) {
        auto it = m_clientIdToPlayerEntity.find(clientId);
        if (it == m_clientIdToPlayerEntity.end()) {
            return;  // Player not found
        }

        ecs::Entity player = it->second;

        // Get network ID before destroying
        auto* networkComp = m_registry.tryGetComponent<ecs::NetworkComponent>(player);
        uint32_t networkId = networkComp ? networkComp->networkId : 0;

        std::cout << "[PlayerManager] Removing player for client " << clientId
                  << " (networkId=" << networkId << ")" << std::endl;

        // Remove from mapping
        m_clientIdToPlayerEntity.erase(it);

        // Destroy entity
        m_registry.destroyEntity(player);

        // Remove from network ID manager
        if (networkId != 0) {
            m_networkIdManager.remove(player);

            // Broadcast destroy to clients
            m_networkManager.broadcastEntityDestroy(networkId);
        }
    }

    void PlayerManager::update(float dt) {
        m_gameTime += dt;

        // Clamp all players to screen bounds
        for (const auto& [clientId, player] : m_clientIdToPlayerEntity) {
            clampToScreen(player);
        }
    }

    ecs::Entity PlayerManager::getPlayerEntity(uint32_t clientId) const {
        auto it = m_clientIdToPlayerEntity.find(clientId);
        return (it != m_clientIdToPlayerEntity.end()) ? it->second : ecs::Entity(ecs::NULL_ENTITY);
    }

    uint8_t PlayerManager::findAvailableSlot() const {
        // Get all used slots
        std::set<uint8_t> usedSlots;
        for (const auto& [clientId, entity] : m_clientIdToPlayerEntity) {
            auto* player = m_registry.tryGetComponent<ecs::PlayerComponent>(entity);
            if (player) {
                usedSlots.insert(static_cast<uint8_t>(player->playerId));
            }
        }

        // Find first free slot (0-3)
        for (uint8_t slot = 0; slot < MAX_PLAYERS; ++slot) {
            if (usedSlots.find(slot) == usedSlots.end()) {
                return slot;
            }
        }

        return 255;  // No slot available
    }

    PlayerManager::Vec2 PlayerManager::getSpawnPosition(uint8_t slot) const {
        // Spawn positions based on slot (left side of screen, vertically spaced)
        switch (slot) {
            case 0:  // Top-left
                return {150.0f, 200.0f};
            case 1:  // Center-left
                return {150.0f, 360.0f};
            case 2:  // Bottom-left
                return {150.0f, 520.0f};
            case 3:  // Very bottom-left
                return {150.0f, SCREEN_HEIGHT - 150.0f};
            default:
                return {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};  // Center fallback
        }
    }

    void PlayerManager::clampToScreen(ecs::Entity entity) {
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
        if (!transform) {
            return;
        }

        transform->x = std::clamp(transform->x, MARGIN, SCREEN_WIDTH - MARGIN);
        transform->y = std::clamp(transform->y, MARGIN, SCREEN_HEIGHT - MARGIN);
    }

    void PlayerManager::spawnPlayerProjectile(ecs::Entity player, uint32_t clientId,
                                              const ecs::TransformComponent& transform,
                                              const ecs::WeaponComponent& weapon) {
        // Create projectile entity
        ecs::Entity projectile = m_registry.createEntity();

        // Position: spawn in front of player ship
        float offsetX = 40.0f;  // Spawn ahead of ship
        float projectileX = transform.x + offsetX;
        float projectileY = transform.y;

        m_registry.addComponent(projectile, ecs::TransformComponent(projectileX, projectileY, 0.0f));

        // Velocity: player bullets travel to the right
        float projectileSpeed = weapon.projectileSpeed;
        m_registry.addComponent(projectile, ecs::VelocityComponent(projectileSpeed, 0.0f, projectileSpeed * 1.5f));

        // Projectile component (isPlayer = true)
        m_registry.addComponent(projectile, ecs::ProjectileComponent(player, weapon.damage, true));

        // Collider for collision detection
        ecs::ColliderComponent collider;
        collider.width = 16.0f;
        collider.height = 16.0f;
        collider.layer = ecs::CollisionLayer::PlayerShot;
        collider.mask = static_cast<ecs::CollisionLayer>(
            static_cast<uint32_t>(ecs::CollisionLayer::Enemy) |
            static_cast<uint32_t>(ecs::CollisionLayer::EnemyShot) |
            static_cast<uint32_t>(ecs::CollisionLayer::Wall)
        );
        m_registry.addComponent(projectile, collider);

        // Lifetime (3 seconds)
        m_registry.addComponent(projectile, ecs::LifetimeComponent(3.0f));

        // Network component - allocate network ID
        uint32_t networkId = m_networkIdManager.allocate(projectile);
        m_registry.addComponent(projectile, ecs::NetworkComponent(networkId, false));

        // Broadcast ENTITY_SPAWN to all clients
        network::EntitySpawnMessage spawnMsg{};
        spawnMsg.networkId = networkId;
        spawnMsg.entityType = network::EntityType::PROJECTILE;
        spawnMsg.x = projectileX;
        spawnMsg.y = projectileY;
        spawnMsg.rotation = 0.0f;
        spawnMsg.vx = projectileSpeed;
        spawnMsg.vy = 0.0f;
        spawnMsg.trajectoryType = static_cast<uint8_t>(ecs::TrajectoryType::Linear);
        spawnMsg.trajectoryParam1 = 0.0f;
        spawnMsg.trajectoryParam2 = 0.0f;
        spawnMsg.spinSpeed = 0.0f;
        spawnMsg.maxLifetime = 3.0f;
        spawnMsg.colliderWidth = 16.0f;
        spawnMsg.colliderHeight = 16.0f;
        spawnMsg.collisionLayer = static_cast<uint32_t>(ecs::CollisionLayer::PlayerShot);
        spawnMsg.collisionMask = static_cast<uint32_t>(ecs::CollisionLayer::Enemy) |
                                 static_cast<uint32_t>(ecs::CollisionLayer::EnemyShot) |
                                 static_cast<uint32_t>(ecs::CollisionLayer::Wall);

        m_networkManager.broadcastEntitySpawn(spawnMsg);

        std::cout << "[PlayerManager] Player " << clientId << " fired projectile [networkId=" << networkId << "]" << std::endl;
    }

} // namespace rtype::server
