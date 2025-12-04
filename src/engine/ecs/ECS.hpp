/*
** R-Type ECS - Main Include Header
** Convenience header to include all ECS components
*/

#pragma once

// Core types and interfaces
#include "core/Types.hpp"
#include "core/EntityTypes.hpp"

#include "core/IComponent.hpp"
#include "core/IComponentArray.hpp"

// Component storage implementations
#include "core/SparseSet.hpp"           // Optimized sparse set data structure
#include "core/ComponentArray.hpp"      // Component storage using SparseSet
#include "core/ComponentStorage.hpp"    // Alternative storage implementation

// Entity management
#include "core/Entity.hpp"
#include "core/EntityPool.hpp"          // Pooled entities with generations

// Query system
#include "core/View.hpp"                // Cached entity views

// System management
#include "core/ISystem.hpp"
#include "core/SystemManager.hpp"
#include "core/EventBus.hpp"

// Registries
#include "core/Registry.hpp"            // Main registry (uses EntityId)
#include "core/SafeRegistry.hpp"        // Safe registry (uses EntityHandle with generations)

// Engine systems
#include "systems/LifetimeSystem.hpp"   // Entity lifetime management

/**
 * @namespace rtype::ecs
 * @brief R-Type Entity Component System
 * 
 * A data-oriented ECS architecture for the R-Type game.
 * 
 * Key concepts:
 * - Entity: A unique identifier (just an ID)
 * - Component: Pure data attached to entities (no logic)
 * - System: Logic that operates on entities with specific components
 * - Registry: Central manager for all entities and components
 * 
 * Performance features:
 * - Sparse-set based component storage (no hash map overhead)
 * - Entity pooling with generation counters (safe references)
 * - Cached views for efficient queries (no allocation per call)
 * - Deferred entity destruction (safe during iteration)
 * 
 * Example usage:
 * @code
 * #include "ECS.hpp"
 * using namespace rtype::ecs;
 * 
 * // Define a component
 * struct PositionComponent : public IComponent {
 *     float x, y;
 *     std::string getTypeName() const override { return "Position"; }
 * };
 * 
 * // Create registry and entity
 * Registry registry;
 * Entity player = registry.createEntity();
 * 
 * // Add component
 * registry.addComponent(player, PositionComponent{100.0f, 200.0f});
 * 
 * // Query components
 * auto& pos = registry.getComponent<PositionComponent>(player);
 * @endcode
 */
