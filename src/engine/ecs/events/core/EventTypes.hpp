/*
** R-Type ECS - Event Types
** Core types and concepts for the event system
*/

#pragma once

#include <cstdint>
#include <functional>

namespace rtype::ecs::events {

    /**
     * @brief Event handler priority levels
     * 
     * Higher priority handlers execute first.
     * Use for things like input validation before processing.
     */
    enum class Priority : int {
        Lowest  = -100,
        Low     = -50,
        Normal  = 0,
        High    = 50,
        Highest = 100,
        
        // Special priorities
        Monitor = -1000,  // Observes events, runs last, shouldn't modify
        First   = 1000,   // Always runs first (use sparingly)
    };

    /**
     * @brief Context passed to event handlers
     * 
     * Allows handlers to control event propagation.
     */
    struct EventContext {
        bool cancelled = false;      // Stop propagation to remaining handlers
        bool consumed = false;       // Mark as handled (informational)
        
        void cancel() { cancelled = true; }
        void consume() { consumed = true; }
        
        bool isCancelled() const { return cancelled; }
        bool isConsumed() const { return consumed; }
    };

    /**
     * @brief Subscription handle for unsubscribing
     */
    struct SubscriptionId {
        std::size_t id = 0;
        std::size_t typeHash = 0;
        
        bool isValid() const { return id != 0; }
        
        bool operator==(const SubscriptionId& other) const {
            return id == other.id && typeHash == other.typeHash;
        }
    };

    /**
     * @brief RAII wrapper for automatic unsubscription
     */
    class ScopedSubscription {
    public:
        using UnsubscribeFunc = std::function<void(SubscriptionId)>;
        
        ScopedSubscription() = default;
        
        ScopedSubscription(SubscriptionId id, UnsubscribeFunc unsub)
            : m_id(id), m_unsubscribe(std::move(unsub)) {}
        
        ~ScopedSubscription() {
            unsubscribe();
        }
        
        // Move only
        ScopedSubscription(ScopedSubscription&& other) noexcept
            : m_id(other.m_id), m_unsubscribe(std::move(other.m_unsubscribe)) {
            other.m_id = {};
        }
        
        ScopedSubscription& operator=(ScopedSubscription&& other) noexcept {
            if (this != &other) {
                unsubscribe();
                m_id = other.m_id;
                m_unsubscribe = std::move(other.m_unsubscribe);
                other.m_id = {};
            }
            return *this;
        }
        
        // No copy
        ScopedSubscription(const ScopedSubscription&) = delete;
        ScopedSubscription& operator=(const ScopedSubscription&) = delete;
        
        void unsubscribe() {
            if (m_id.isValid() && m_unsubscribe) {
                m_unsubscribe(m_id);
                m_id = {};
            }
        }
        
        void release() {
            m_id = {};
            m_unsubscribe = nullptr;
        }
        
        SubscriptionId getId() const { return m_id; }
        bool isActive() const { return m_id.isValid(); }
        
    private:
        SubscriptionId m_id;
        UnsubscribeFunc m_unsubscribe;
    };

} // namespace rtype::ecs::events
