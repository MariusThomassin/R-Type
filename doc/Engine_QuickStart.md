# R-Type Engine Quick Start Guide

This guide covers building and running the R-Type project, plus essential development patterns.

---

## Building the Project

### Prerequisites

- **CMake** 3.16+
- **C++17** compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- **Raylib** (fetched automatically via CMake)

### Linux Build

```bash
# Clone the repository
git clone https://github.com/MariusThomassin/R-Type.git
cd R-Type

# Create build directory
mkdir -p build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run the client
./r-type_client
```

### Windows Build (WSL)

See [WINDOWS_BUILD.md](build/WINDOWS_BUILD.md) for detailed Windows instructions.

---

## Controls

| Key | Action |
|-----|--------|
| **WASD** / **Arrow Keys** | Move player ship |
| **Space** | Shoot |
| **F3** | Toggle debug mode |
| **Tab** | Switch debug tabs (when debug active) |
| **Escape** | Exit game |

---

## Project Structure

```
R-Type/
├── src/
│   ├── client/          # Client application
│   │   └── main.cpp     # Entry point
│   ├── server/          # Server application (multiplayer)
│   │   └── main.cpp
│   ├── engine/          # Core engine code
│   │   ├── ecs/         # Entity-Component-System
│   │   │   ├── core/    # Registry, ComponentArray, SparseSet
│   │   │   ├── components/
│   │   │   ├── systems/
│   │   │   └── events/
│   │   └── graphics/    # IRenderer abstraction
│   └── game/            # Game-specific code
│       ├── components/  # Game components (bullets, patterns)
│       └── systems/     # Game systems (trajectory, pattern)
├── doc/                 # Documentation
├── build/               # Build output
└── CMakeLists.txt       # Build configuration
```

---

## Common Development Tasks

### Creating a New Entity

```cpp
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/components/Components.hpp"

// In your initialization code:
EntityId enemy = registry.createEntity();
registry.addComponent<TransformComponent>(enemy, {400.0f, 300.0f});
registry.addComponent<VelocityComponent>(enemy, {-50.0f, 0.0f});
registry.addComponent<EnemyComponent>(enemy, {"basic", 1});
registry.addComponent<HealthComponent>(enemy, {30, 30});
```

### Iterating Entities (Modern Pattern)

```cpp
// Zero-allocation iteration with forEach
registry.forEach<TransformComponent, VelocityComponent>(
    [dt](EntityId entity) {
        auto& transform = registry.getComponent<TransformComponent>(entity);
        auto& velocity = registry.getComponent<VelocityComponent>(entity);
        transform.x += velocity.vx * dt;
        transform.y += velocity.vy * dt;
    }
);

// With direct component access
registry.forEachWith<HealthComponent>(
    [](EntityId id, HealthComponent& health) {
        if (health.currentHealth <= 0) {
            // Handle death
        }
    }
);
```

### Creating Bullet Patterns

```cpp
#include "game/components/patterns/Patterns.hpp"

// Create a pattern spawner
Entity spawner = registry.createEntity();
registry.addComponent(spawner, TransformComponent(400, 100));

PatternSpawnerComponent patternSpawner;
patternSpawner.addPattern(
    PatternFactory::createSpiral(4, 20, 180.0f, 60.0f,
                                 BulletType::Rice, BulletColor::Cyan)
);
patternSpawner.autoStart = true;
registry.addComponent(spawner, patternSpawner);
```

### Creating a Self-Rendering Component

```cpp
// src/shared/ecs/components/ExplosionComponent.hpp
#pragma once

#include "../IComponent.hpp"
#include "../IRenderable.hpp"
#include "TransformComponent.hpp"
#include <raylib.h>

namespace rtype::ecs {

struct ExplosionComponent : public IComponent, public IRenderable {
    float radius = 20.0f;
    float maxLifetime = 0.5f;
    float elapsed = 0.0f;
    int layer = 100;  // Above everything
    
    std::string getTypeName() const override { return "ExplosionComponent"; }
    
    bool isRenderable() const override { return elapsed < maxLifetime; }
    int getRenderLayer() const override { return layer; }
    
    void render(const TransformComponent& t, const RenderContext& ctx) const override {
        float progress = elapsed / maxLifetime;
        float currentRadius = radius * (1.0f + progress);
        unsigned char alpha = static_cast<unsigned char>(255 * (1.0f - progress));
        
        DrawCircle(static_cast<int>(t.x), static_cast<int>(t.y), 
                   currentRadius, {255, 200, 100, alpha});
        DrawCircle(static_cast<int>(t.x), static_cast<int>(t.y), 
                   currentRadius * 0.5f, {255, 255, 200, alpha});
    }
};

} // namespace rtype::ecs
```

