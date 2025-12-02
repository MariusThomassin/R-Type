/*
** R-Type ECS - Event Channel
** Type-safe event dispatch for a single event type
*/

#pragma once

#include "EventTypes.hpp"

#include <algorithm>
#include <functional>
#include <vector>

namespace rtype::ecs::events {

    /**
     * @brief Manages subscribers for a single event type
     * @tparam E The event type
     * 
     * Provides type-safe dispatch without std::any overhead.
     */
    template <typename E>
    class EventChannel {
    public:
        // Handler that receives event and context
        using Handler = std::function<void(const E&, EventContext&)>;
        
        // Simple handler (no context)
        using SimpleHandler = std::function<void(const E&)>;
        
        EventChannel() = default;
        
        /**
         * @brief Subscribe with priority and context access
         */
        SubscriptionId subscribe(Handler handler, Priority priority = Priority::Normal) {
            SubscriptionId id{m_nextId++, typeid(E).hash_code()};
            
            m_subscribers.push_back({
                id,
                static_cast<int>(priority),
                std::move(handler)
            });
            
            sortSubscribers();
            return id;
        }
        
        /**
         * @brief Subscribe with simple handler (no context)
         */
        SubscriptionId subscribe(SimpleHandler handler, Priority priority = Priority::Normal) {
            return subscribe(
                [h = std::move(handler)](const E& event, EventContext&) {
                    h(event);
                },
                priority
            );
        }
        
        /**
         * @brief Unsubscribe by ID
         */
        bool unsubscribe(SubscriptionId id) {
            auto it = std::find_if(m_subscribers.begin(), m_subscribers.end(),
                [&id](const Subscriber& sub) { return sub.id == id; });
            
            if (it != m_subscribers.end()) {
                m_subscribers.erase(it);
                return true;
            }
            return false;
        }
        
        /**
         * @brief Emit event to all subscribers
         * @return EventContext with final state
         */
        EventContext emit(const E& event) {
            EventContext ctx;
            
            for (const auto& subscriber : m_subscribers) {
                if (ctx.cancelled) break;
                
                subscriber.handler(event, ctx);
            }
            
            return ctx;
        }
        
        /**
         * @brief Emit mutable event (handlers can modify)
         */
        EventContext emitMutable(E& event) {
            EventContext ctx;
            
            for (const auto& subscriber : m_subscribers) {
                if (ctx.cancelled) break;
                
                subscriber.handler(event, ctx);
            }
            
            return ctx;
        }
        
        /**
         * @brief Get subscriber count
         */
        std::size_t size() const { return m_subscribers.size(); }
        
        /**
         * @brief Check if empty
         */
        bool empty() const { return m_subscribers.empty(); }
        
        /**
         * @brief Clear all subscribers
         */
        void clear() { m_subscribers.clear(); }
        
    private:
        struct Subscriber {
            SubscriptionId id;
            int priority;
            Handler handler;
        };
        
        void sortSubscribers() {
            std::stable_sort(m_subscribers.begin(), m_subscribers.end(),
                [](const Subscriber& a, const Subscriber& b) {
                    return a.priority > b.priority;  // Higher priority first
                });
        }
        
        std::vector<Subscriber> m_subscribers;
        std::size_t m_nextId = 1;
    };

} // namespace rtype::ecs::events
