/*
** R-Type ECS - InputSystem
** Handles player input via EventBus
** Subscribes to input events from InputManager
*/

#pragma once

#include "../core/ISystem.hpp"
#include "../core/Registry.hpp"
#include "../core/EventBus.hpp"
#include "../events/InputEvents.hpp"
#include "../../game/components/PlayerComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../../game/components/WeaponComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief System that handles player movement and actions via events
     *
     * Subscribes to KeyStateEvent for continuous input (movement)
     * and KeyPressedEvent for one-shot actions (danmaku).
     * Emits ShootEvent and DanmakuEvent for other systems to handle.
     */
    class InputSystem : public ISystem {
    public:
        InputSystem(EventBus& eventBus, float moveSpeed = 300.0f)
            : m_eventBus(eventBus), m_moveSpeed(moveSpeed) {
            
            m_keyStateSubId = m_eventBus.subscribe<events::KeyStateEvent>(
                [this](const events::KeyStateEvent& e) {
                    m_keyState = e.state;
                }
            );

            m_keyPressedSubId = m_eventBus.subscribe<events::KeyPressedEvent>(
                [this](const events::KeyPressedEvent& e) {
                    if (e.key == events::KeyCode::G) {
                        m_danmakuPressed = true;
                    }
                }
            );
        }

        ~InputSystem() override {
            m_eventBus.unsubscribe<events::KeyStateEvent>(m_keyStateSubId);
            m_eventBus.unsubscribe<events::KeyPressedEvent>(m_keyPressedSubId);
        }

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

                if (m_keyState.moveRight()) velocity.vx = m_moveSpeed;
                if (m_keyState.moveLeft()) velocity.vx = -m_moveSpeed;
                if (m_keyState.moveUp()) velocity.vy = -m_moveSpeed;
                if (m_keyState.moveDown()) velocity.vy = m_moveSpeed;

                if (velocity.vx != 0.0f && velocity.vy != 0.0f) {
                    float factor = 0.7071f;
                    velocity.vx *= factor;
                    velocity.vy *= factor;
                }

                if (m_keyState.space) {
                    m_eventBus.emit(events::ShootEvent{entity});
                }
            }

            if (m_danmakuPressed) {
                m_eventBus.emit(events::DanmakuEvent{0, 0});
                m_danmakuPressed = false;
            }
        }

        SystemPhase getPhase() const override {
            return SystemPhase::Input;
        }

        void setMoveSpeed(float speed) { m_moveSpeed = speed; }
        float getMoveSpeed() const { return m_moveSpeed; }

    private:
        EventBus& m_eventBus;
        float m_moveSpeed;
        
        events::KeyState m_keyState;
        bool m_danmakuPressed = false;

        EventBus::SubscriberId m_keyStateSubId;
        EventBus::SubscriberId m_keyPressedSubId;
    };

} // namespace rtype::ecs
