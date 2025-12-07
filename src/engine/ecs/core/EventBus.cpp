/*
** R-Type ECS - EventBus Implementation
** Publish-subscribe event system for decoupled communication
*/

#include "EventBus.hpp"

namespace rtype::ecs {

    void EventBus::clearAllSubscribers() {
        m_subscribers.clear();
    }

} // namespace rtype::ecs