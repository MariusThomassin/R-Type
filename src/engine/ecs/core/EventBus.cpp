/*
** R-Type ECS - EventBus Implementation
** Thread-safe publish-subscribe event system for decoupled communication
*/

#include "EventBus.hpp"

namespace rtype::ecs {

    void EventBus::clearAllSubscribers() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_subscribers.clear();
    }

    void EventBus::processCrossThreadEvents() {
        // Extract all queued events under lock
        std::queue<QueuedEvent> eventsToProcess;
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            std::swap(eventsToProcess, m_crossThreadQueue);
        }

        // Process each event
        while (!eventsToProcess.empty()) {
            QueuedEvent& queuedEvent = eventsToProcess.front();
            
            // Get subscribers for this event type
            std::vector<Subscriber> subscribersCopy;
            {
                std::shared_lock<std::shared_mutex> lock(m_mutex);
                auto it = m_subscribers.find(queuedEvent.typeIndex);
                if (it != m_subscribers.end()) {
                    subscribersCopy = it->second;
                }
            }

            // Dispatch to all subscribers
            for (const auto& subscriber : subscribersCopy) {
                subscriber.callback(queuedEvent.event);
            }

            eventsToProcess.pop();
        }
    }

    std::size_t EventBus::getPendingEventCount() const {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        return m_crossThreadQueue.size();
    }

} // namespace rtype::ecs