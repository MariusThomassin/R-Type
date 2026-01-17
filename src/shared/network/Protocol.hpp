/*
** R-Type Network Protocol
** Binary protocol for client-server communication
*/

#pragma once

#include <cstdint>
#include <vector>
#include <cstring>

namespace rtype::network {

    /**
     * @brief Message types for network communication
     * 
     * Protocol Version: 2
     * 
     * Ranges:
     * - 0x00-0x09: Client → Server messages
     * - 0x0A-0x1F: Server → Client messages (legacy)
     * - 0x20-0x2F: Level/Game state messages (Server → Client)
     * - 0x30-0x3F: Player profile messages (Bidirectional)
     */
    enum class MessageType : uint8_t {
        // ============================================================
        // Client → Server (0x00 - 0x09)
        // ============================================================
        CLIENT_HELLO = 0x00,        // Initial connection
        CLIENT_INPUT = 0x01,        // Player input
        CLIENT_DISCONNECT = 0x02,   // Clean disconnect
        PLAYER_READY = 0x03,        // Player clicked Play button and is ready
        CHAT_MESSAGE = 0x04,        // Chat message from player
        PAUSE_REQUEST = 0x05,       // Request to pause/unpause
        LOAD_LEVEL_REQUEST = 0x06,  // Request to load a specific level (host only)

        // ============================================================
        // Server → Client: Core (0x0A - 0x1F)
        // ============================================================
        SERVER_WELCOME = 0x0A,      // Connection response (assigns clientId)
        ENTITY_SPAWN = 0x0B,        // Spawn new entity
        ENTITY_STATE = 0x0C,        // Update position/velocity
        ENTITY_DESTROY = 0x0D,      // Destroy entity
        SERVER_SNAPSHOT = 0x0E,     // Full world state snapshot
        PLAYER_SPAWN = 0x0F,        // Spawn player entity
        PLAYER_HIT = 0x10,          // Player took damage
        PLAYER_DEATH = 0x11,        // Player died (will respawn)
        PLAYER_RESPAWN = 0x12,      // Player respawned
        GAME_OVER = 0x13,           // Game over (all players dead)
        CHAT_BROADCAST = 0x14,      // Chat message broadcast to all
        PAUSE_STATE = 0x15,         // Server pause state change
        SCORE_UPDATE = 0x16,        // Score update for a player

        // ============================================================
        // Server → Client: Level Management (0x20 - 0x2F)
        // ============================================================
        LEVEL_INFO = 0x20,          // Level metadata (assets, waves, difficulty)
        LEVEL_START = 0x21,         // Level is starting
        LEVEL_COMPLETE = 0x22,      // Level completed successfully
        WAVE_START = 0x23,          // Wave is starting
        WAVE_COMPLETE = 0x24,       // Wave completed
        BOSS_START = 0x25,          // Boss fight starting
        BOSS_DEFEATED = 0x26,       // Boss was defeated
        DIFFICULTY_CHANGE = 0x27,   // Difficulty multiplier changed (loop)

        // ============================================================
        // Bidirectional: Player Profile (0x30 - 0x3F)
        // ============================================================
        PLAYER_PROFILE = 0x30,      // Player profile (name, avatar)
        PROFILE_UPDATE = 0x31,      // Profile updated (broadcast to others)

        // ============================================================
        // Room/Lobby Management (0x40 - 0x4F)
        // ============================================================
        // Client → Server
        CREATE_ROOM = 0x40,         // Create a new room
        JOIN_ROOM = 0x41,           // Join an existing room
        LEAVE_ROOM = 0x42,          // Leave current room
        ROOM_LIST_REQUEST = 0x43,   // Request list of available rooms
        HOST_START_GAME = 0x44,     // Host starts the game

        // Server → Client
        ROOM_CREATED = 0x45,        // Room creation confirmed
        ROOM_JOINED = 0x46,         // Successfully joined room
        ROOM_LEFT = 0x47,           // Left room confirmation
        ROOM_LIST = 0x48,           // List of available rooms
        ROOM_INFO = 0x49,           // Current room state update
        HOST_CHANGED = 0x4A,        // Host has changed (migration)
        ROOM_ERROR = 0x4B           // Room operation error
    };

