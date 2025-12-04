/*
** R-Type ECS - EventBus
** Publish-subscribe event system for decoupled communication
*/

#pragma once

#include <any>
#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "Types.hpp"

namespace rtype::ecs {

    /**
     * @brief Type-safe event bus for decoupled system communication
     *
     * Allows systems to communicate without direct dependencies.
     * Events are dispatched immediately (synchronous).
     */
    class EventBus {
    public:
        using SubscriberId = std::size_t;

        EventBus() = default;
        ~EventBus() = default;

        // Prevent copying
        EventBus(const EventBus&) = delete;
        EventBus& operator=(const EventBus&) = delete;

        /**
         * @brief Subscribe to an event type
         * @tparam EventType The event type to subscribe to
         * @param callback Function to call when event is emitted
         * @return Subscriber ID for unsubscribing
         */
        template <typename EventType>
        SubscriberId subscribe(std::function<void(const EventType&)> callback) {
            std::type_index typeIndex(typeid(EventType));

            SubscriberId id = m_nextSubscriberId++;

            auto wrapper = [callback](const std::any& event) {
                callback(std::any_cast<const EventType&>(event));
            };

            m_subscribers[typeIndex].push_back({id, wrapper});

            return id;
        }

        /**
         * @brief Unsubscribe from an event type
         * @tparam EventType The event type
         * @param subscriberId The ID returned from subscribe()
         * @return true if subscriber was found and removed
         */
        template <typename EventType>
        bool unsubscribe(SubscriberId subscriberId) {
            std::type_index typeIndex(typeid(EventType));

            auto it = m_subscribers.find(typeIndex);
            if (it == m_subscribers.end()) {
                return false;
            }

            auto& subscribers = it->second;
            for (auto subIt = subscribers.begin(); subIt != subscribers.end(); ++subIt) {
                if (subIt->id == subscriberId) {
                    subscribers.erase(subIt);
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief Emit an event to all subscribers
         * @tparam EventType The event type
         * @param event The event data
         */
        template <typename EventType>
        void emit(const EventType& event) {
            std::type_index typeIndex(typeid(EventType));

            auto it = m_subscribers.find(typeIndex);
            if (it == m_subscribers.end()) {
                return;
            }

            auto subscribers = it->second;

            for (const auto& subscriber : subscribers) {
                subscriber.callback(std::any(event));
            }
        }

        /**
         * @brief Emit an event constructed in-place
         * @tparam EventType The event type
         * @tparam Args Constructor argument types
         * @param args Constructor arguments
         */
        template <typename EventType, typename... Args>
        void emitEmplace(Args&&... args) {
            emit(EventType(std::forward<Args>(args)...));
        }

        /**
         * @brief Remove all subscribers for an event type
         * @tparam EventType The event type
         */
        template <typename EventType>
        void clearSubscribers() {
            std::type_index typeIndex(typeid(EventType));
            m_subscribers.erase(typeIndex);
        }

        /**
         * @brief Remove all subscribers for all event types
         */
        void clearAllSubscribers() {
            m_subscribers.clear();
        }

        /**
         * @brief Get subscriber count for an event type
         * @tparam EventType The event type
         */
        template <typename EventType>
        std::size_t getSubscriberCount() const {
            std::type_index typeIndex(typeid(EventType));
            auto it = m_subscribers.find(typeIndex);
            if (it == m_subscribers.end()) {
                return 0;
            }
            return it->second.size();
        }

    private:
        struct Subscriber {
            SubscriberId id;
            std::function<void(const std::any&)> callback;
        };

        SubscriberId m_nextSubscriberId = 1;
        std::unordered_map<std::type_index, std::vector<Subscriber>> m_subscribers;
    };

    // ==================== Common Event Types ====================

    /**
     * @brief Event emitted when an entity is created
     */
    struct EntityCreatedEvent {
        EntityId entity;
    };

    /**
     * @brief Event emitted when an entity is destroyed
     */
    struct EntityDestroyedEvent {
        EntityId entity;
    };

    /**
     * @brief Event emitted when a collision occurs
     */
    struct CollisionEvent {
        EntityId entityA;
        EntityId entityB;
        float normalX;
        float normalY;
        float penetration;
    };

    /**
     * @brief Event emitted when damage is dealt
     */
    struct DamageEvent {
        EntityId targetId;
        EntityId sourceId;
        int damage;
        int remainingHealth;
    };

    /**
     * @brief Event emitted when an entity dies
     */
    struct DeathEvent {
        EntityId entity;
        EntityId killer;  // NULL_ENTITY if no killer
    };

    /**
     * @brief Event emitted for player input actions
     */
    struct InputEvent {
        int playerId;
        int keyCode;
        bool pressed;  // true = pressed, false = released
        float mouseX;
        float mouseY;
    };

    /**
     * @brief Event for spawning projectiles
     */
    struct SpawnProjectileEvent {
        EntityId shooter;
        float x;
        float y;
        float directionX;
        float directionY;
        float speed;
        int damage;
    };

    /**
     * @brief Event for playing sound effects
     */
    struct PlaySoundEvent {
        std::string soundId;
        float volume;
        float pitch;
    };

    /**
     * @brief Event for score changes
     */
    struct ScoreEvent {
        int playerId;
        int pointsAdded;
        int newTotal;
    };

} // namespace rtype::ecs
