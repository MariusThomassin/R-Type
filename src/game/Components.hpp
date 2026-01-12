/**
 * @file Components.hpp
 * @brief Convenience header for all R-Type game components
 * 
 * Include this file to get access to all game-specific components.
 * For engine base components, include engine/ecs/Components.hpp
 */

#pragma once

// Engine base components
#include "engine/ecs/Components.hpp"

// Game-specific components
#include "components/PlayerComponent.hpp"
#include "components/EnemyComponent.hpp"
#include "components/WeaponComponent.hpp"
#include "components/ProjectileComponent.hpp"
#include "components/PlayerShipComponent.hpp"
#include "components/BackgroundComponent.hpp"
#include "components/SpritesheetComponent.hpp"
#include "components/BulletTypes.hpp"
#include "components/BulletSprites.hpp"
#include "components/PowerupComponent.hpp"

// Bullet/trajectory components (refactored)
#include "components/bullets/Bullets.hpp"

// Pattern system components (refactored)
#include "components/patterns/Patterns.hpp"
