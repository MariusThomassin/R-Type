/*
** R-Type ECS - Components Header
** Convenience header to include all component types
*/

#pragma once

// Core components
#include "TransformComponent.hpp"
#include "VelocityComponent.hpp"
#include "SpriteComponent.hpp"
#include "BulletTypes.hpp"
#include "BulletSprites.hpp"
#include "SpritesheetComponent.hpp"
#include "ColliderComponent.hpp"

// Gameplay components
#include "HealthComponent.hpp"
#include "WeaponComponent.hpp"
#include "LifetimeComponent.hpp"
#include "ProjectileComponent.hpp"

// Entity type components
#include "PlayerComponent.hpp"
#include "PlayerShipComponent.hpp"
#include "EnemyComponent.hpp"
#include "AIComponent.hpp"

// Self-rendering components
#include "BackgroundComponent.hpp"

// Multiplayer components
#include "NetworkComponent.hpp"
