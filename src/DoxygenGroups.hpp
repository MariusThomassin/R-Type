/**
 * @file DoxygenGroups.hpp
 * @brief Doxygen module and group definitions for R-Type ECS Engine
 *
 * This file defines the documentation structure for organizing
 * classes, functions, and concepts into logical groups.
 */

#ifndef DOXYGEN_GROUPS_HPP
#define DOXYGEN_GROUPS_HPP

/**
 * @defgroup ECS Entity Component System
 * @brief Core ECS architecture components
 *
 * The Entity Component System (ECS) is a data-oriented design pattern
 * that separates data (Components) from behavior (Systems), using
 * Entities as lightweight identifiers.
 *
 * @section ecs_overview Overview
 * - **Entity**: A unique identifier (EntityId) representing a game object
 * - **Component**: Pure data structs attached to entities
 * - **System**: Logic that operates on entities with specific components
 * - **Registry**: Central manager for entities and components
 * - **EventBus**: Pub/sub system for decoupled communication
 */

/**
 * @defgroup ECS_Core Core ECS Types
 * @ingroup ECS
 * @brief Fundamental ECS types and the Registry
 *
 * Core types include EntityId, ComponentPool, and the Registry
 * which manages all entity-component relationships.
 */

/**
 * @defgroup ECS_Components Components
 * @ingroup ECS
 * @brief Data components for game entities
 *
 * Components are plain data structures that hold state.
 * They should have no behavior (methods), only data members.
 *
 * @par Naming Convention
 * All component names should be descriptive and end with a clear noun
 * (e.g., Position, Velocity, Health, Sprite).
 */

/**
 * @defgroup ECS_Systems Systems
 * @ingroup ECS
 * @brief Systems that process entities with specific components
 *
 * Systems contain the game logic. Each system typically queries
 * for entities with a specific set of components and processes them.
 *
 * @par System Types
 * - **Update Systems**: Run every frame (physics, AI, movement)
 * - **Render Systems**: Handle drawing (sprites, particles, UI)
 * - **Event Systems**: React to events via EventBus
 */

/**
 * @defgroup Events Event System
 * @ingroup ECS
 * @brief EventBus and event types for decoupled communication
 *
 * The EventBus enables systems to communicate without direct
 * dependencies, following the publish-subscribe pattern.
 */

/**
 * @defgroup Graphics Graphics & Rendering
 * @brief Rendering, windowing, and visual systems
 *
 * Graphics module handles all visual aspects including:
 * - Window management via raylib
 * - Sprite and texture rendering
 * - Animation systems
 * - Particle effects
 * - Debug visualization
 */

/**
 * @defgroup UI User Interface
 * @ingroup Graphics
 * @brief UI widgets and management
 *
 * The UI system provides reusable widgets for game interfaces:
 * - TextWidget, ButtonWidget, PanelWidget, ProgressBarWidget
 * - UIManager for widget lifecycle and event routing
 * - Full ECS EventBus integration for input handling
 */

/**
 * @defgroup Input Input Handling
 * @brief Input processing and event generation
 *
 * Input handling converts raw hardware input (keyboard, mouse, gamepad)
 * into game events published through the EventBus.
 */

/**
 * @defgroup Audio Audio System
 * @brief Sound and music playback
 *
 * Audio module handles sound effects and background music
 * using raylib's audio capabilities.
 */

/**
 * @defgroup Networking Network System
 * @brief Client-server communication
 *
 * Network module handles multiplayer functionality:
 * - UDP/TCP communication
 * - State synchronization
 * - Client prediction and interpolation
 */

/**
 * @defgroup Game Game Logic
 * @brief R-Type specific game implementation
 *
 * Game-specific components and systems for the R-Type shoot-em-up:
 * - Player ship control
 * - Enemy AI and spawning
 * - Bullet patterns and collision
 * - Scoring and progression
 */

/**
 * @defgroup Debug Debug Tools
 * @ingroup Game
 * @brief Development and debugging utilities
 *
 * Debug tools for development:
 * - DebugSystem with tabbed interface
 * - Performance profiling
 * - Entity inspector
 * - Component editors
 */

/**
 * @defgroup Utils Utilities
 * @brief Helper classes and utility functions
 *
 * General purpose utilities:
 * - Math helpers (Vector2, Color)
 * - Type traits and concepts
 * - Memory management utilities
 */

#endif // DOXYGEN_GROUPS_HPP
