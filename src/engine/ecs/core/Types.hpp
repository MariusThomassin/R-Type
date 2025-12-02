/*
** R-Type ECS - Core Types
** Common type definitions for the ECS architecture
*/

#pragma once

#include <bitset>
#include <cstddef>
#include <cstdint>

namespace rtype::ecs {

    using EntityId = std::size_t;

    using ComponentTypeId = std::size_t;

    constexpr std::size_t MAX_COMPONENTS = 64;

    using Signature = std::bitset<MAX_COMPONENTS>;

    constexpr EntityId NULL_ENTITY = 0;

    enum class SystemPhase {
        Input,      // Handle input first
        Physics,    // Physics and movement
        Collision,  // Collision detection/resolution
        GameLogic,  // Game-specific logic (AI, weapons, etc.)
        Render,     // Rendering (last)
        Count       // Number of phases
    };

} // namespace rtype::ecs
