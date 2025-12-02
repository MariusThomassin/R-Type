/*
** R-Type ECS - InputSystem
** Handles player input using Raylib
*/

#pragma once

#include "../ISystem.hpp"
#include "../Registry.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../components/WeaponComponent.hpp"

#include <raylib.h>
#include <functional>

namespace rtype::ecs {

    /**
     * @brief System that handles keyboard input for player movement and shooting
     *
     * Updates velocity based on arrow keys or WASD.
     * Fires weapons with Space key.
     */
    class InputSystem : public ISystem {
    public:
        using ShootCallback = std::function<void(EntityId)>;

        InputSystem(float moveSpeed = 300.0f)
            : m_moveSpeed(moveSpeed) {}

        ~InputSystem() override = default;

        void update(float dt) override {
            (void)dt;

            if (!m_registry) return;

            auto entities = m_registry->getEntitiesWith<PlayerComponent, VelocityComponent>();

            for (EntityId entity : entities) {
                const auto& player = m_registry->getComponent<PlayerComponent>(entity);

                if (!player.isLocal) continue;

                auto& velocity = m_registry->getComponent<VelocityComponent>(entity);

                velocity.vx = 0.0f;
                velocity.vy = 0.0f;

                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
                    velocity.vx = m_moveSpeed;
                }
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
                    velocity.vx = -m_moveSpeed;
                }
                if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
                    velocity.vy = -m_moveSpeed;
                }
                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
                    velocity.vy = m_moveSpeed;
                }

                if (velocity.vx != 0.0f && velocity.vy != 0.0f) {
                    float factor = 0.7071f;
                    velocity.vx *= factor;
                    velocity.vy *= factor;
                }

                if (IsKeyDown(KEY_SPACE) && m_shootCallback) {
                    m_shootCallback(entity);
                }
            }
        }

        SystemPhase getPhase() const override {
            return SystemPhase::Input;
        }

        void setMoveSpeed(float speed) { m_moveSpeed = speed; }
        float getMoveSpeed() const { return m_moveSpeed; }

        /**
         * @brief Set callback for when player fires
         * @param callback Function to call with shooter's EntityId
         */
        void setShootCallback(ShootCallback callback) {
            m_shootCallback = callback;
        }

    private:
        float m_moveSpeed;
        ShootCallback m_shootCallback;
    };

} // namespace rtype::ecs
