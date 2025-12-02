/*
** R-Type ECS - Target Tracker
** Caches target entity position for aimed shots, homing, and AI
*/

#pragma once

#include "../../../engine/ecs/core/Registry.hpp"
#include "../../../engine/ecs/core/Types.hpp"
#include "../../../engine/ecs/components/TransformComponent.hpp"
#include "../../../engine/ecs/components/VelocityComponent.hpp"
#include "../../components/PlayerComponent.hpp"
#include <cmath>

namespace rtype::ecs {

    /**
     * @brief Tracks a target entity's position and velocity
     * Can be used for bullets tracking players, enemies tracking players,
     * effects following entities, etc.
     */
    class TargetTracker {
    public:
        void update(Registry& registry) {
            bool found = false;
            
            registry.forEach<PlayerComponent, TransformComponent>([&](EntityId playerId) {
                if (found) return; // Only track first player
                
                const auto& transform = registry.getComponent<TransformComponent>(playerId);
                m_x = transform.x;
                m_y = transform.y;
                m_entityId = playerId;

                if (registry.hasComponent<VelocityComponent>(playerId)) {
                    const auto& vel = registry.getComponent<VelocityComponent>(playerId);
                    m_velX = vel.vx;
                    m_velY = vel.vy;
                } else {
                    m_velX = 0.0f;
                    m_velY = 0.0f;
                }
                found = true;
            });
        }

        /**
         * @brief Update tracker to follow a specific entity
         */
        void updateForEntity(Registry& registry, EntityId entity) {
            if (!registry.entityExists(entity)) return;
            if (!registry.hasComponent<TransformComponent>(entity)) return;

            const auto& transform = registry.getComponent<TransformComponent>(entity);
            m_x = transform.x;
            m_y = transform.y;
            m_entityId = entity;

            if (registry.hasComponent<VelocityComponent>(entity)) {
                const auto& vel = registry.getComponent<VelocityComponent>(entity);
                m_velX = vel.vx;
                m_velY = vel.vy;
            } else {
                m_velX = 0.0f;
                m_velY = 0.0f;
            }
        }

        float getX() const { return m_x; }
        float getY() const { return m_y; }
        float getVelX() const { return m_velX; }
        float getVelY() const { return m_velY; }
        EntityId getEntityId() const { return m_entityId; }

        float getPlayerX() const { return m_x; }
        float getPlayerY() const { return m_y; }
        float getPlayerVelX() const { return m_velX; }
        float getPlayerVelY() const { return m_velY; }
        EntityId getPlayerId() const { return m_entityId; }

        void getLeadPosition(float fromX, float fromY, float projectileSpeed,
                             float& targetX, float& targetY) const {
            targetX = m_x;
            targetY = m_y;

            if (projectileSpeed > 0.0f) {
                float dx = m_x - fromX;
                float dy = m_y - fromY;
                float dist = std::sqrt(dx * dx + dy * dy);
                float travelTime = dist / projectileSpeed;
                targetX += m_velX * travelTime * 0.5f;
                targetY += m_velY * travelTime * 0.5f;
            }
        }

    private:
        float m_x = 0.0f;
        float m_y = 0.0f;
        float m_velX = 0.0f;
        float m_velY = 0.0f;
        EntityId m_entityId = NULL_ENTITY;
    };

    // Backward compatibility alias
    using PlayerTracker = TargetTracker;

} // namespace rtype::ecs