    /**
     * @brief Entity types for network spawning
     */
    enum class EntityType : uint8_t {
        PROJECTILE = 0,
        PLAYER = 1,
        ENEMY = 2,
        POWERUP = 3
    };

    /**
     * @brief Message header (5 bytes)
     * Format: [MessageType:1][PayloadSize:4]
     */
    struct MessageHeader {
        MessageType type;
        uint32_t payloadSize;
    };

    // ============================================================
    // Client → Server Messages
    // ============================================================

    /**
     * @brief CLIENT_HELLO: Initial connection request
     */
    struct ClientHelloMessage {
        uint32_t protocolVersion = 1;
        char playerName[32] = {0};  // For future use
    };

    /**
     * @brief CLIENT_DISCONNECT: Clean disconnect notification
     */
    struct ClientDisconnectMessage {
        uint32_t clientId;
    };

    /**
     * @brief Input flags for player controls (bitfield)
     */
    enum InputFlags : uint8_t {
        INPUT_NONE  = 0,        // 0x00
        INPUT_UP    = 1 << 0,   // 0x01
        INPUT_DOWN  = 1 << 1,   // 0x02
        INPUT_LEFT  = 1 << 2,   // 0x04
        INPUT_RIGHT = 1 << 3,   // 0x08
        INPUT_SHOOT = 1 << 4    // 0x10
    };

    /**
     * @brief CLIENT_INPUT: Player input state
     */
    struct ClientInputMessage {
        uint32_t sequenceNumber;    // For prediction/reconciliation (future)
        uint8_t inputFlags;          // Bitfield: UP|DOWN|LEFT|RIGHT|SHOOT
        float deltaTime;             // For lag compensation
    };

    /**
     * @brief PLAYER_READY: Player has clicked Play and is ready to start
     * 
     * This message is sent when the player clicks the Play button in the main menu.
     * The game starts only when at least 2 clients have sent this message.
     * This ensures that the game begins with enough players for multiplayer gameplay.
     */
    struct PlayerReadyMessage {
        uint32_t clientId;          // Client ID that is ready
    };

    /**
     * @brief CHAT_MESSAGE: Chat message from client
     */
    struct ChatMessagePacket {
        uint32_t clientId;          // Sender client ID
        char message[128];          // Message text (null-terminated)
    };

    /**
     * @brief PAUSE_REQUEST: Client requests pause state change
     */
    struct PauseRequestMessage {
        uint32_t clientId;          // Requesting client
        bool pause;                 // true = pause, false = unpause
    };

    // ============================================================
    // Server → Client Messages
    // ============================================================

    /**
     * @brief SERVER_WELCOME: Connection acknowledgment
     */
    struct ServerWelcomeMessage {
        uint32_t clientId;          // Assigned client ID
        uint32_t protocolVersion;
        float serverTime;           // For time synchronization
    };

    /**
     * @brief ENTITY_SPAWN: Spawn new entity on clients
     */
    struct EntitySpawnMessage {
        uint32_t networkId;
        EntityType entityType;

        // Transform
        float x, y;
        float rotation;

        // Velocity
        float vx, vy;

        // Trajectory (for projectiles)
        uint8_t trajectoryType;     // 0=None, 1=Linear, 2=Sinusoidal, etc.
        float trajectoryParam1;     // Amplitude, radius, etc.
        float trajectoryParam2;     // Frequency, speed, etc.

        // Spin
        float spinSpeed;

        // Lifetime
        float maxLifetime;

        // Collider
        float colliderWidth;
        float colliderHeight;
        uint32_t collisionLayer;
        uint32_t collisionMask;
    };

    /**
     * @brief ENTITY_STATE: Update entity transform/velocity
     */
    struct EntityStateMessage {
        uint32_t networkId;
        float x, y;
        float vx, vy;
        float rotation;
    };

    /**
     * @brief ENTITY_DESTROY: Remove entity from clients
     */
    struct EntityDestroyMessage {
        uint32_t networkId;
    };

