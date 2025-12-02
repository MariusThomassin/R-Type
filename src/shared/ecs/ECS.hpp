/*
** R-Type ECS - Main Include Header
** Convenience header to include all ECS components
*/

#pragma once

#include "Types.hpp"

#include "IComponent.hpp"
#include "IComponentArray.hpp"
#include "ComponentArray.hpp"

#include "Entity.hpp"

#include "ISystem.hpp"
#include "SystemManager.hpp"
#include "EventBus.hpp"

#include "Registry.hpp"

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
