/*
** R-Type ECS - Event Compatibility Layer
** Maps old event types to new organized event types
** This allows gradual migration from old EventBus to new EventBusNew
*/

#pragma once

#include "EventsNew.hpp"
#include "core/EventBus.hpp"  // Old event bus

namespace rtype::ecs {

    // ==================== Type Aliases for Backward Compatibility ====================

    // Old EntityCreatedEvent -> new events::EntityCreated
    using EntityCreatedEvent = events::EntityCreated;

    // Old EntityDestroyedEvent -> new events::EntityDestroyed
    using EntityDestroyedEvent = events::EntityDestroyed;

    // Old CollisionEvent -> new events::Collision
    using CollisionEvent = events::Collision;

    // Old DamageEvent -> new events::DamageDealt  
    using DamageEvent = events::DamageDealt;

    // Old DeathEvent -> new events::Death
    using DeathEvent = events::Death;

    // Old InputEvent -> we'll keep this as-is since InputEvents.hpp has more detailed input
    // The new InputAction is different, so we provide InputEventLegacy
    struct InputEventLegacy {
        int playerId;
        int keyCode;
        bool pressed;
        float mouseX;
        float mouseY;
    };

    // Old SpawnProjectileEvent -> new events::SpawnProjectile
    using SpawnProjectileEvent = events::SpawnProjectile;

    // Old PlaySoundEvent -> new events::PlaySound
    using PlaySoundEvent = events::PlaySound;

    // Old ScoreEvent -> new events::ScoreChanged
    using ScoreEvent = events::ScoreChanged;

    // ==================== Helper for Migration ====================

    /**
     * @brief Check if code is using old event bus
     * Compile-time constant for conditional compilation during migration
     */
    constexpr bool USING_LEGACY_EVENT_BUS = false;

    /**
     * @brief Bridge class to forward events from old bus to new bus
     * 
     * Usage: Create one of these and call forwardAll() to bridge systems
     */
    class EventBusBridge {
    public:
        EventBusBridge(EventBus& oldBus, events::EventBusNew& newBus)
            : m_oldBus(oldBus), m_newBus(newBus) {}

        /**
         * @brief Set up forwarding from old bus to new bus for common events
         */
        void setupForwarding() {
            // Forward entity events
            m_oldBus.subscribe<events::EntityCreated>([this](const events::EntityCreated& e) {
                m_newBus.publish(e);
            });

            m_oldBus.subscribe<events::EntityDestroyed>([this](const events::EntityDestroyed& e) {
                m_newBus.publish(e);
            });

            // Forward combat events
            m_oldBus.subscribe<events::DamageDealt>([this](const events::DamageDealt& e) {
                m_newBus.publish(e);
            });

            m_oldBus.subscribe<events::Death>([this](const events::Death& e) {
                m_newBus.publish(e);
            });

            // Forward collision events
            m_oldBus.subscribe<events::Collision>([this](const events::Collision& e) {
                m_newBus.publish(e);
            });
        }

    private:
        EventBus& m_oldBus;
        events::EventBusNew& m_newBus;
    };

} // namespace rtype::ecs