    /**
     * @brief SERVER_SNAPSHOT: Complete world state (for new clients)
     */
    struct ServerSnapshotMessage {
        uint32_t entityCount;
        float serverTime;
        // Followed by entityCount × EntitySpawnMessage
    };

    /**
     * @brief PLAYER_SPAWN: Spawn player entity
     */
    struct PlayerSpawnMessage {
        uint32_t networkId;
        uint32_t clientId;      // Which client owns this player
        uint8_t playerSlot;     // 0-3 (up to 4 players)
        float x, y;
        float health;
    };

    /**
     * @brief PLAYER_HIT: Player took damage
     */
    struct PlayerHitMessage {
        uint32_t networkId;
        float newHealth;
        float hitX, hitY;       // Impact position (for visual effect)
    };

    /**
     * @brief PLAYER_DEATH: Player died (will respawn after delay)
     */
    struct PlayerDeathMessage {
        uint32_t networkId;
        uint8_t remainingLives;  // Lives left after death
        float deathX, deathY;    // Death position
    };

    /**
     * @brief PLAYER_RESPAWN: Player respawned
     */
    struct PlayerRespawnMessage {
        uint32_t networkId;
        uint8_t playerSlot;
        float x, y;
        float health;
    };

    /**
     * @brief GAME_OVER: All players are dead
     */
    struct GameOverMessage {
        uint8_t survivorCount;  // Number of players still alive (0 for game over)
    };

    /**
     * @brief CHAT_BROADCAST: Chat message broadcast to all clients
     */
    struct ChatBroadcastMessage {
        uint32_t senderId;          // Original sender client ID
        char senderName[32];        // Sender display name
        char message[128];          // Message text (null-terminated)
    };

    /**
     * @brief PAUSE_STATE: Server broadcasts pause state change
     */
    struct PauseStateMessage {
        bool isPaused;              // Current pause state
        uint32_t requesterId;       // Client who triggered the pause
    };

    /**
     * @brief SCORE_UPDATE: Server broadcasts score change
     */
    struct ScoreUpdateMessage {
        uint32_t clientId;          // Client whose score changed
        int32_t newScore;           // New total score
        int32_t delta;              // Points added
    };

    // ============================================================
    // Level Management Messages (Server → Client)
    // ============================================================

    /**
     * @brief Score reason codes for SCORE_UPDATE
     */
    enum class ScoreReason : uint8_t {
        ENEMY_KILL = 0,
        BOSS_KILL = 1,
        WAVE_BONUS = 2,
        LEVEL_BONUS = 3,
        POWERUP = 4,
        NO_DAMAGE_BONUS = 5
    };

    /**
     * @brief LOAD_LEVEL_REQUEST: Client requests to load a level (host only)
     */
    struct LoadLevelRequestMessage {
        uint32_t clientId;          // Requesting client (must be host)
        uint8_t levelIndex;         // Level index (0-255)
    };

    /**
     * @brief LEVEL_INFO: Server sends level metadata before starting
     * 
     * Clients use this to preload assets (background, music).
     */
    struct LevelInfoMessage {
        uint8_t levelIndex;         // Level number (0-based)
        char levelName[32];         // Display name
        char backgroundPath[64];    // Background asset path
        char stageMusicPath[64];    // Stage music asset path
        char bossMusicPath[64];     // Boss music asset path
        uint8_t totalWaves;         // Number of waves in level
        uint8_t difficulty;         // Base difficulty (1-10)
        uint8_t bossEnabled;        // Has boss section (0 or 1)
        uint8_t loopCount;          // Current loop number (0 = first playthrough)
        float difficultyMultiplier; // Difficulty scaling (1.0 = normal)
    };

    /**
     * @brief LEVEL_START: Level is starting
     */
    struct LevelStartMessage {
        uint8_t levelIndex;         // Level number
        float serverTime;           // Server time for synchronization
        uint8_t playerCount;        // Number of active players
    };

