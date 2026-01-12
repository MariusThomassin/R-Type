/**
 * @file DoxygenMainPage.hpp
 * @brief Additional documentation pages for R-Type ECS Engine
 *
 * This file provides supplementary documentation pages that complement
 * the main README.md landing page.
 */

#ifndef DOXYGEN_MAINPAGE_HPP
#define DOXYGEN_MAINPAGE_HPP

/**
 * @page architecture_page Architecture Overview
 *
 * @section arch_overview Introduction
 *
 * The R-Type ECS Engine implements a high-performance Entity Component System
 * architecture for building games with raylib.
 *
 * @section features Key Features
 *
 * - **Data-Oriented Design**: Cache-friendly component storage
 * - **Flexible Entity Management**: Create, destroy, and query entities efficiently
 * - **EventBus System**: Decoupled communication between systems
 * - **raylib Integration**: Modern C graphics library for rendering
 * - **UI Widget Library**: Reusable widgets with ECS integration
 * - **Debug Tools**: Built-in profiling and inspection tools
 *
 * @section architecture Architecture Overview
 *
 * @subsection ecs_arch ECS Pattern
 *
 * The engine follows the ECS pattern:
 *
 * | Concept | Description |
 * |---------|-------------|
 * | Entity | Lightweight identifier (uint32_t) |
 * | Component | Pure data struct (Position, Velocity, Sprite) |
 * | System | Logic operating on component sets |
 * | Registry | Central entity/component manager |
 * | EventBus | Publish-subscribe event system |
 *
 * @subsection modules Module Structure
 *
 * @code
 * src/
 * ├── engine/
 * │   ├── ecs/          # Core ECS (Registry, EventBus, Types)
 * │   ├── graphics/     # Rendering and window management
 * │   └── ui/           # Widget library
 * ├── game/
 * │   ├── components/   # Game-specific components
 * │   └── systems/      # Game-specific systems
 * ├── client/           # Client executable
 * └── server/           # Server executable
 * @endcode
 *
 * @section quickstart Quick Start
 *
 * @subsection create_entity Creating an Entity
 *
 * @code{.cpp}
 * #include "src/engine/ecs/core/Registry.hpp"
 *
 * rtype::ecs::Registry registry;
 *
 * // Create entity with components
 * auto entity = registry.createEntity();
 * registry.addComponent<Position>(entity, 100.0f, 200.0f);
 * registry.addComponent<Velocity>(entity, 5.0f, 0.0f);
 * registry.addComponent<Sprite>(entity, "player.png");
 * @endcode
 *
 * @subsection query_entities Querying Entities
 *
 * @code{.cpp}
 * // Get all entities with Position and Velocity
 * auto view = registry.view<Position, Velocity>();
 *
 * for (auto entity : view) {
 *     auto& pos = registry.getComponent<Position>(entity);
 *     auto& vel = registry.getComponent<Velocity>(entity);
 *     pos.x += vel.x * deltaTime;
 *     pos.y += vel.y * deltaTime;
 * }
 * @endcode
 *
 * @subsection use_events Using Events
 *
 * @code{.cpp}
 * #include "src/engine/ecs/core/EventBus.hpp"
 *
 * // Subscribe to events
 * auto subId = eventBus.subscribe<CollisionEvent>(
 *     [](const CollisionEvent& e) {
 *         std::cout << "Collision between " << e.entityA << " and " << e.entityB;
 *     }
 * );
 *
 * // Publish events
 * eventBus.publish(CollisionEvent{playerEntity, enemyEntity});
 *
 * // Unsubscribe when done
 * eventBus.unsubscribe<CollisionEvent>(subId);
 * @endcode
 *
 * @section building Building
 *
 * @code{.sh}
 * mkdir build && cd build
 * cmake ..
 * make -j$(nproc)
 * @endcode
 *
 * @section documentation Documentation Aliases
 *
 * This project uses custom Doxygen aliases for ECS documentation:
 *
 * - `@@ecscomponent` - Mark a struct as an ECS component
 * - `@@ecssystem` - Mark a class as an ECS system
 * - `@@eventbus` - Document EventBus integration
 * - `@@thread_safe` / `@@not_thread_safe` - Thread safety notes
 * - `@@performance` - Performance considerations
 * - `@@usage` - Usage examples
 * - `@@raylib` - raylib integration notes
 *
 * @section links Related Documentation
 *
 * - @ref ECS "ECS Module Documentation"
 * - @ref Graphics "Graphics Module"
 * - @ref UI "UI Widget Library"
 * - [raylib Documentation](https://www.raylib.com/cheatsheet/cheatsheet.html)
 *
 * @section license License
 *
 * See LICENSE file for details.
 */

#endif // DOXYGEN_MAINPAGE_HPP
