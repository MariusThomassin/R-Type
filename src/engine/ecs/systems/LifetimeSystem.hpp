/*
** R-Type ECS - Lifetime System
** Handles entity lifetime updates and cleanup
** Used for projectiles, effects, particles, temporary entities
*/

#pragma once

#include "../core/ISystem.hpp"
#include "../core/Registry.hpp"
#include "../components/LifetimeComponent.hpp"
#include "../components/TransformComponent.hpp"

#include <vector>
#include <functional>

namespace rtype::ecs {

    /**
     * @brief Configuration for the LifetimeSystem
     */
    struct LifetimeSystemConfig {
        int screenWidth = 1920;
        int screenHeight = 1080;
        float offscreenMargin = 50.0f;
        bool destroyOffscreen = true;
    };

    /**
     * @brief System that manages entity lifetimes and cleanup
     * 
     * Handles:
     * - Time-based lifetime expiration (LifetimeComponent)
     * - Off-screen entity destruction
     * - Optional callback for entity destruction events
     */
    class LifetimeSystem : public ISystem {
    public:
        using OnEntityExpired = std::function<void(EntityId)>;

        explicit LifetimeSystem(LifetimeSystemConfig config = {})
            : m_config(config) {}

        ~LifetimeSystem() override = default;

        void update(float dt) override {
            if (!m_registry) return;

            updateLifetimes(dt);
            
            if (m_config.destroyOffscreen) {
                destroyOffscreen();
            }

            // Destroy collected expired entities
            destroyExpired();
        }

        SystemPhase getPhase() const override {
            return SystemPhase::GameLogic;
        }

        /**
         * @brief Set callback for when entities expire
         */
        void setOnExpired(OnEntityExpired callback) {
            m_onExpired = std::move(callback);
        }

        /**
         * @brief Update screen dimensions for off-screen detection
         */
        void setScreenSize(int width, int height) {
            m_config.screenWidth = width;
            m_config.screenHeight = height;
        }

        /**
         * @brief Set the margin beyond screen edges before destruction
         */
        void setOffscreenMargin(float margin) {
            m_config.offscreenMargin = margin;
        }

        /**
         * @brief Enable/disable off-screen destruction
         */
        void setDestroyOffscreen(bool enabled) {
            m_config.destroyOffscreen = enabled;
        }

        /**
         * @brief Get entities that expired in the last update
         * Useful for spawning effects, playing sounds, etc.
         */
        const std::vector<EntityId>& getExpiredEntities() const {
            return m_expiredLastFrame;
        }

        /**
         * @brief Get entities destroyed for being off-screen last frame
         */
        const std::vector<EntityId>& getOffscreenEntities() const {
            return m_offscreenLastFrame;
        }

    private:
        void updateLifetimes(float dt) {
            m_expiredLastFrame.clear();

            // Use the new forEach API for zero-allocation iteration
            m_registry->forEach<LifetimeComponent>([&](EntityId entity) {
                auto& lifetime = m_registry->getComponent<LifetimeComponent>(entity);
                
                if (lifetime.update(dt)) {
                    m_expiredLastFrame.push_back(entity);
                }
            });
        }

        void destroyOffscreen() {
            m_offscreenLastFrame.clear();

            m_registry->forEach<TransformComponent>([&](EntityId entity) {
                const auto& transform = m_registry->getComponent<TransformComponent>(entity);

                float margin = m_config.offscreenMargin;
                if (transform.x < -margin || 
                    transform.x > m_config.screenWidth + margin ||
                    transform.y < -margin || 
                    transform.y > m_config.screenHeight + margin) {
                    m_offscreenLastFrame.push_back(entity);
                }
            });
        }

        void destroyExpired() {
            // Destroy time-expired entities
            for (EntityId entity : m_expiredLastFrame) {
                if (m_registry->entityExists(entity)) {
                    if (m_onExpired) {
                        m_onExpired(entity);
                    }
                    m_registry->destroyEntity(entity);
                }
            }

            // Destroy off-screen entities
            for (EntityId entity : m_offscreenLastFrame) {
                if (m_registry->entityExists(entity)) {
                    if (m_onExpired) {
                        m_onExpired(entity);
                    }
                    m_registry->destroyEntity(entity);
                }
            }
        }

        LifetimeSystemConfig m_config;
        OnEntityExpired m_onExpired;

        std::vector<EntityId> m_expiredLastFrame;
        std::vector<EntityId> m_offscreenLastFrame;
    };

} // namespace rtype::ecs
