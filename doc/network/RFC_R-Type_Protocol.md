# RFC: R-Type Network Protocol Specification

**Document Version:** 1.0  
**Status:** Standard  
**Date:** January 2026  
**Authors:** R-Type Development Team

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Protocol Overview](#2-protocol-overview)
3. [Transport Layer](#3-transport-layer)
4. [Message Format](#4-message-format)
5. [Message Types](#5-message-types)
6. [Connection Lifecycle](#6-connection-lifecycle)
7. [Game State Synchronization](#7-game-state-synchronization)
8. [Entity Management](#8-entity-management)
9. [Player Management](#9-player-management)
10. [Error Handling](#10-error-handling)
11. [Security Considerations](#11-security-considerations)
12. [References](#12-references)

---

## 1. Introduction

### 1.1 Purpose

This document specifies the binary network protocol used for client-server communication in the R-Type multiplayer game. The protocol enables real-time game state synchronization between a game server and multiple game clients.

### 1.2 Scope

This RFC covers:
- Binary message format and serialization
- Client-to-server and server-to-client message types
- Connection establishment and termination
- Entity state synchronization
- Player input handling

### 1.3 Requirements Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

### 1.4 Terminology

| Term | Definition |
|------|------------|
| **Client** | A game instance connecting to the server |
| **Server** | The authoritative game simulation host |
| **Entity** | Any game object (player, projectile, enemy, powerup) |
| **Network ID** | Unique identifier for networked entities |
| **Client ID** | Unique identifier assigned to each connected client |
| **Tick** | One iteration of the game simulation (60 Hz) |

---

## 2. Protocol Overview

### 2.1 Architecture

The R-Type protocol follows a **client-server authoritative model**:

```
┌─────────────────────────────────────────────────────────────────┐
│                         GAME SERVER                              │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │              Authoritative Game Simulation               │    │
│  │                     (60 Hz Tick Rate)                    │    │
│  └─────────────────────────────────────────────────────────┘    │
│                              │                                   │
│                    ┌─────────┴─────────┐                        │
│                    ▼                   ▼                         │
│              UDP Socket           State Updates                  │
│                    │                   │                         │
└────────────────────│───────────────────│────────────────────────┘
                     │                   │
         ┌───────────┼───────────────────┼───────────┐
         │           │                   │           │
         ▼           ▼                   ▼           ▼
    ┌─────────┐ ┌─────────┐       ┌─────────┐ ┌─────────┐
    │ Client 1│ │ Client 2│  ...  │ Client 3│ │ Client 4│
    └─────────┘ └─────────┘       └─────────┘ └─────────┘
```

### 2.2 Design Principles

1. **Server Authority**: The server is the single source of truth for game state
2. **Stateless Messages**: Each message is self-contained
3. **Binary Format**: Compact binary serialization for low bandwidth
4. **Fixed-Rate Simulation**: Server runs at constant 60 Hz
5. **Broadcast Updates**: Entity states are broadcast to all connected clients

### 2.3 Protocol Version

The current protocol version is **1**. Clients and servers MUST agree on protocol version during connection handshake.

---

## 3. Transport Layer

### 3.1 Transport Protocol

The R-Type protocol operates over **UDP (User Datagram Protocol)**.

**Rationale**: UDP is chosen for:
- Lower latency (no connection overhead)
- Better suited for real-time game updates
- Tolerance for occasional packet loss

### 3.2 Default Port

| Type | Port |
|------|------|
| Server | **4242** (configurable) |
| Client | Ephemeral (OS-assigned) |

### 3.3 Packet Size

- **Maximum Payload Size**: 1024 bytes
- **Message Header**: 5 bytes
- **Effective Payload**: 1019 bytes

### 3.4 Byte Order

All multi-byte values MUST be transmitted in **little-endian** byte order.

---

## 4. Message Format

### 4.1 Message Structure

All messages follow a consistent binary format:

```
┌────────────────────────────────────────────────────────────┐
│                      MESSAGE PACKET                         │
├────────────────────────────────────────────────────────────┤
│  ┌──────────────────────────────────────────────────────┐  │
│  │                  MESSAGE HEADER (5 bytes)             │  │
│  ├─────────────────┬────────────────────────────────────┤  │
│  │  MessageType    │         PayloadSize                │  │
│  │   (1 byte)      │          (4 bytes)                 │  │
│  │   uint8_t       │          uint32_t                  │  │
│  └─────────────────┴────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │                  MESSAGE PAYLOAD                      │  │
│  │              (PayloadSize bytes)                      │  │
│  │                                                       │  │
│  │         Structure depends on MessageType              │  │
│  └──────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────┘
```

### 4.2 Header Definition

```cpp
struct MessageHeader {
    uint8_t  type;        // MessageType enum value
    uint32_t payloadSize; // Size of payload in bytes
};
```

**Total Header Size**: 5 bytes

---

## 5. Message Types

### 5.1 Message Type Enumeration

Messages are categorized by direction and purpose:

| Value | Name | Direction | Description |
|-------|------|-----------|-------------|
| **0x00** | `CLIENT_HELLO` | Client → Server | Initial connection request |
| **0x01** | `CLIENT_INPUT` | Client → Server | Player input state |
| **0x02** | `CLIENT_DISCONNECT` | Client → Server | Clean disconnection |
| **0x03** | `PLAYER_READY` | Client → Server | Player ready to start |
| **0x0A** | `SERVER_WELCOME` | Server → Client | Connection acknowledgment |
| **0x0B** | `ENTITY_SPAWN` | Server → Client | Spawn new entity |
| **0x0C** | `ENTITY_STATE` | Server → Client | Update entity position |
| **0x0D** | `ENTITY_DESTROY` | Server → Client | Remove entity |
| **0x0E** | `SERVER_SNAPSHOT` | Server → Client | Full world state |
| **0x0F** | `PLAYER_SPAWN` | Server → Client | Spawn player entity |
| **0x10** | `PLAYER_HIT` | Server → Client | Player took damage |
| **0x11** | `PLAYER_DEATH` | Server → Client | Player died |
| **0x12** | `PLAYER_RESPAWN` | Server → Client | Player respawned |
| **0x13** | `GAME_OVER` | Server → Client | Game ended |

---

### 5.2 Client → Server Messages

#### 5.2.1 CLIENT_HELLO (0x00)

Sent by client to initiate connection.

```
┌─────────────────────────────────────────┐
│           CLIENT_HELLO (36 bytes)       │
├─────────────────────────────────────────┤
│  protocolVersion   │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  playerName[32]    │  char[32] (32 bytes)│
└─────────────────────────────────────────┘
```

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `protocolVersion` | uint32_t | 4 | Protocol version (MUST be 1) |
| `playerName` | char[32] | 32 | Player display name (null-terminated, reserved for future use) |

#### 5.2.2 CLIENT_INPUT (0x01)

Sent by client to transmit player input state.

```
┌─────────────────────────────────────────┐
│           CLIENT_INPUT (9 bytes)        │
├─────────────────────────────────────────┤
│  sequenceNumber    │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  inputFlags        │  uint8_t  (1 byte) │
├─────────────────────────────────────────┤
│  deltaTime         │  float    (4 bytes)│
└─────────────────────────────────────────┘
```

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `sequenceNumber` | uint32_t | 4 | Sequence number for reconciliation |
| `inputFlags` | uint8_t | 1 | Bitfield of pressed inputs |
| `deltaTime` | float | 4 | Client delta time (lag compensation) |

**Input Flags Bitfield:**

```
┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
│ Bit │  7  │  6  │  5  │  4  │  3  │  2  │  1  │  0  │
├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤
│Value│ --- │ --- │ --- │SHOOT│RIGHT│LEFT │DOWN │ UP  │
│     │ 0x80│ 0x40│ 0x20│ 0x10│ 0x08│ 0x04│ 0x02│ 0x01│
└─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
```

| Bit | Mask | Action |
|-----|------|--------|
| 0 | 0x01 | Move Up |
| 1 | 0x02 | Move Down |
| 2 | 0x04 | Move Left |
| 3 | 0x08 | Move Right |
| 4 | 0x10 | Shoot |
| 5-7 | --- | Reserved |

#### 5.2.3 CLIENT_DISCONNECT (0x02)

Sent by client for clean disconnection.

```
┌─────────────────────────────────────────┐
│         CLIENT_DISCONNECT (4 bytes)     │
├─────────────────────────────────────────┤
│  clientId          │  uint32_t (4 bytes)│
└─────────────────────────────────────────┘
```

#### 5.2.4 PLAYER_READY (0x03)

Sent when player clicks "Play" button.

```
┌─────────────────────────────────────────┐
│           PLAYER_READY (4 bytes)        │
├─────────────────────────────────────────┤
│  clientId          │  uint32_t (4 bytes)│
└─────────────────────────────────────────┘
```

**Note**: The game starts only when at least 2 clients have sent `PLAYER_READY`.

---

### 5.3 Server → Client Messages

#### 5.3.1 SERVER_WELCOME (0x0A)

Sent by server to acknowledge connection and assign client ID.

```
┌─────────────────────────────────────────┐
│         SERVER_WELCOME (12 bytes)       │
├─────────────────────────────────────────┤
│  clientId          │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  protocolVersion   │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  serverTime        │  float    (4 bytes)│
└─────────────────────────────────────────┘
```

| Field | Type | Size | Description |
|-------|------|------|-------------|
| `clientId` | uint32_t | 4 | Assigned unique client identifier |
| `protocolVersion` | uint32_t | 4 | Server protocol version |
| `serverTime` | float | 4 | Current server game time (synchronization) |

#### 5.3.2 ENTITY_SPAWN (0x0B)

Sent to spawn a new entity on clients.

```
┌─────────────────────────────────────────┐
│          ENTITY_SPAWN (57 bytes)        │
├─────────────────────────────────────────┤
│  networkId         │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  entityType        │  uint8_t  (1 byte) │
├─────────────────────────────────────────┤
│  x                 │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  y                 │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  rotation          │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  vx                │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  vy                │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  trajectoryType    │  uint8_t  (1 byte) │
├─────────────────────────────────────────┤
│  trajectoryParam1  │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  trajectoryParam2  │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  spinSpeed         │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  maxLifetime       │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  colliderWidth     │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  colliderHeight    │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  collisionLayer    │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  collisionMask     │  uint32_t (4 bytes)│
└─────────────────────────────────────────┘
```

**Entity Types:**

| Value | Type | Description |
|-------|------|-------------|
| 0 | PROJECTILE | Bullet/projectile |
| 1 | PLAYER | Player ship |
| 2 | ENEMY | Enemy entity |
| 3 | POWERUP | Power-up item |

**Trajectory Types:**

| Value | Type | Description |
|-------|------|-------------|
| 0 | None | No special trajectory |
| 1 | Linear | Straight line movement |
| 2 | Sinusoidal | Wave pattern movement |
| 3 | Spiral | Spiral movement |
| 4 | Homing | Tracking movement |
| 5 | Circular | Circular orbit |
| 6 | Zigzag | Zigzag pattern |
| 7 | Figure8 | Figure-8 pattern |

#### 5.3.3 ENTITY_STATE (0x0C)

Sent to update entity position/velocity.

```
┌─────────────────────────────────────────┐
│          ENTITY_STATE (24 bytes)        │
├─────────────────────────────────────────┤
│  networkId         │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  x                 │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  y                 │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  vx                │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  vy                │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  rotation          │  float    (4 bytes)│
└─────────────────────────────────────────┘
```

#### 5.3.4 ENTITY_DESTROY (0x0D)

Sent to remove entity from clients.

```
┌─────────────────────────────────────────┐
│         ENTITY_DESTROY (4 bytes)        │
├─────────────────────────────────────────┤
│  networkId         │  uint32_t (4 bytes)│
└─────────────────────────────────────────┘
```

#### 5.3.5 SERVER_SNAPSHOT (0x0E)

Sent to provide full world state (for new clients or resync).

```
┌─────────────────────────────────────────┐
│         SERVER_SNAPSHOT (8 bytes + N)   │
├─────────────────────────────────────────┤
│  entityCount       │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  serverTime        │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  entities[]        │  EntitySpawnMessage│
│                    │  × entityCount     │
└─────────────────────────────────────────┘
```

#### 5.3.6 PLAYER_SPAWN (0x0F)

Sent to spawn a player entity.

```
┌─────────────────────────────────────────┐
│          PLAYER_SPAWN (21 bytes)        │
├─────────────────────────────────────────┤
│  networkId         │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  clientId          │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  playerSlot        │  uint8_t  (1 byte) │
├─────────────────────────────────────────┤
│  x                 │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  y                 │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  health            │  float    (4 bytes)│
└─────────────────────────────────────────┘
```

**Player Slots**: 0-3 (supports up to 4 players)

#### 5.3.7 PLAYER_HIT (0x10)

Sent when player takes damage.

```
┌─────────────────────────────────────────┐
│           PLAYER_HIT (16 bytes)         │
├─────────────────────────────────────────┤
│  networkId         │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  newHealth         │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  hitX              │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  hitY              │  float    (4 bytes)│
└─────────────────────────────────────────┘
```

#### 5.3.8 PLAYER_DEATH (0x11)

Sent when player dies (health reaches 0).

```
┌─────────────────────────────────────────┐
│          PLAYER_DEATH (13 bytes)        │
├─────────────────────────────────────────┤
│  networkId         │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  remainingLives    │  uint8_t  (1 byte) │
├─────────────────────────────────────────┤
│  deathX            │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  deathY            │  float    (4 bytes)│
└─────────────────────────────────────────┘
```

#### 5.3.9 PLAYER_RESPAWN (0x12)

Sent when player respawns after death.

```
┌─────────────────────────────────────────┐
│         PLAYER_RESPAWN (17 bytes)       │
├─────────────────────────────────────────┤
│  networkId         │  uint32_t (4 bytes)│
├─────────────────────────────────────────┤
│  playerSlot        │  uint8_t  (1 byte) │
├─────────────────────────────────────────┤
│  x                 │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  y                 │  float    (4 bytes)│
├─────────────────────────────────────────┤
│  health            │  float    (4 bytes)│
└─────────────────────────────────────────┘
```

#### 5.3.10 GAME_OVER (0x13)

Sent when game ends (all players dead).

```
┌─────────────────────────────────────────┐
│           GAME_OVER (1 byte)            │
├─────────────────────────────────────────┤
│  survivorCount     │  uint8_t  (1 byte) │
└─────────────────────────────────────────┘
```

---

## 6. Connection Lifecycle

### 6.1 Connection Establishment

```
┌──────────┐                              ┌──────────┐
│  Client  │                              │  Server  │
└────┬─────┘                              └────┬─────┘
     │                                         │
     │  ──────── CLIENT_HELLO ───────────────► │
     │           (protocolVersion=1)           │
     │                                         │
     │  ◄─────── SERVER_WELCOME ───────────── │
     │           (clientId, serverTime)        │
     │                                         │
     │  ◄─────── SERVER_SNAPSHOT ──────────── │
     │           (current world state)         │
     │                                         │
```

### 6.2 Game Start Sequence

```
┌──────────┐      ┌──────────┐              ┌──────────┐
│ Client 1 │      │ Client 2 │              │  Server  │
└────┬─────┘      └────┬─────┘              └────┬─────┘
     │                 │                         │
     │   PLAYER_READY  │                         │
     │  ─────────────────────────────────────► │
     │                 │                         │
     │                 │   PLAYER_READY          │
     │                 │  ──────────────────────►│
     │                 │                         │
     │  ◄──────────── PLAYER_SPAWN ───────────  │
     │                 │◄─────────────────────── │
     │                 │                         │
     │                 │     Game Started        │
     │                 │     (2+ players ready)  │
```

### 6.3 Clean Disconnection

```
┌──────────┐                              ┌──────────┐
│  Client  │                              │  Server  │
└────┬─────┘                              └────┬─────┘
     │                                         │
     │  ──────── CLIENT_DISCONNECT ──────────► │
     │           (clientId)                    │
     │                                         │
     │                          Remove client  │
     │                          Destroy player │
     │                                         │
     │  Broadcast ENTITY_DESTROY to others     │
```

---

## 7. Game State Synchronization

### 7.1 Server Tick Rate

- **Simulation Rate**: 60 Hz (16.67ms per tick)
- **Network Update Rate**: 20 Hz (50ms intervals) for entity states
- **Fixed Timestep**: 1/60 second

### 7.2 Entity State Updates

The server broadcasts `ENTITY_STATE` messages for all moving entities at regular intervals:

```
Time ────────────────────────────────────────────────►
     │                                               │
     ├── Tick 1 ─┼── Tick 2 ─┼── Tick 3 ─┼─ ...     │
     │           │           │           │           │
     │           ▼           │           ▼           │
     │    ENTITY_STATE       │    ENTITY_STATE       │
     │    (broadcast)        │    (broadcast)        │
```

### 7.3 Snapshot Mechanism

Full snapshots are sent:
- On client connection (via `SERVER_SNAPSHOT`)
- For resynchronization requests

---

## 8. Entity Management

### 8.1 Network ID Assignment

- Network IDs are assigned server-side using `NetworkIdManager`
- IDs are unique 32-bit unsigned integers
- IDs are reused after entity destruction

### 8.2 Entity Lifecycle

```
┌─────────────────────────────────────────────────────────────┐
│                    ENTITY LIFECYCLE                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   Server Creates Entity                                     │
│          │                                                  │
│          ▼                                                  │
│   ┌─────────────────┐                                      │
│   │ Assign NetworkId │                                      │
│   └────────┬────────┘                                      │
│            │                                                │
│            ▼                                                │
│   ┌─────────────────┐         ┌──────────────────┐        │
│   │ ENTITY_SPAWN    │────────►│ Client creates   │        │
│   │ (broadcast)     │         │ local entity     │        │
│   └────────┬────────┘         └──────────────────┘        │
│            │                                                │
│            ▼                                                │
│   ┌─────────────────┐         ┌──────────────────┐        │
│   │ ENTITY_STATE    │────────►│ Client updates   │        │
│   │ (periodic)      │         │ entity transform │        │
│   └────────┬────────┘         └──────────────────┘        │
│            │                                                │
│            ▼                                                │
│   ┌─────────────────┐         ┌──────────────────┐        │
│   │ ENTITY_DESTROY  │────────►│ Client removes   │        │
│   │ (broadcast)     │         │ local entity     │        │
│   └─────────────────┘         └──────────────────┘        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 9. Player Management

### 9.1 Player Slots

The game supports up to **4 concurrent players** (slots 0-3).

### 9.2 Spawn Positions

| Slot | X Position | Y Position |
|------|------------|------------|
| 0 | 100 | 180 |
| 1 | 100 | 360 |
| 2 | 100 | 540 |
| 3 | 100 | 720 |

### 9.3 Player Death and Respawn

```
┌──────────┐                              ┌──────────┐
│  Client  │                              │  Server  │
└────┬─────┘                              └────┬─────┘
     │                                         │
     │                    Player takes damage  │
     │  ◄──────── PLAYER_HIT ────────────────  │
     │                                         │
     │                    Health reaches 0     │
     │  ◄──────── PLAYER_DEATH ──────────────  │
     │           (remainingLives)              │
     │                                         │
     │                    After 3 second delay │
     │  ◄──────── PLAYER_RESPAWN ────────────  │
     │           (new position, full health)   │
```

**Respawn Delay**: 3 seconds

---

## 10. Error Handling

### 10.1 Protocol Version Mismatch

If client and server protocol versions don't match:
- Server SHOULD reject connection
- Server MAY send error response (future extension)

### 10.2 Invalid Message Handling

Messages with invalid format or type:
- MUST be silently discarded
- SHOULD be logged server-side for debugging

### 10.3 Client Timeout

- Heartbeat tracking via `lastHeartbeat` field
- Inactive clients SHOULD be disconnected after timeout (implementation-defined)

---

## 11. Security Considerations

### 11.1 Current Limitations

The protocol currently does NOT provide:
- Encryption
- Authentication
- Anti-cheat mechanisms

### 11.2 Recommendations

For production deployments:
- Use TLS/DTLS for encryption
- Implement server-side validation of all client inputs
- Rate-limit input messages to prevent flooding

---

## 12. References

1. R-Type Game Source Code - `src/shared/network/Protocol.hpp`
2. ASIO Networking Library - https://think-async.com/Asio/
3. RFC 2119 - Key words for use in RFCs
4. R-Type ECS Architecture - `doc/ECS_Documentation.md`

---

## Appendix A: Message Size Summary

| Message Type | Payload Size (bytes) |
|--------------|---------------------|
| CLIENT_HELLO | 36 |
| CLIENT_INPUT | 9 |
| CLIENT_DISCONNECT | 4 |
| PLAYER_READY | 4 |
| SERVER_WELCOME | 12 |
| ENTITY_SPAWN | 57 |
| ENTITY_STATE | 24 |
| ENTITY_DESTROY | 4 |
| SERVER_SNAPSHOT | 8 + N×57 |
| PLAYER_SPAWN | 21 |
| PLAYER_HIT | 16 |
| PLAYER_DEATH | 13 |
| PLAYER_RESPAWN | 17 |
| GAME_OVER | 1 |

---

## Appendix B: Example Message Serialization

### B.1 CLIENT_INPUT Example

```
Binary representation (Little-Endian):
┌──────┬──────────────────┬───────────────────────────────┐
│Header│     Payload      │          Description          │
├──────┼──────────────────┼───────────────────────────────┤
│ 0x01 │                  │ MessageType: CLIENT_INPUT     │
│ 0x09 0x00 0x00 0x00    │ PayloadSize: 9                │
├──────┼──────────────────┼───────────────────────────────┤
│      │ 0x42 0x00 0x00 0x00 │ sequenceNumber: 66         │
│      │ 0x05              │ inputFlags: UP + LEFT        │
│      │ 0x89 0x88 0x88 0x3C │ deltaTime: 0.01666...     │
└──────┴──────────────────┴───────────────────────────────┘
Total: 14 bytes (5 header + 9 payload)
```

---

*End of RFC Document*