### Subscribing to Events

```cpp
#include "engine/ecs/EventBus.hpp"
#include "engine/ecs/events/InputEvents.hpp"

// In your system's constructor or init:
m_eventBus.subscribe<events::KeyPressedEvent>(
    [this](const events::KeyPressedEvent& e) {
        if (e.key == events::KeyCode::Space) {
            handleShoot();
        }
    }
);

m_eventBus.subscribe<events::MouseButtonPressedEvent>(
    [this](const events::MouseButtonPressedEvent& e) {
        if (e.button == events::MouseButton::Left) {
            handleClick(e.x, e.y);
        }
    }
);
```

### Emitting Events

```cpp
// Fire a bullet
m_eventBus.emit(events::ShootEvent{playerId});

// Spawn danmaku pattern at position
m_eventBus.emit(events::DanmakuEvent{x, y});
```

### Querying Components Safely

```cpp
void processEntities(Registry& registry, float dt) {
    // Modern pattern - use forEach for zero-allocation iteration
    registry.forEach<TransformComponent, VelocityComponent>(
        [&registry, dt](EntityId id) {
            auto& transform = registry.getComponent<TransformComponent>(id);
            auto& velocity = registry.getComponent<VelocityComponent>(id);
            
            transform.x += velocity.vx * dt;
            transform.y += velocity.vy * dt;
        }
    );
    
    // For optional components, use tryGetComponent
    registry.forEach<TransformComponent>([&registry](EntityId id) {
        auto* health = registry.tryGetComponent<HealthComponent>(id);
        if (health && health->current <= 0) {
            registry.destroyEntity(id);
        }
    });
}
```

---

## Debug Mode (F3)

Press **F3** to open the debug overlay. Available tabs:

| Tab | Description |
|-----|-------------|
| **Stats** | FPS, entity count, memory usage |
| **Textures** | Preview all loaded textures |
| **Architecture** | Visual ECS diagram |
| **Inspector** | Browse entities and components |
| **Spawner** | Interactively create test entities |
| **Bullets** | Preview bullet spritesheet |
| **Performance** | Frame time graphs |

Click tabs to switch. Some tabs support mouse interaction:
- **Textures**: Scroll to pan, Ctrl+scroll to zoom
- **Inspector**: Click entities to select, scroll component list
- **Spawner**: Click to spawn entities
- **Bullets**: Click to select bullet types/colors

---

## Architecture Overview

```
┌──────────────────────────────────────────────────────┐
│                    Game Loop                         │
│  ┌────────────────────────────────────────────────┐  │
│  │ 1. InputManager.pollInput()                    │  │
│  │ 2. SystemManager.updateAll()                   │  │
│  │    ├── InputSystem      (handle input)         │  │
│  │    ├── MovementSystem   (physics)              │  │
│  │    ├── BulletSystem     (projectiles)          │  │
│  │    ├── RenderSystem     (draw all)             │  │
│  │    └── DebugSystem      (overlay)              │  │
│  └────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────┘

┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Registry   │────▶│   Systems   │────▶│  EventBus   │
│  (Entities) │◀────│  (Logic)    │◀────│  (Events)   │
└─────────────┘     └─────────────┘     └─────────────┘
```

---

## Tips & Tricks

1. **Use F3 liberally** - The debug system is your best friend for understanding what's happening.

2. **Check entity count** - If performance drops, watch the entity count in Stats tab.

3. **Prefer tryGetComponent** - Always use `tryGetComponent<T>()` unless you're certain the component exists.

4. **Self-rendering components** - For visual-heavy components, implement `IRenderable` to keep rendering logic with the data.

5. **Events over direct calls** - Use the EventBus for cross-system communication.

---

## Troubleshooting

### Build fails with raylib errors

Raylib is fetched automatically. If it fails:
```bash
cd build
rm -rf _deps
cmake ..
make
```

### Game window doesn't appear

Check if you have a display available (especially in WSL):
```bash
echo $DISPLAY  # Should show :0 or similar
```

### Low FPS

1. Press F3 to check entity count
2. Look at Performance tab for frame time spikes
3. Check if too many bullets are spawning (LifetimeComponent should destroy old ones)

---

For detailed API documentation, see [ECS_Documentation.md](ECS_Documentation.md).
