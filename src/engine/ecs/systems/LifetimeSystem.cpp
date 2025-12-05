/*
** R-Type ECS - Lifetime System Implementation
** Handles entity lifetime updates and cleanup
*/

#include "LifetimeSystem.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "engine/ecs/components/TransformComponent.hpp"

namespace rtype::ecs {

    void LifetimeSystem::update(float dt) {
        if (!m_registry) return;

        updateLifetimes(dt);
        
        if (m_config.destroyOffscreen) {
            destroyOffscreen();
        }

        // Destroy collected expired entities
        destroyExpired();
    }

    SystemPhase LifetimeSystem::getPhase() const {
        return SystemPhase::GameLogic;
    }

    void LifetimeSystem::setOnExpired(OnEntityExpired callback) {
        m_onExpired = std::move(callback);
    }

    void LifetimeSystem::setScreenSize(int width, int height) {
        m_config.screenWidth = width;
        m_config.screenHeight = height;
    }

    void LifetimeSystem::setOffscreenMargin(float margin) {
        m_config.offscreenMargin = margin;
    }

    void LifetimeSystem::setDestroyOffscreen(bool enabled) {
        m_config.destroyOffscreen = enabled;
    }

    const std::vector<EntityId>& LifetimeSystem::getExpiredEntities() const {
        return m_expiredLastFrame;
    }

    const std::vector<EntityId>& LifetimeSystem::getOffscreenEntities() const {
        return m_offscreenLastFrame;
    }

    void LifetimeSystem::updateLifetimes(float dt) {
        m_expiredLastFrame.clear();

        // Use the new forEach API for zero-allocation iteration
        m_registry->forEach<LifetimeComponent>([&](EntityId entity) {
            auto& lifetime = m_registry->getComponent<LifetimeComponent>(entity);
            
            if (lifetime.update(dt)) {
                m_expiredLastFrame.push_back(entity);
            }
        });
    }

    void LifetimeSystem::destroyOffscreen() {
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

    void LifetimeSystem::destroyExpired() {
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

} // namespace rtype::ecs