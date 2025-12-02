/*
** R-Type ECS - Event Queue
** Deferred event processing for safe dispatch
*/

#pragma once

#include <any>
#include <functional>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace rtype::ecs::events {

    /**
     * @brief Queues events for deferred processing
     * 
     * Benefits:
     * - Avoids stack overflow from cascading events
     * - Safe to emit during iteration
     * - Predictable execution order
     * - Process all events at defined sync points
     */
    class EventQueue {
    public:
        using Dispatcher = std::function<void(const std::any&)>;
        
        EventQueue() = default;
        
        /**
         * @brief Register a dispatcher for an event type
         */
        template <typename E>
        void registerDispatcher(std::function<void(const E&)> dispatcher) {
            std::type_index type(typeid(E));
            m_dispatchers[type] = [dispatcher](const std::any& event) {
                dispatcher(std::any_cast<const E&>(event));
            };
        }
        
        /**
         * @brief Queue an event for later processing
         */
        template <typename E>
        void queue(E&& event) {
            std::type_index type(typeid(std::decay_t<E>));
            m_queue.push({type, std::forward<E>(event)});
        }
        
        /**
         * @brief Queue event constructed in-place
         */
        template <typename E, typename... Args>
        void emplace(Args&&... args) {
            queue(E{std::forward<Args>(args)...});
        }
        
        /**
         * @brief Process all queued events
         * @return Number of events processed
         */
        std::size_t processAll() {
            std::size_t count = 0;
            
            while (!m_queue.empty()) {
                auto& [type, event] = m_queue.front();
                
                auto it = m_dispatchers.find(type);
                if (it != m_dispatchers.end()) {
                    it->second(event);
                }
                
                m_queue.pop();
                ++count;
            }
            
            return count;
        }
        
        /**
         * @brief Process up to N events
         * @return Number of events processed
         */
        std::size_t process(std::size_t maxEvents) {
            std::size_t count = 0;
            
            while (!m_queue.empty() && count < maxEvents) {
                auto& [type, event] = m_queue.front();
                
                auto it = m_dispatchers.find(type);
                if (it != m_dispatchers.end()) {
                    it->second(event);
                }
                
                m_queue.pop();
                ++count;
            }
            
            return count;
        }
        
        /**
         * @brief Get number of pending events
         */
        std::size_t pending() const { return m_queue.size(); }
        
        /**
         * @brief Check if queue is empty
         */
        bool empty() const { return m_queue.empty(); }
        
        /**
         * @brief Clear all pending events
         */
        void clear() {
            while (!m_queue.empty()) {
                m_queue.pop();
            }
        }
        
    private:
        struct QueuedEvent {
            std::type_index type;
            std::any event;
        };
        
        std::queue<QueuedEvent> m_queue;
        std::unordered_map<std::type_index, Dispatcher> m_dispatchers;
    };

} // namespace rtype::ecs::events