    /**
     * @brief LEVEL_COMPLETE: Level completed successfully
     */
    struct LevelCompleteMessage {
        uint8_t levelIndex;         // Completed level
        uint32_t totalScore;        // Team total score
        float completionTime;       // Time to complete
        uint8_t nextLevelIndex;     // Next level (0xFF = loop/no more)
        uint8_t isLooping;          // 1 if starting a new loop
    };

    /**
     * @brief WAVE_START: Wave is starting
     */
    struct WaveStartMessage {
        uint8_t waveNumber;         // Current wave (1-based)
        uint8_t totalWaves;         // Total waves in level
        uint8_t enemyCount;         // Enemies in this wave
    };

    /**
     * @brief WAVE_COMPLETE: Wave completed
     */
    struct WaveCompleteMessage {
        uint8_t waveNumber;         // Completed wave
        uint32_t timeBonus;         // Time bonus points
    };

    /**
     * @brief BOSS_START: Boss fight starting
     */
    struct BossStartMessage {
        uint32_t bossNetworkId;     // Boss entity network ID
        uint8_t totalPhases;        // Number of boss phases
    };

    /**
     * @brief BOSS_DEFEATED: Boss was defeated
     */
    struct BossDefeatedMessage {
        uint32_t bossNetworkId;     // Boss entity that was defeated
        uint32_t scoreValue;        // Points awarded
        float fightDuration;        // How long the fight took
    };

    /**
     * @brief DIFFICULTY_CHANGE: New loop started, difficulty increased
     */
    struct DifficultyChangeMessage {
        uint8_t loopCount;          // Current loop (1 = first repeat)
        float difficultyMultiplier; // New difficulty (1.5 = 150%)
        char displayName[16];       // "NEW GAME+", "LOOP 2", etc.
    };

    // ============================================================
    // Player Profile Messages (Bidirectional)
    // ============================================================

    /**
     * @brief PLAYER_PROFILE: Player profile data
     * 
     * Sent by client after connection, broadcast by server to others.
     */
    struct PlayerProfileMessage {
        uint32_t clientId;          // Client ID
        char playerName[10];        // Player name (9 chars + null)
        uint8_t avatarId;           // Avatar index (0-255)
        uint8_t colorScheme;        // Color scheme (0-7)
    };

    /**
     * @brief Extended score update with reason
     */
    struct ScoreUpdateExtMessage {
        uint32_t clientId;          // Client whose score changed
        uint32_t playerScore;       // Player's individual score
        uint32_t teamScore;         // Team total score
        int32_t delta;              // Points added this update
        ScoreReason reason;         // Why score changed
    };

    // ============================================================
    // Room/Lobby Messages
    // ============================================================

    /**
     * @brief Room state enum
     */
    enum class RoomState : uint8_t {
        LOBBY = 0,      // Waiting for players/host to start
        PLAYING = 1,    // Game in progress
        FINISHED = 2    // Game ended
    };

    /**
     * @brief Room error codes
     */
    enum class RoomError : uint8_t {
        NONE = 0,
        ROOM_FULL = 1,
        ROOM_NOT_FOUND = 2,
        ALREADY_IN_ROOM = 3,
        NOT_HOST = 4,
        GAME_ALREADY_STARTED = 5,
        INVALID_ROOM_NAME = 6
    };

    /**
     * @brief CREATE_ROOM: Client requests to create a new room
     */
    struct CreateRoomMessage {
        uint32_t clientId;          // Client creating the room
        char roomName[32];          // Room name (31 chars + null)
        uint8_t maxPlayers;         // Max players (2-4, default 4)
    };

    /**
     * @brief JOIN_ROOM: Client requests to join a room
     */
    struct JoinRoomMessage {
        uint32_t clientId;          // Client joining
        char roomName[32];          // Room to join
    };

    /**
     * @brief LEAVE_ROOM: Client leaves current room
     */
    struct LeaveRoomMessage {
        uint32_t clientId;          // Client leaving
    };

    /**
     * @brief ROOM_LIST_REQUEST: Client requests list of rooms
     */
    struct RoomListRequestMessage {
        uint32_t clientId;          // Requesting client
    };

