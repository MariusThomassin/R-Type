/*
** R-Type ECS - InputSystem
** Handles player input via EventBus
** Subscribes to input events from InputManager
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/InputEvents.hpp"
#include "game/components/PlayerComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "game/components/WeaponComponent.hpp"

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
            /**
             * @brief Construct a new Input System object
             * @param eventBus Reference to EventBus for input handling
             * @param moveSpeed Movement speed for player entities
             */
            InputSystem(EventBus& eventBus, float moveSpeed = 300.0f)
                : m_eventBus(eventBus), m_moveSpeed(moveSpeed) {
                
                m_keyStateSubId = m_eventBus.subscribe<events::KeyStateEvent>(
                    [this](const events::KeyStateEvent& e) {
                        m_keyState = e.state;
                    }
                );

                m_keyPressedSubId = m_eventBus.subscribe<events::KeyPressedEvent>(
                    [this](const events::KeyPressedEvent& e) {
                        if (e.key == events::KeyCode::G && !m_showoffActive && !m_stressTestActive) {
                            m_danmakuPressed = true;
                        }
                        if (e.key == events::KeyCode::O) {
                            m_eventBus.emit(events::DebugToggleEvent{});
                        }
                        // Note: Showoff (P) and Stress Test (Shift+P) are now controlled via Debug Menu -> Modes tab
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

                m_stressTestSubId = m_eventBus.subscribe<events::StressTestToggleEvent>(
                    [this](const events::StressTestToggleEvent&) {
                        m_stressTestActive = !m_stressTestActive;
                    }
                );
            }

            /**
             * @brief Destroy the Input System object
             */
            ~InputSystem() override {
                m_eventBus.unsubscribe<events::KeyStateEvent>(m_keyStateSubId);
                m_eventBus.unsubscribe<events::KeyPressedEvent>(m_keyPressedSubId);
                m_eventBus.unsubscribe<events::ShowoffStartEvent>(m_showoffStartSubId);
                m_eventBus.unsubscribe<events::ShowoffEndEvent>(m_showoffEndSubId);
                m_eventBus.unsubscribe<events::StressTestToggleEvent>(m_stressTestSubId);
            }

            /**
             * @brief Update all player entities based on input state
             * @param dt Delta time since last update
             */
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
                // Showoff and Stress Test are now controlled via Debug Menu -> Modes tab
            }

            /**
             * @brief Get the system phase (Input)
             * @return SystemPhase
             */
            SystemPhase getPhase() const override {
                return SystemPhase::Input;
            }

            /**
             * @brief Set the movement speed for player entities
             * @param speed Movement speed
             */
            void setMoveSpeed(float speed) { m_moveSpeed = speed; }
            /**
             * @brief Get the movement speed for player entities
             * @return Movement speed
             */
            float getMoveSpeed() const { return m_moveSpeed; }

        private:
            /**
             * @brief Reference to the EventBus for subscribing/emitting events
             */
            EventBus& m_eventBus;
            /**
             * @brief Movement speed for player entities
             */
            float m_moveSpeed;

            /**
             * @brief Current key state for movement
             */
            events::KeyState m_keyState;
            /**
             * @brief Whether the danmaku key was pressed
             */
            bool m_danmakuPressed = false;
            /**
             * @brief Whether showoff mode is active
             */
            bool m_showoffActive = false;
            /**
             * @brief Whether stress test mode is active
             */
            bool m_stressTestActive = false;

            /**
             * @brief Subscriber IDs for event subscriptions
             */
            EventBus::SubscriberId m_keyStateSubId;
            /**
             * @brief Subscriber ID for key pressed event
             */
            EventBus::SubscriberId m_keyPressedSubId;
            /**
             * @brief Subscriber ID for showoff start event
             */
            EventBus::SubscriberId m_showoffStartSubId;
            /**
             * @brief Subscriber ID for showoff end event
             */
            EventBus::SubscriberId m_showoffEndSubId;
            /**
             * @brief Subscriber ID for stress test event
             */
            EventBus::SubscriberId m_stressTestSubId;
        };
} // namespace rtype::ecs
