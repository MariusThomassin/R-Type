/*
** R-Type ECS - Combat Events
** Events related to combat, damage, and health
*/

#pragma once

#include "engine/ecs/core/Types.hpp"

namespace rtype::ecs::events {

    /**
     * @brief Damage types for combat
     */
    enum class DamageType {
        Normal,
        Fire,
        Ice,
        Electric,
        Explosive,
        Collision,
        Environmental
    };

    /**
     * @brief Emitted when damage is about to be dealt (cancellable)
     */
    struct DamageAttempt {
        EntityId target;
        EntityId source;
        int amount;
        DamageType type = DamageType::Normal;
        
        // Can be modified by handlers
        mutable float damageMultiplier = 1.0f;
        mutable bool blocked = false;
    };

    /**
     * @brief Emitted after damage is dealt
     */
    struct DamageDealt {
        EntityId target;
        EntityId source;
        int originalAmount;
        int finalAmount;
        int remainingHealth;
        DamageType type;
        bool wasKillingBlow;
    };

    /**
     * @brief Emitted when an entity is healed
     */
    struct Healed {
        EntityId target;
        EntityId source;  // NULL_ENTITY if self-heal or pickup
        int amount;
        int newHealth;
        int maxHealth;
    };

    /**
     * @brief Emitted when an entity's health changes
     */
    struct HealthChanged {
        EntityId entity;
        int oldHealth;
        int newHealth;
        int maxHealth;
    };

    /**
     * @brief Emitted when an entity dies
     */
    struct Death {
        EntityId entity;
        EntityId killer;  // NULL_ENTITY if no killer
        DamageType causeOfDeath = DamageType::Normal;
    };

    /**
     * @brief Emitted when an entity respawns
     */
    struct Respawn {
        EntityId entity;
        float x, y;  // Respawn position
    };

    /**
     * @brief Emitted when invincibility starts/ends
     */
    struct InvincibilityChanged {
        EntityId entity;
        bool isInvincible;
        float duration;  // 0 if ended
    };

} // namespace rtype::ecs::events