    /**
     * @brief HOST_START_GAME: Host starts the game
     */
    struct HostStartGameMessage {
        uint32_t clientId;          // Must be host
        uint8_t levelIndex;         // Level to start (0-based)
    };

    /**
     * @brief ROOM_CREATED: Server confirms room creation
     */
    struct RoomCreatedMessage {
        char roomName[32];          // Created room name
        uint32_t hostClientId;      // Host (creator) client ID
    };

    /**
     * @brief Player info for room state
     */
    struct RoomPlayerInfo {
        uint32_t clientId;
        char playerName[32];
        uint8_t slot;               // Player slot (0-3)
        bool isReady;
        bool isHost;
    };

    /**
     * @brief ROOM_JOINED: Server confirms room join
     */
    struct RoomJoinedMessage {
        char roomName[32];          // Room name
        uint32_t hostClientId;      // Current host
        uint8_t playerCount;        // Current player count
        uint8_t maxPlayers;         // Max allowed
        uint8_t yourSlot;           // Assigned slot for joining player
        bool youAreHost;            // True if this client is the host
    };

    /**
     * @brief ROOM_LEFT: Server confirms room leave
     */
    struct RoomLeftMessage {
        uint32_t clientId;          // Client who left
    };

    /**
     * @brief Room summary for room list
     */
    struct RoomSummary {
        char roomName[32];
        uint8_t playerCount;
        uint8_t maxPlayers;
        RoomState state;
    };

    /**
     * @brief ROOM_LIST: Server sends list of available rooms
     */
    struct RoomListMessage {
        uint8_t roomCount;          // Number of rooms (max 16)
        RoomSummary rooms[16];      // Room summaries
    };

    /**
     * @brief ROOM_INFO: Server broadcasts room state update
     * 
     * Sent when players join/leave/ready or host changes.
     */
    struct RoomInfoMessage {
        char roomName[32];
        uint32_t hostClientId;
        uint8_t playerCount;
        uint8_t maxPlayers;
        RoomState state;
        RoomPlayerInfo players[4];  // Up to 4 players
    };

    /**
     * @brief HOST_CHANGED: Server notifies of host migration
     */
    struct HostChangedMessage {
        uint32_t newHostClientId;   // New host client ID
        char reason[32];            // "Previous host left", etc.
    };

    /**
     * @brief ROOM_ERROR: Server reports room operation error
     */
    struct RoomErrorMessage {
        RoomError error;            // Error code
        char message[64];           // Human-readable error message
    };

    // ============================================================
    // Serialization utilities
    // ============================================================

    /**
     * @brief Serialize a message into a byte buffer
     *
     * @tparam T Message structure type
     * @param type Message type identifier
     * @param message Message data
     * @return Serialized byte buffer with header
     */
    template<typename T>
    std::vector<uint8_t> serializeMessage(MessageType type, const T& message) {
        std::vector<uint8_t> buffer;

        // Reserve space: header (5 bytes) + payload
        buffer.resize(sizeof(MessageHeader) + sizeof(T));

        // Write header
        MessageHeader header;
        header.type = type;
        header.payloadSize = sizeof(T);

        std::memcpy(buffer.data(), &header, sizeof(MessageHeader));

        // Write payload
        std::memcpy(buffer.data() + sizeof(MessageHeader), &message, sizeof(T));

        return buffer;
    }

    /**
     * @brief Deserialize message header from byte buffer
     *
     * @param buffer Input byte buffer
     * @param outHeader Output header
     * @return true if successful
     */
    inline bool deserializeHeader(const std::vector<uint8_t>& buffer, MessageHeader& outHeader) {
        if (buffer.size() < sizeof(MessageHeader)) {
            return false;
        }

        std::memcpy(&outHeader, buffer.data(), sizeof(MessageHeader));
        return true;
    }

