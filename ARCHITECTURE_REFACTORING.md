# Architecture Refactoring - Critical Flaws

## Overview

This document identifies architectural flaws that will block implementing advanced features (multi-threading, second game, scripting, modding, etc.). Fix these **before** adding complex features.

---

## CRITICAL BLOCKERS (Must Fix First)

### 1. Non-Thread-Safe EventBus

**Problem**: `EventBus` has no mutex protection despite being used across threads.

**Location**: `src/engine/ecs/core/EventBus.hpp`

**Why it blocks you**:
- Network thread (`NetworkManager::receiveLoop`) processes messages → triggers events
- Main thread (`GameServer::tick`) emits events
- Race condition: both threads access `subscribers_` map concurrently
- `std::any` type erasure + direct function call = data race

**Fix Required**:
```cpp
// Add to EventBus:
mutable std::shared_mutex mutex_;
std::queue<Event> crossThreadQueue_;
std::mutex queueMutex_;

// For cross-thread events:
void emitCrossThread(const Event& event); // Thread-safe enqueue
void processCrossThreadEvents();          // Call from main thread
```

**Priority**: URGENT - Blocks all multi-threading work

---

### 2. Global Mutable State in Registry

**Problem**: `Registry` stores all components in `std::unordered_map` with no thread safety. Systems hold raw pointers and modify directly.

**Location**: `src/engine/ecs/core/Registry.hpp`

**Why it blocks you**:
- Multiple systems cannot run in parallel (Physics, Network, Rendering threads)
- No isolation between game instances
- Cannot clone/copy registry state for replay system
- `forEachWith` provides direct references - unsafe for threading

**Fix Required**:
```cpp
class Registry {
    // Per-component mutex for fine-grained locking
    std::unordered_map<ComponentType, std::unique_ptr<std::shared_mutex>> componentMutexes_;

    // Command pattern for deferred modifications
    struct Command {
        std::function<void(Registry&)> execute;
    };
    std::vector<Command> commandQueue_;

    // Transaction mechanism
    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();
};
```

**Priority**: URGENT - Blocks multi-threading, replay system

---

### 3. Float Determinism Violations

**Problem**: Extensive use of `float` throughout physics. Operations like `std::sqrt`, division without consistency checks.

**Location**: `src/engine/ecs/systems/MovementSystem.cpp`, all physics systems

**Why it blocks you**:
- Float operations are non-deterministic across:
  - CPU architectures (x86 vs ARM)
  - Compiler optimizations (-O2 vs -O3)
  - SIMD instructions
- Network replay WILL desync
- Cannot verify replays

**Fix Required**:
```cpp
// Separate deterministic floats from rendering floats
namespace Game {
    using Fixed = std::int64_t;  // 16.16 fixed-point
    constexpr Fixed FIXED_SCALE = 65536;

    struct Vec2 {
        Fixed x, y;
        // Deterministic operations only
    };
}

namespace Render {
    using Float = float;  // Can be non-deterministic
}

// Critical: Use deterministic math for game state
// - No FMA (fused multiply-add)
// - Consistent rounding mode
// - No std::sqrt (use bit operations or lookup)
```

**Priority**: HIGH - Blocks network determinism, replay verification

---

## HIGH PRIORITY (Blocks Multiple Features)

### 4. Tight Coupling: Game Logic in Engine

**Problem**: Game-specific components (`PlayerComponent`, `WeaponComponent`) and systems (`PatternSystem`) are in `src/game/` but reference engine types directly. No clear separation.

**Location**: `src/game/` directory

**Why it blocks you**:
- Cannot reuse ECS for a second game without R-Type baggage
- No plugin/modding architecture
- Hard-coded game rules in systems
- Blocks: second game, scripting system, moddability

**Fix Required**:
```
Restructure to:
├── engine/          # Pure ECS, no game logic
│   ├── ecs/
│   └── core/
├── runtime/         # Game-agnostic systems
│   ├── assets/
│   └── scripting/
└── games/
    ├── rtype/       # R-Type specific
    └── flappy/      # Second game
```

Add plugin system:
```cpp
// IGamePlugin.hpp
class IGamePlugin {
public:
    virtual void registerComponents(Registry&) = 0;
    virtual void registerSystems(SystemManager&) = 0;
    virtual void registerScripts(ScriptEngine&) = 0;
};

// Load via DLL/so
extern "C" IGamePlugin* createPlugin();
```

**Priority**: HIGH - Blocks engine modularity, second game

---

### 5. No Asset Management System

**Problem**: Assets loaded directly via Raylib in systems. No central manager, no hot-reloading, hard-coded paths.

**Location**: `src/client/main.cpp` (textures loaded inline)

