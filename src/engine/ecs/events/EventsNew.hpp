/*
** R-Type ECS - Events Aggregate Header
** Includes all event system components and definitions
*/

#pragma once

// Core event system
#include "core/EventTypes.hpp"
#include "core/EventChannel.hpp"
#include "core/EventQueue.hpp"
#include "core/EventBusNew.hpp"

// Event definitions organized by domain
#include "definitions/EntityEvents.hpp"
#include "definitions/CombatEvents.hpp"
#include "definitions/CollisionEvents.hpp"
#include "definitions/GameEvents.hpp"
#include "definitions/AudioEvents.hpp"
#include "definitions/InputEvents.hpp"
#include "definitions/NetworkEvents.hpp"

namespace rtype::ecs {

    /**
     * @brief Global event bus instance access
     * 
     * Usage:
     *   // Subscribe to events
     *   events::getEventBus().subscribe<events::DamageDealt>([](const events::DamageDealt& e) {
     *       // Handle damage
     *   });
     * 
     *   // Publish events
     *   events::getEventBus().publish(events::DamageDealt{entity, 10, DamageType::Projectile});
     * 
     *   // Queue deferred events
     *   events::getEventBus().queue(events::PlayerDied{playerId});
     *   // ... later
     *   events::getEventBus().processQueue<events::PlayerDied>();
     */
    inline events::EventBusNew& getEventBus() {
        static events::EventBusNew instance;
        return instance;
    }

    /**
     * @brief Helper to get the event queue for a specific event type
     */
    template<typename EventType>
    events::EventQueue<EventType>& getEventQueue() {
        static events::EventQueue<EventType> instance;
        return instance;
    }

} // namespace rtype::ecs
