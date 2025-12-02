/**
 * @file Systems.hpp
 * @brief Convenience header for all R-Type game systems
 * 
 * Include this file to get access to all game-specific systems.
 * For engine base systems, include engine/ecs/Systems.hpp
 */

#pragma once

// Engine base systems
#include "engine/ecs/Systems.hpp"

// Game-specific systems
#include "systems/InputSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/BulletSystem.hpp"
#include "systems/DebugSystem.hpp"
