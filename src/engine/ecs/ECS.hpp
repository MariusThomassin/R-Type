/*
** R-Type ECS - Main Include Header
** Convenience header to include all ECS components
*/

#pragma once

#include "core/Types.hpp"

#include "core/IComponent.hpp"
#include "core/IComponentArray.hpp"
#include "core/ComponentArray.hpp"

#include "core/Entity.hpp"

#include "core/ISystem.hpp"
#include "core/SystemManager.hpp"
#include "core/EventBus.hpp"

#include "core/Registry.hpp"

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
