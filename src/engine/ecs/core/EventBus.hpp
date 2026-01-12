/*
** R-Type ECS - EventBus
** Thread-safe publish-subscribe event system for decoupled communication
*/

#pragma once

#include <any>
#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "Types.hpp"

namespace rtype::ecs {

    /**
     * @brief Type-safe, thread-safe event bus for decoupled system communication
     *
     * Allows systems to communicate without direct dependencies.
     * Supports both synchronous (same-thread) and cross-thread event dispatch.
     * 
     * Thread Safety:
     * - subscribe/unsubscribe: Thread-safe (uses exclusive lock)
     * - emit: Thread-safe for same-thread dispatch (uses shared lock)
     * - emitCrossThread: Thread-safe for cross-thread dispatch (uses queue)
     * - processCrossThreadEvents: Must be called from main thread
     */
    class EventBus {
    public:
        /**
         * @brief Subscriber identifier type
         */
        using SubscriberId = std::size_t;

        /**
         * @brief Construct a new Event Bus object
         */
        EventBus() = default;
        /**
         * @brief Destroy the Event Bus object
         */
        ~EventBus() = default;

        /**
         * @brief Deleted copy constructor and assignment operator
         */
        EventBus(const EventBus&) = delete;
        /**
         * @brief Deleted copy constructor and assignment operator
         */
        EventBus& operator=(const EventBus&) = delete;

        /**
         * @brief Subscribe to an event type (thread-safe)
         * @tparam EventType The event type to subscribe to
         * @param callback Function to call when event is emitted
         * @return Subscriber ID for unsubscribing
         */
        template <typename EventType>
            SubscriberId subscribe(std::function<void(const EventType&)> callback)
            {
                std::unique_lock<std::shared_mutex> lock(m_mutex);
                
                std::type_index typeIndex(typeid(EventType));

                SubscriberId id = m_nextSubscriberId++;

                auto wrapper = [callback](const std::any& event) {
                    callback(std::any_cast<const EventType&>(event));
                };

                m_subscribers[typeIndex].push_back({id, wrapper});

                return id;
            }

        /**
         * @brief Unsubscribe from an event type (thread-safe)
         * @tparam EventType The event type
         * @param subscriberId The ID returned from subscribe()
         * @return true if subscriber was found and removed
         */
        template <typename EventType>
            bool unsubscribe(SubscriberId subscriberId)
            {
                std::unique_lock<std::shared_mutex> lock(m_mutex);
                
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
         * @brief Emit an event to all subscribers (thread-safe, synchronous)
         * @tparam EventType The event type
         * @param event The event data
         * 
         * @note Uses shared lock - safe to call from any thread but callbacks
         *       run immediately on the calling thread. For cross-thread events,
         *       use emitCrossThread() instead.
         */
        template <typename EventType>
            void emit(const EventType& event)
            {
                std::vector<Subscriber> subscribersCopy;
                
                {
                    std::shared_lock<std::shared_mutex> lock(m_mutex);
                    
                    std::type_index typeIndex(typeid(EventType));

                    auto it = m_subscribers.find(typeIndex);
                    if (it == m_subscribers.end()) {
                        return;
                    }

                    // Copy subscribers to avoid holding lock during callbacks
                    subscribersCopy = it->second;
                }

                // Dispatch outside lock to prevent deadlocks
                for (const auto& subscriber : subscribersCopy) {
                    subscriber.callback(std::any(event));
                }
            }

        /**
         * @brief Queue an event for cross-thread dispatch (thread-safe)
         * @tparam EventType The event type
         * @param event The event data
         * 
         * @note Events are queued and dispatched when processCrossThreadEvents()
         *       is called from the main thread. Use this when emitting events
         *       from network/worker threads.
         */
        template <typename EventType>
            void emitCrossThread(const EventType& event)
            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                
                std::type_index typeIndex(typeid(EventType));
                m_crossThreadQueue.push({typeIndex, std::any(event)});
            }

        /**
         * @brief Process all queued cross-thread events (call from main thread)
         * 
         * Dispatches all events queued via emitCrossThread() to their subscribers.
         * Should be called once per frame from the main game loop.
         */
        void processCrossThreadEvents();

        /**
         * @brief Get the number of pending cross-thread events
         * @return Number of events in the queue
         */
        std::size_t getPendingEventCount() const;

        /**
         * @brief Emit an event constructed in-place
         * @tparam EventType The event type
         * @tparam Args Constructor argument types
         * @param args Constructor arguments
         */
        template <typename EventType, typename... Args>
            void emitEmplace(Args&&... args)
            {
                emit(EventType(std::forward<Args>(args)...));
            }

        /**
         * @brief Queue an event constructed in-place for cross-thread dispatch
         * @tparam EventType The event type
         * @tparam Args Constructor argument types
         * @param args Constructor arguments
         */
        template <typename EventType, typename... Args>
            void emitCrossThreadEmplace(Args&&... args)
            {
                emitCrossThread(EventType(std::forward<Args>(args)...));
            }

        /**
         * @brief Remove all subscribers for an event type (thread-safe)
         * @tparam EventType The event type
         */
        template <typename EventType>
            void clearSubscribers()
            {
                std::unique_lock<std::shared_mutex> lock(m_mutex);
                
                std::type_index typeIndex(typeid(EventType));
                m_subscribers.erase(typeIndex);
            }

        /**
         * @brief Remove all subscribers for all event types (thread-safe)
         */
        void clearAllSubscribers();

        /**
         * @brief Get subscriber count for an event type (thread-safe)
         * @tparam EventType The event type
         */
        template <typename EventType>
            std::size_t getSubscriberCount() const
            {
                std::shared_lock<std::shared_mutex> lock(m_mutex);
                
                std::type_index typeIndex(typeid(EventType));
                auto it = m_subscribers.find(typeIndex);
                if (it == m_subscribers.end()) {
                    return 0;
                }
                return it->second.size();
            }

    private:
        /**
         * @brief Subscriber information structure
         * 
         * Contains subscriber ID and callback function
         */
        struct Subscriber {
            SubscriberId id;
            std::function<void(const std::any&)> callback;
        };

        /**
         * @brief Queued event for cross-thread dispatch
         */
        struct QueuedEvent {
            std::type_index typeIndex;
            std::any event;
        };

        /**
         * @brief Mutex for subscriber map access (read-write lock)
         */
        mutable std::shared_mutex m_mutex;

        /**
         * @brief Mutex for cross-thread event queue
         */
        mutable std::mutex m_queueMutex;

        /**
         * @brief Queue for cross-thread events
         */
        std::queue<QueuedEvent> m_crossThreadQueue;

        /**
         * @brief Next subscriber ID to assign
         */
        std::atomic<SubscriberId> m_nextSubscriberId{1};

        /**
         * @brief Map of event type to list of subscribers
         */
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
