/*
** R-Type ECS - EventBus (New Implementation)
** Type-safe, priority-based event system with queuing support
*/

#pragma once

#include "EventTypes.hpp"
#include "EventChannel.hpp"
#include "EventQueue.hpp"

#include <memory>
#include <set>
#include <typeindex>
#include <unordered_map>

namespace rtype::ecs::events {

    /**
     * @brief Central event bus with type-safe dispatch
     * 
     * Improvements over old EventBus:
     * - No std::any in hot path (type-safe channels)
     * - Priority-based handler ordering
     * - Event cancellation support
     * - Deferred event queue
     * - RAII scoped subscriptions
     */
    class EventBusNew {
    public:
        EventBusNew() = default;
        ~EventBusNew() = default;
        
        // Non-copyable
        EventBusNew(const EventBusNew&) = delete;
        EventBusNew& operator=(const EventBusNew&) = delete;
        
        // ==================== Subscription ====================
        
        /**
         * @brief Subscribe to an event type with priority
         */
        template <typename E>
        SubscriptionId subscribe(
            typename EventChannel<E>::Handler handler,
            Priority priority = Priority::Normal
        ) {
            return getChannel<E>().subscribe(std::move(handler), priority);
        }
        
        /**
         * @brief Subscribe with simple handler (no context)
         */
        template <typename E>
        SubscriptionId subscribe(
            typename EventChannel<E>::SimpleHandler handler,
            Priority priority = Priority::Normal
        ) {
            return getChannel<E>().subscribe(std::move(handler), priority);
        }
        
        /**
         * @brief Subscribe with scoped lifetime (auto-unsubscribe)
         */
        template <typename E>
        ScopedSubscription scoped(
            typename EventChannel<E>::Handler handler,
            Priority priority = Priority::Normal
        ) {
            SubscriptionId id = subscribe<E>(std::move(handler), priority);
            return ScopedSubscription(id, [this](SubscriptionId subId) {
                unsubscribe<E>(subId);
            });
        }
        
        /**
         * @brief Subscribe with scoped lifetime (simple handler)
         */
        template <typename E>
        ScopedSubscription scoped(
            typename EventChannel<E>::SimpleHandler handler,
            Priority priority = Priority::Normal
        ) {
            SubscriptionId id = subscribe<E>(std::move(handler), priority);
            return ScopedSubscription(id, [this](SubscriptionId subId) {
                unsubscribe<E>(subId);
            });
        }
        
        /**
         * @brief Unsubscribe by ID
         */
        template <typename E>
        bool unsubscribe(SubscriptionId id) {
            auto* channel = tryGetChannel<E>();
            return channel ? channel->unsubscribe(id) : false;
        }
        
        // ==================== Immediate Dispatch ====================
        
        /**
         * @brief Emit event immediately to all subscribers
         */
        template <typename E>
        EventContext emit(const E& event) {
            auto* channel = tryGetChannel<E>();
            if (channel) {
                return channel->emit(event);
            }
            return {};
        }
        
        /**
         * @brief Emit mutable event (handlers can modify)
         */
        template <typename E>
        EventContext emitMutable(E& event) {
            auto* channel = tryGetChannel<E>();
            if (channel) {
                return channel->emitMutable(event);
            }
            return {};
        }
        
        /**
         * @brief Emit event constructed in-place
         */
        template <typename E, typename... Args>
        EventContext emitEmplace(Args&&... args) {
            return emit(E{std::forward<Args>(args)...});
        }
        
        // ==================== Deferred Dispatch ====================
        
        /**
         * @brief Queue event for deferred processing
         */
        template <typename E>
        void queue(E&& event) {
            ensureDispatcher<std::decay_t<E>>();
            m_queue.queue(std::forward<E>(event));
        }
        
        /**
         * @brief Queue event constructed in-place
         */
        template <typename E, typename... Args>
        void queueEmplace(Args&&... args) {
            ensureDispatcher<E>();
            m_queue.emplace<E>(std::forward<Args>(args)...);
        }
        
        /**
         * @brief Process all queued events
         */
        std::size_t processQueue() {
            return m_queue.processAll();
        }
        
        /**
         * @brief Process up to N queued events
         */
        std::size_t processQueue(std::size_t maxEvents) {
            return m_queue.process(maxEvents);
        }
        
        /**
         * @brief Get number of pending queued events
         */
        std::size_t pendingEvents() const {
            return m_queue.pending();
        }
        
        // ==================== Utilities ====================
        
        /**
         * @brief Get subscriber count for event type
         */
        template <typename E>
        std::size_t subscriberCount() const {
            auto* channel = tryGetChannel<E>();
            return channel ? channel->size() : 0;
        }
        
        /**
         * @brief Clear subscribers for event type
         */
        template <typename E>
        void clearSubscribers() {
            auto* channel = tryGetChannel<E>();
            if (channel) {
                channel->clear();
            }
        }
        
        /**
         * @brief Clear all subscribers and queued events
         */
        void clear() {
            m_channels.clear();
            m_queue.clear();
        }
        
    private:
        template <typename E>
        EventChannel<E>& getChannel() {
            std::type_index type(typeid(E));
            auto it = m_channels.find(type);
            
            if (it == m_channels.end()) {
                auto wrapper = std::make_unique<TypedChannel<E>>();
                auto* ptr = &wrapper->channel;
                m_channels[type] = std::move(wrapper);
                return *ptr;
            }
            
            return static_cast<TypedChannel<E>*>(it->second.get())->channel;
        }
        
        template <typename E>
        EventChannel<E>* tryGetChannel() {
            std::type_index type(typeid(E));
            auto it = m_channels.find(type);
            
            if (it != m_channels.end()) {
                return &static_cast<TypedChannel<E>*>(it->second.get())->channel;
            }
            return nullptr;
        }
        
        template <typename E>
        const EventChannel<E>* tryGetChannel() const {
            std::type_index type(typeid(E));
            auto it = m_channels.find(type);
            
            if (it != m_channels.end()) {
                return &static_cast<const TypedChannel<E>*>(it->second.get())->channel;
            }
            return nullptr;
        }
        
        template <typename E>
        void ensureDispatcher() {
            std::type_index type(typeid(E));
            if (m_registeredDispatchers.find(type) == m_registeredDispatchers.end()) {
                m_queue.registerDispatcher<E>([this](const E& event) {
                    emit(event);
                });
                m_registeredDispatchers.insert(type);
            }
        }
        
        // Type-erased channel base
        struct IChannelBase {
            virtual ~IChannelBase() = default;
        };
        
        template <typename E>
        struct TypedChannel : IChannelBase {
            EventChannel<E> channel;
        };
        
        std::unordered_map<std::type_index, std::unique_ptr<IChannelBase>> m_channels;
        std::set<std::type_index> m_registeredDispatchers;
        EventQueue m_queue;
    };

} // namespace rtype::ecs::events
