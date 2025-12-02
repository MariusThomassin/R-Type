/*
** R-Type ECS - InputSystem
** Handles player input via EventBus
** Subscribes to input events from InputManager
*/

#pragma once

#include "../../engine/ecs/core/ISystem.hpp"
#include "../../engine/ecs/core/Registry.hpp"
#include "../../engine/ecs/core/EventBus.hpp"
#include "../../engine/ecs/events/InputEvents.hpp"
#include "../components/PlayerComponent.hpp"
#include "../../engine/ecs/components/VelocityComponent.hpp"
#include "../components/WeaponComponent.hpp"

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
                    if (e.key == events::KeyCode::G && !m_showoffActive) {
                        m_danmakuPressed = true;
                    }
                    if (e.key == events::KeyCode::P) {
                        m_showoffPressed = true;
                    }
                }
            );

            m_showoffStartSubId = m_eventBus.subscribe<events::ShowoffStartEvent>(
                [this](const events::ShowoffStartEvent&) {
                    m_showoffActive = true;
                }
            );
            m_showoffEndSubId = m_eventBus.subscribe<events::ShowoffEndEvent>(
                [this](const events::ShowoffEndEvent&) {
                    m_showoffActive = false;
                }
            );
        }

        ~InputSystem() override {
            m_eventBus.unsubscribe<events::KeyStateEvent>(m_keyStateSubId);
            m_eventBus.unsubscribe<events::KeyPressedEvent>(m_keyPressedSubId);
            m_eventBus.unsubscribe<events::ShowoffStartEvent>(m_showoffStartSubId);
            m_eventBus.unsubscribe<events::ShowoffEndEvent>(m_showoffEndSubId);
        }

        void update(float dt) override {
            (void)dt;

            if (!m_registry) return;

            m_registry->forEach<PlayerComponent, VelocityComponent>(
                [this](EntityId entity) {
                    const auto& player = m_registry->getComponent<PlayerComponent>(entity);

                    if (!player.isLocal) return;

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
            );

            if (m_danmakuPressed) {
                m_eventBus.emit(events::DanmakuEvent{0, 0});
                m_danmakuPressed = false;
            }

            if (m_showoffPressed) {
                if (!m_showoffActive) {
                    m_eventBus.emit(events::ShowoffStartEvent{});
                } else {
                    m_eventBus.emit(events::ShowoffEndEvent{});
                }
                m_showoffPressed = false;
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
        bool m_showoffPressed = false;
        bool m_showoffActive = false;

        EventBus::SubscriberId m_keyStateSubId;
        EventBus::SubscriberId m_keyPressedSubId;
        EventBus::SubscriberId m_showoffStartSubId;
        EventBus::SubscriberId m_showoffEndSubId;
    };

} // namespace rtype::ecs
