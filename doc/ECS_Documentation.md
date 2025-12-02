# R-Type ECS Engine Documentation

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Core Concepts](#core-concepts)
4. [Components](#components)
5. [Systems](#systems)
6. [Events](#events)
7. [Rendering](#rendering)
8. [Input Handling](#input-handling)
9. [Debug System](#debug-system)
10. [Best Practices](#best-practices)

---

## Overview

The R-Type ECS Engine is a custom **Entity-Component-System** architecture built for the R-Type game project. It emphasizes:

- **Data-oriented design**: Components are pure data, systems contain logic
- **Self-rendering pattern**: Components can optionally render themselves via `IRenderable`
- **Event-driven communication**: Decoupled systems via `EventBus`
- **Pointer-safe access**: `tryGetComponent<T>()` returns nullable pointers for safe component queries

### Directory Structure

```
src/shared/ecs/
├── IComponent.hpp          # Base component interface
├── IRenderable.hpp         # Self-rendering interface
├── ISystem.hpp             # System interface
├── Registry.hpp            # Entity & component storage
├── SystemManager.hpp       # System orchestration
├── EventBus.hpp            # Publish-subscribe events
├── ComponentArray.hpp      # Type-safe component storage
├── components/             # All component types
│   ├── Components.hpp      # Convenience header
│   ├── TransformComponent.hpp
│   ├── VelocityComponent.hpp
│   ├── SpriteComponent.hpp
│   ├── SpritesheetComponent.hpp
│   ├── PlayerShipComponent.hpp
│   ├── BackgroundComponent.hpp
│   └── ...
├── systems/                # All system types
│   ├── Systems.hpp         # Convenience header
│   ├── InputSystem.hpp
│   ├── MovementSystem.hpp
│   ├── RenderSystem.hpp
│   ├── BulletSystem.hpp
│   ├── DebugSystem.hpp
│   └── debug/              # Debug tab modules
│       ├── DebugTab.hpp
│       ├── StatsTab.hpp
│       ├── TexturesTab.hpp
│       └── ...
└── events/                 # Event types
    ├── Events.hpp          # Convenience header
    ├── InputEvents.hpp     # Input event types
    └── InputManager.hpp    # Platform input abstraction
```

---

## Architecture

### High-Level Flow

```
┌─────────────────────────────────────────────────────────────┐
│                      Game Loop                               │
├─────────────────────────────────────────────────────────────┤
│  1. InputManager.pollInput()    → Emit input events          │
│  2. SystemManager.updateAll()   → Run all systems            │
│     ├── InputSystem            → Handle player input         │
│     ├── MovementSystem         → Update positions            │
│     ├── BulletSystem           → Manage projectiles          │
│     ├── RenderSystem           → Draw everything             │
│     └── DebugSystem            → Debug overlay (F3)          │
└─────────────────────────────────────────────────────────────┘
```

### Data Flow

```
                    ┌─────────────┐
                    │   Registry  │
                    │  (Entities  │
                    │ +Components)│
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
           ▼               ▼               ▼
    ┌────────────┐  ┌────────────┐  ┌────────────┐
    │InputSystem │  │MovementSys │  │RenderSystem│
    └─────┬──────┘  └─────┬──────┘  └─────┬──────┘
          │               │               │
          │   EventBus    │               │
          └───────┬───────┘               │
                  ▼                       ▼
          ┌─────────────┐          ┌─────────────┐
          │ShootEvent   │          │ Draw calls  │
          │DanmakuEvent │          │ (Raylib)    │
          └─────────────┘          └─────────────┘
```

---

## Core Concepts

### Entity

An entity is simply a unique ID (`size_t`) that represents a game object. It has no behavior or data on its own—it's just an identifier that components are attached to.

```cpp
EntityId player = registry.createEntity();
EntityId bullet = registry.createEntity();
registry.destroyEntity(bullet);
```

### Component

Components are **pure data structures** that attach to entities. They implement `IComponent` for type identification.

```cpp
struct TransformComponent : public IComponent {
    float x = 0.0f, y = 0.0f;
    float rotation = 0.0f;
    float scaleX = 1.0f, scaleY = 1.0f;
    
    std::string getTypeName() const override {
        return "TransformComponent";
    }
};
```

### System

Systems contain **all the logic**. They query the registry for entities with specific component combinations and process them.

```cpp
class MovementSystem : public ISystem {
public:
    void update(Registry& registry, float dt) override {
        for (auto& [id, entity] : registry.getEntities()) {
            auto* transform = registry.tryGetComponent<TransformComponent>(id);
            auto* velocity = registry.tryGetComponent<VelocityComponent>(id);
            
            if (transform && velocity) {
                transform->x += velocity->vx * dt;
                transform->y += velocity->vy * dt;
            }
        }
    }
};
```

### Registry

The registry is the central storage for all entities and their components.

```cpp
// Create entity with components
EntityId id = registry.createEntity();
registry.addComponent<TransformComponent>(id, {100.0f, 200.0f});
registry.addComponent<VelocityComponent>(id, {50.0f, 0.0f});

// Query components (safe - returns nullptr if not found)
auto* transform = registry.tryGetComponent<TransformComponent>(id);
if (transform) {
    transform->x += 10.0f;
}

// Query components (throws if not found)
auto& velocity = registry.getComponent<VelocityComponent>(id);

// Check existence
if (registry.hasComponent<PlayerComponent>(id)) {
    // Handle player-specific logic
}
```

---

## Components

### Data Components

| Component | Purpose | Key Fields |
|-----------|---------|------------|
| `TransformComponent` | Position, rotation, scale | `x, y, rotation, scaleX, scaleY` |
| `VelocityComponent` | Movement speed | `vx, vy, maxSpeed` |
| `ColliderComponent` | Collision bounds | `width, height, isTrigger` |
| `HealthComponent` | Health & damage | `current, max, isInvincible` |
| `WeaponComponent` | Shooting capabilities | `fireRate, damage, cooldown` |
| `PlayerComponent` | Player identity | `playerId, score, lives` |
| `EnemyComponent` | Enemy properties | `enemyType, difficultyLevel` |
| `ProjectileComponent` | Bullet properties | `damage, ownerId, bulletType` |
| `LifetimeComponent` | Auto-destruction | `timeRemaining, elapsed` |
| `AIComponent` | AI state machine | `state, targetId` |
| `NetworkComponent` | Multiplayer sync | `networkId, isOwned` |

### Self-Rendering Components

These components implement both `IComponent` and `IRenderable`:

| Component | Purpose | Rendering |
|-----------|---------|-----------|
| `SpriteComponent` | Static/animated sprites | Texture-based |
| `SpritesheetComponent` | Bullet/danmaku patterns | Spritesheet frames + glow |
| `PlayerShipComponent` | Player ship with effects | Procedural pixel-art |
| `BackgroundComponent` | Parallax starfield | Procedural stars |

#### IRenderable Interface

```cpp
class IRenderable {
public:
    virtual void render(const TransformComponent& transform, 
                       const RenderContext& ctx) const = 0;
    virtual bool isRenderable() const { return true; }
    virtual int getRenderLayer() const { return 0; }
};
```

Components that implement `IRenderable` are responsible for their own drawing:

```cpp
struct PlayerShipComponent : public IComponent, public IRenderable {
    // ... data fields ...
    
    void render(const TransformComponent& transform, 
                const RenderContext& ctx) const override {
        // Draw the ship at transform.x, transform.y
        DrawRectangle(transform.x - 10, transform.y - 5, 20, 10, BLUE);
        // Draw engine flame animation using ctx.animTime
        // ...
    }
    
    int getRenderLayer() const override { return 10; } // Player layer
};
```

---

## Systems

### System Execution Order

Systems are updated in a specific order each frame:

1. **InputSystem** - Process player input, emit events
2. **MovementSystem** - Apply velocities to transforms
3. **BulletSystem** - Update projectiles, check lifetimes
4. **RenderSystem** - Draw all entities
5. **DebugSystem** - Draw debug overlay (when active)

### InputSystem

Handles keyboard and mouse input via `InputManager`, emits events for game actions.

```cpp
// Subscribes to input events
m_eventBus.subscribe<KeyPressedEvent>([this](const KeyPressedEvent& e) {
    if (e.key == KeyCode::Space) {
        m_eventBus.emit(ShootEvent{playerId});
    }
});
```

### MovementSystem

Updates entity positions based on velocity components.

### BulletSystem

Manages projectile lifecycle:
- Spawns bullets on `ShootEvent`
- Updates bullet positions
- Destroys bullets when `LifetimeComponent` expires

### RenderSystem

Coordinates all rendering:
1. Collects all entities with `IRenderable` components
2. Sorts by render layer
3. Calls `render()` on each component (self-rendering pattern)
4. Draws UI elements

```cpp
void update(Registry& registry, float dt) override {
    // Collect renderables
    std::vector<RenderInfo> renderables;
    for (auto& [id, entity] : registry.getEntities()) {
        auto* transform = registry.tryGetComponent<TransformComponent>(id);
        // Check for IRenderable components...
        // Add to renderables list with layer info
    }
    
    // Sort by layer
    std::sort(renderables.begin(), renderables.end(), 
              [](const auto& a, const auto& b) { return a.layer < b.layer; });
    
    // Render each
    for (const auto& r : renderables) {
        r.renderable->render(*r.transform, m_context);
    }
}
```

### DebugSystem

Modular debug overlay activated by pressing **F3**:

- **StatsTab** - FPS, entity count, component stats
- **TexturesTab** - Loaded texture viewer with zoom
- **ArchitectureTab** - Visual ECS diagram
- **EntityInspectorTab** - Browse entities and their components
- **EntitySpawnerTab** - Interactive entity creation demo
- **BulletsTab** - Bullet spritesheet preview
- **PerformanceTab** - Frame time graphs

---

## Events

### EventBus

The `EventBus` enables decoupled communication between systems using a publish-subscribe pattern.

```cpp
// Subscribe to an event
EventBus::SubscriberId id = eventBus.subscribe<ShootEvent>(
    [this](const ShootEvent& e) {
        spawnBullet(e.shooterId);
    }
);

// Emit an event
eventBus.emit(ShootEvent{playerId});

// Unsubscribe when done
eventBus.unsubscribe<ShootEvent>(id);
```

### Input Events

| Event | When Emitted | Key Fields |
|-------|--------------|------------|
| `KeyPressedEvent` | Key pressed (single frame) | `key: KeyCode` |
| `KeyReleasedEvent` | Key released | `key: KeyCode` |
| `KeyStateEvent` | Every frame | `state: KeyState` |
| `MouseMoveEvent` | Mouse moved | `x, y, deltaX, deltaY` |
| `MouseButtonPressedEvent` | Mouse click | `button, x, y` |
| `MouseWheelEvent` | Scroll wheel | `delta` |

### Game Events

| Event | Purpose |
|-------|---------|
| `ShootEvent` | Player wants to fire |
| `DanmakuEvent` | Spawn bullet pattern |
| `DebugToggleEvent` | Toggle debug overlay |

---

## Rendering

### RenderContext

Passed to `IRenderable::render()` with all necessary resources:

```cpp
struct RenderContext {
    const std::unordered_map<std::string, Texture2D>* textures;
    int screenWidth, screenHeight;
    float animTime;   // Total elapsed time for animations
    float deltaTime;  // Frame delta for smooth animations
    
    const Texture2D* getTexture(const std::string& id) const;
};
```

### Render Layers

Entities are rendered in layer order (lower = behind):

| Layer | Content |
|-------|---------|
| -100 | Background (stars) |
| 0 | Default |
| 5 | Enemies |
| 10 | Player |
| 15 | Projectiles |
| 100 | UI/Effects |

---

## Input Handling

### InputManager

Platform-agnostic input polling that emits events:

```cpp
// In game loop (once per frame)
inputManager.pollInput();

// Query current state
const KeyState& keys = inputManager.getKeyState();
if (keys.moveUp()) {
    // W or Up arrow held
}

const MouseState& mouse = inputManager.getMouseState();
float mx = mouse.x;
float my = mouse.y;
```

### MouseInput (for UI)

Simplified mouse state for UI/debug components:

```cpp
struct MouseInput {
    int x, y;           // Position
    bool clicked;       // Left button pressed this frame
    bool rightClicked;  // Right button pressed this frame
    float wheelDelta;   // Scroll wheel
    bool isHeld;        // Left button held
};
```

---

## Debug System

### Activating Debug Mode

Press **F3** to toggle the debug overlay. Click tabs to switch views.

### Creating Custom Debug Tabs

Implement `IDebugTab`:

```cpp
class MyDebugTab : public IDebugTab {
public:
    std::string getName() const override { return "My Tab"; }
    
    void draw(Registry& registry) override {
        DrawText("My custom debug info", 50, 100, 16, WHITE);
    }
    
    void handleMouse(const MouseInput& mouse) override {
        if (mouse.clicked && isMouseOver(100, 100, 50, 20)) {
            // Handle button click
        }
    }
};
```

### UI Helpers

Base class provides helper methods:

```cpp
// Check if mouse is over a rectangle
bool isMouseOver(int x, int y, int w, int h) const;

// Draw a clickable button (returns true if clicked)
bool drawButton(const char* text, int x, int y, int w, int h, Color bg);
```

---

## Best Practices

### 1. Use `tryGetComponent` for Safety

```cpp
// ✅ Good - null-safe
auto* health = registry.tryGetComponent<HealthComponent>(id);
if (health) {
    health->current -= damage;
}

// ❌ Risky - throws if component missing
auto& health = registry.getComponent<HealthComponent>(id);
```

### 2. Keep Components as Pure Data

```cpp
// ✅ Good - pure data
struct HealthComponent : public IComponent {
    int current = 100;
    int max = 100;
};

// ❌ Avoid - logic in components
struct HealthComponent : public IComponent {
    void takeDamage(int amount) { ... }  // Logic belongs in system
};
```

**Exception**: `IRenderable` components contain render logic because it's intrinsic to the component's visual representation.

### 3. Use Events for Cross-System Communication

```cpp
// ✅ Good - decoupled via events
eventBus.emit(ShootEvent{playerId});

// ❌ Avoid - tight coupling
bulletSystem.spawnBullet(playerId);
```

### 4. Layer Your Rendering

```cpp
int getRenderLayer() const override {
    return 10;  // Define clear layer values
}
```

### 5. Self-Rendering for Visual Components

When a component is primarily visual, make it `IRenderable`:

```cpp
struct ExplosionComponent : public IComponent, public IRenderable {
    float radius = 10.0f;
    float lifetime = 0.5f;
    
    void render(const TransformComponent& t, const RenderContext& ctx) const override {
        // Explosion knows how to draw itself
        float alpha = 1.0f - (ctx.animTime / lifetime);
        DrawCircle(t.x, t.y, radius * alpha, Fade(ORANGE, alpha));
    }
};
```

---

## Quick Reference

### Adding a New Component

1. Create `src/shared/ecs/components/MyComponent.hpp`
2. Inherit from `IComponent` (and optionally `IRenderable`)
3. Include in `Components.hpp`

### Adding a New System

1. Create `src/shared/ecs/systems/MySystem.hpp`
2. Inherit from `ISystem`
3. Implement `update(Registry&, float dt)`
4. Add to `SystemManager` in main

### Adding a New Event

1. Define struct in `InputEvents.hpp` or new file
2. Emit with `eventBus.emit(MyEvent{...})`
3. Subscribe with `eventBus.subscribe<MyEvent>(...)`

---

*Last updated: December 2025*