    /**
     * @brief Deserialize message payload from byte buffer
     *
     * @tparam T Message structure type
     * @param buffer Input byte buffer
     * @param outMessage Output message
     * @return true if successful
     */
    template<typename T>
    bool deserializeMessage(const std::vector<uint8_t>& buffer, T& outMessage) {
        if (buffer.size() < sizeof(MessageHeader) + sizeof(T)) {
            return false;
        }

        std::memcpy(&outMessage, buffer.data() + sizeof(MessageHeader), sizeof(T));
        return true;
    }

    /**
     * @brief Get message type name for logging
     */
    inline const char* getMessageTypeName(MessageType type) {
        switch (type) {
            // Client → Server
            case MessageType::CLIENT_HELLO: return "CLIENT_HELLO";
            case MessageType::CLIENT_INPUT: return "CLIENT_INPUT";
            case MessageType::CLIENT_DISCONNECT: return "CLIENT_DISCONNECT";
            case MessageType::PLAYER_READY: return "PLAYER_READY";
            case MessageType::CHAT_MESSAGE: return "CHAT_MESSAGE";
            case MessageType::PAUSE_REQUEST: return "PAUSE_REQUEST";
            case MessageType::LOAD_LEVEL_REQUEST: return "LOAD_LEVEL_REQUEST";
            
            // Server → Client: Core
            case MessageType::SERVER_WELCOME: return "SERVER_WELCOME";
            case MessageType::ENTITY_SPAWN: return "ENTITY_SPAWN";
            case MessageType::ENTITY_STATE: return "ENTITY_STATE";
            case MessageType::ENTITY_DESTROY: return "ENTITY_DESTROY";
            case MessageType::SERVER_SNAPSHOT: return "SERVER_SNAPSHOT";
            case MessageType::PLAYER_SPAWN: return "PLAYER_SPAWN";
            case MessageType::PLAYER_HIT: return "PLAYER_HIT";
            case MessageType::PLAYER_DEATH: return "PLAYER_DEATH";
            case MessageType::PLAYER_RESPAWN: return "PLAYER_RESPAWN";
            case MessageType::GAME_OVER: return "GAME_OVER";
            case MessageType::CHAT_BROADCAST: return "CHAT_BROADCAST";
            case MessageType::PAUSE_STATE: return "PAUSE_STATE";
            case MessageType::SCORE_UPDATE: return "SCORE_UPDATE";
            
            // Server → Client: Level Management
            case MessageType::LEVEL_INFO: return "LEVEL_INFO";
            case MessageType::LEVEL_START: return "LEVEL_START";
            case MessageType::LEVEL_COMPLETE: return "LEVEL_COMPLETE";
            case MessageType::WAVE_START: return "WAVE_START";
            case MessageType::WAVE_COMPLETE: return "WAVE_COMPLETE";
            case MessageType::BOSS_START: return "BOSS_START";
            case MessageType::BOSS_DEFEATED: return "BOSS_DEFEATED";
            case MessageType::DIFFICULTY_CHANGE: return "DIFFICULTY_CHANGE";
            
            // Player Profile
            case MessageType::PLAYER_PROFILE: return "PLAYER_PROFILE";
            case MessageType::PROFILE_UPDATE: return "PROFILE_UPDATE";

            // Room/Lobby Management
            case MessageType::CREATE_ROOM: return "CREATE_ROOM";
            case MessageType::JOIN_ROOM: return "JOIN_ROOM";
            case MessageType::LEAVE_ROOM: return "LEAVE_ROOM";
            case MessageType::ROOM_LIST_REQUEST: return "ROOM_LIST_REQUEST";
            case MessageType::HOST_START_GAME: return "HOST_START_GAME";
            case MessageType::ROOM_CREATED: return "ROOM_CREATED";
            case MessageType::ROOM_JOINED: return "ROOM_JOINED";
            case MessageType::ROOM_LEFT: return "ROOM_LEFT";
            case MessageType::ROOM_LIST: return "ROOM_LIST";
            case MessageType::ROOM_INFO: return "ROOM_INFO";
            case MessageType::HOST_CHANGED: return "HOST_CHANGED";
            case MessageType::ROOM_ERROR: return "ROOM_ERROR";
            
            default: return "UNKNOWN";
        }
    }

} // namespace rtype::network