**Why it blocks you**:
- Cannot unload unused assets → memory leaks
- No mod support
- No asset versioning
- No asset pipeline
- Blocks: asset editor, moddable assets, multiple levels

**Fix Required**:
```cpp
// AssetManager.hpp
class AssetManager {
public:
    template<typename T>
    std::shared_ptr<T> load(const std::string& path);

    template<typename T>
    void unload(const std::string& path);

    void reloadChanged();  // Hot-reload

    struct AssetMetadata {
        std::uuid id;
        std::string path;
        uint64_t hash;
        int refCount;
    };

private:
    std::unordered_map<std::string, AssetMetadata> registry_;
    std::unordered_map<std::uuid, std::shared_ptr<void>> cache_;
};

// Asset pack format (ZIP/archive)
// Asset metadata (JSON)
// Asset cooking pipeline
```

**Priority**: HIGH - Blocks asset editor, modding

---

### 6. Hard-Coded Configuration Values

**Problem**: Magic numbers everywhere. Screen sizes (1280x720), speeds, delays all hard-coded.

**Location**: `src/server/GameServer.hpp:191-196`, `src/client/main.cpp`

**Why it blocks you**:
- Cannot configure without recompiling
- No difficulty settings
- No mod support
- Blocks: asset editor, multiple levels, scripting

**Fix Required**:
```cpp
// config.json
{
    "video": {
        "resolution": [1280, 720],
        "fullscreen": false,
        "vsync": true
    },
    "game": {
        "playerSpeed": 200.0,
        "fireRate": 0.15,
        "startingLives": 3
    },
    "difficulty": {
        "enemySpawnRate": 2.0,
        "enemyHealthMultiplier": 1.0
    }
}

// ConfigManager.hpp
class ConfigManager {
public:
    void load(const std::string& path);
    void save(const std::string& path);
    void reload();  // Hot-reload

    template<typename T>
    T get(const std::string& key);

private:
    nlohmann::json config_;
    std::filesystem::file_time_t lastModified_;
};
```

**Priority**: HIGH - Blocks configuration, difficulty system

---

## MEDIUM PRIORITY (Blocks Specific Features)

### 7. Network Protocol Lacks Compression

**Problem**: Raw binary serialization, no compression, no delta encoding. State updates send full floats every time (20 Hz).

**Location**: `src/shared/network/Protocol.hpp`

**Impact**: 100 entities = 2KB/packet × 20 Hz = 40 KB/s per server

**Fix Required**:
```cpp
// Quantize floats to 16-bit
struct QuantizedVec2 {
    int16_t x, y;  // -32k to 32k, ~0.001 precision
};

// Delta compression
struct EntityDelta {
    NetworkId id;
    uint32_t changedMask;  // Bitmask of changed fields
    QuantizedVec2 position;  // Only if changed
    QuantizedVec2 velocity;
    // ...
};

// Snapshot interpolation
struct Snapshot {
    uint32_t sequence;
    std::vector<EntityState> entities;
};
std::deque<Snapshot> snapshots_;
```

**Priority**: MEDIUM - Blocks network compression requirement

---

### 8. No Serialization Framework

**Problem**: No way to save/load entities or levels. Components are POD structs but no serialization code.

**Location**: All component files

**Why it blocks you**:
- Cannot save game state
- Cannot load levels from files
- Cannot record/verify replays

**Fix Required**:
```cpp
// Component base
struct IComponent {
    virtual void serialize(nlohmann::json& out) const = 0;
    virtual void deserialize(const nlohmann::json& in) = 0;
    static constexpr uint32_t VERSION = 1;
};

// Level file format
{
    "version": 1,
    "entities": [
        {
            "id": 123,
            "components": {
                "Transform": {"position": [100, 200], "rotation": 0},
                "Sprite": {"texture": "enemy1.gif"}
            }
        }
    ]
}
```

**Priority**: MEDIUM - Blocks replay system, level loading

---

### 9. Monolithic Main Loop

**Problem**: `src/client/main.cpp` has 450 lines doing everything: window init, UI setup, game loop, rendering.

**Why it blocks you**:
- Cannot embed engine in another application
- Cannot swap rendering backends easily
- Blocks: making engine a library

**Fix Required**:
```cpp
// GameApplication.hpp
class GameApplication {
public:
    GameApplication(const Config& config);
    void run();

private:
    void initialize();
    void shutdown();
    void update(float dt);
    void render();

    std::unique_ptr<StateMachine> stateMachine_;
    std::unique_ptr<IRenderer> renderer_;
};

// Usage in main.cpp
int main() {
    Config config = ConfigManager::load("config.json");
    GameApplication app(config);
    app.run();
}
```

**Priority**: MEDIUM - Blocks engine as library

---

### 10. No Level/Scene Management

**Problem**: No concept of "levels" or "scenes". Entities created manually in code.

**Why it blocks you**:
- Cannot create level editor
- Cannot load different levels
- No scene transitions

**Fix Required**:
```cpp
// SceneManager.hpp
class SceneManager {
public:
    void loadLevel(const std::string& name);
    void unloadLevel();
    void transitionTo(const std::string& name, float duration);

    struct Level {
        std::string name;
        std::string theme;
        std::vector<EntitySpawn> entities;
        std::string backgroundMusic;
        // ...
    };

private:
    std::unordered_map<std::string, Level> levels_;
    Level* currentLevel_ = nullptr;
};
```

**Priority**: MEDIUM - Blocks multiple levels, procedural generation

---

### 11. Collision System is O(N²)

**Problem**: `CollisionSystem::update` does pairwise checks for all entities. No spatial partitioning.

**Location**: `src/game/systems/CollisionSystem.cpp:34-59`

**Impact**: 1000 entities = 1,000,000 checks per frame

**Fix Required**:
```cpp
// Spatial hash grid
class SpatialHash {
public:
    void insert(Entity entity, const AABB& bounds);
    void query(const AABB& bounds, std::vector<Entity>& out);

private:
    static constexpr int CELL_SIZE = 64;
    std::unordered_map<int, std::vector<Entity>> cells_;
    int hash(const Vec2& pos);
};

// Broad-phase optimization
void CollisionSystem::update(float dt) {
    // 1. Broad phase: spatial hash
    // 2. Narrow phase: AABB check
    // 3. Resolve collisions
}
```

**Priority**: MEDIUM - Blocks scaling, procedural levels

---

### 12. No Authentication or Session Management

**Problem**: `NetworkManager` assigns client IDs sequentially with no authentication. No session tokens.

**Location**: `src/server/NetworkManager.cpp`

**Why it blocks you**:
- Anyone can connect and inject packets
- No way to verify client identity
- No replay attack protection

**Fix Required**:
```cpp
// Session management
struct Session {
    std::string token;
    std::chrono::system_clock::time_point expires;
    uint32_t sequence;
    bool authenticated = false;
};

// Authentication
struct AuthChallenge {
    std::array<uint8_t, 32> nonce;
    // Client must respond with HMAC(nonce, shared_secret)
};

// Anti-replay
uint32_t lastSequence_ = 0;
void validateSequence(uint32_t seq);
```

**Priority**: MEDIUM - Blocks authentication requirement

---

## Feature Blockage Matrix

| Feature | Blocking Issues |
|---------|-----------------|
| **Multi-threading** | 1, 2, 3, 11 |
| **Engine as library** | 2, 4, 9 |
| **Scripting system** | 4, 6 |
| **Asset editor** | 5, 6 |
| **Game recording/replay** | 1, 2, 3, 8 |
| **Multiple levels/themes** | 4, 5, 6, 8, 10 |
| **Complex boss fights** | 3, 11 |
| **Procedural generation** | 8, 11 |
| **Authentication** | 12 |
| **Network compression** | 7 |

---

## Recommended Fix Order

### Phase 1: Threading Foundation (1-2 weeks)
1. **Fix EventBus thread safety** (Issue #1)
2. **Add Registry transaction system** (Issue #2)
3. **Implement deterministic math** (Issue #3)

### Phase 2: Core Architecture (2-3 weeks)
4. **Decouple engine from game** (Issue #4)
5. **Add asset management** (Issue #5)
6. **Externalize configuration** (Issue #6)

### Phase 3: Advanced Features (2-3 weeks)
7. **Add serialization** (Issue #8)
8. **Implement spatial partitioning** (Issue #11)
9. **Add network compression** (Issue #7)
10. **Implement level/scene system** (Issue #10)

---

## Risk Assessment

| Issue | Risk if NOT Fixed | Effort |
|-------|-------------------|--------|
| 1. EventBus | Data races, crashes | 2-3 days |
| 2. Registry | Cannot multi-thread | 3-5 days |
| 3. Float determinism | Replay desync | 3-5 days |
| 4. Coupling | Cannot reuse engine | 5-7 days |
| 5. Assets | Memory leaks, no mods | 3-4 days |
| 6. Config | Inflexible game | 1-2 days |

**Total Estimated Refactoring Time**: ~4-6 weeks

---

## Quick Wins (Low Effort, High Impact)

1. **Add ConfigManager** (1-2 days) - Unlocks difficulty, settings
2. **Fix EventBus mutex** (2-3 days) - Enables thread-safe events
3. **Add spatial hash** (2-3 days) - Massive performance gain
4. **Externalize constants** (1 day) - Immediate flexibility
