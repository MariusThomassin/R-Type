// R-Type Network Protocol Tests
// Tests for binary protocol serialization/deserialization

#include <catch2/catch_all.hpp>
#include "shared/network/Protocol.hpp"

using namespace rtype::network;

// ============================================================
// Client → Server Message Tests
// ============================================================

TEST_CASE("ClientHelloMessage serialization/deserialization", "[protocol][client]") {
    ClientHelloMessage original;
    original.protocolVersion = 1;
    strcpy(original.playerName, "TestPlayer");

    auto buffer = serializeMessage(MessageType::CLIENT_HELLO, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(ClientHelloMessage));

    ClientHelloMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.protocolVersion == original.protocolVersion);
    REQUIRE(strcmp(deserialized.playerName, original.playerName) == 0);
}

TEST_CASE("ClientInputMessage serialization/deserialization", "[protocol][client]") {
    ClientInputMessage original;
    original.sequenceNumber = 12345;
    original.inputFlags = INPUT_UP | INPUT_SHOOT;
    original.deltaTime = 0.016f;

    auto buffer = serializeMessage(MessageType::CLIENT_INPUT, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(ClientInputMessage));

    ClientInputMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.sequenceNumber == original.sequenceNumber);
    REQUIRE(deserialized.inputFlags == original.inputFlags);
    REQUIRE(deserialized.deltaTime == original.deltaTime);
}

TEST_CASE("ClientDisconnectMessage serialization/deserialization", "[protocol][client]") {
    ClientDisconnectMessage original;
    original.clientId = 42;

    auto buffer = serializeMessage(MessageType::CLIENT_DISCONNECT, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(ClientDisconnectMessage));

    ClientDisconnectMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.clientId == original.clientId);
}

TEST_CASE("PLAYER_READY message serialization/deserialization", "[protocol][client]") {
    PlayerReadyMessage original;
    original.clientId = 99;

    auto buffer = serializeMessage(MessageType::PLAYER_READY, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(PlayerReadyMessage));

    PlayerReadyMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.clientId == original.clientId);
}

// ============================================================
// Server → Client Message Tests
// ============================================================

TEST_CASE("SERVER_WELCOME message serialization/deserialization", "[protocol][server]") {
    ServerWelcomeMessage original;
    original.clientId = 123;
    original.protocolVersion = 1;
    original.serverTime = 0.0f;

    auto buffer = serializeMessage(MessageType::SERVER_WELCOME, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(ServerWelcomeMessage));

    ServerWelcomeMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.clientId == original.clientId);
    REQUIRE(deserialized.protocolVersion == original.protocolVersion);
    REQUIRE(deserialized.serverTime == original.serverTime);
}

TEST_CASE("ENTITY_SPAWN message serialization/deserialization", "[protocol][server]") {
    EntitySpawnMessage original;
    original.networkId = 456;
    original.entityType = EntityType::PROJECTILE;
    original.x = 100.5f;
    original.y = 200.25f;
    original.rotation = 0.0f;
    original.vx = 300.0f;
    original.vy = 0.0f;

    auto buffer = serializeMessage(MessageType::ENTITY_SPAWN, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(EntitySpawnMessage));

    EntitySpawnMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.networkId == original.networkId);
    REQUIRE(deserialized.entityType == original.entityType);
    REQUIRE(deserialized.x == original.x);
    REQUIRE(deserialized.y == original.y);
    REQUIRE(deserialized.rotation == original.rotation);
    REQUIRE(deserialized.vx == original.vx);
    REQUIRE(deserialized.vy == original.vy);
}

TEST_CASE("ENTITY_STATE message serialization/deserialization", "[protocol][server]") {
    EntityStateMessage original;
    original.networkId = 789;
    original.x = 500.0f;
    original.y = 300.0f;
    original.rotation = 1.5f;
    original.vx = 250.0f;
    original.vy = -100.0f;

    auto buffer = serializeMessage(MessageType::ENTITY_STATE, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(EntityStateMessage));

    EntityStateMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.networkId == original.networkId);
    REQUIRE(deserialized.x == original.x);
    REQUIRE(deserialized.y == original.y);
    REQUIRE(deserialized.rotation == original.rotation);
    REQUIRE(deserialized.vx == original.vx);
    REQUIRE(deserialized.vy == original.vy);
}

TEST_CASE("ENTITY_DESTROY message serialization/deserialization", "[protocol][server]") {
    EntityDestroyMessage original;
    original.networkId = 999;

    auto buffer = serializeMessage(MessageType::ENTITY_DESTROY, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(EntityDestroyMessage));

    EntityDestroyMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.networkId == original.networkId);
}

TEST_CASE("PLAYER_SPAWN message serialization/deserialization", "[protocol][server]") {
    PlayerSpawnMessage original;
    original.networkId = 111;
    original.clientId = 222;
    original.playerSlot = 1;
    original.x = 150.0f;
    original.y = 360.0f;
    original.health = 100.0f;

    auto buffer = serializeMessage(MessageType::PLAYER_SPAWN, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(PlayerSpawnMessage));

    PlayerSpawnMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.networkId == original.networkId);
    REQUIRE(deserialized.clientId == original.clientId);
    REQUIRE(deserialized.playerSlot == original.playerSlot);
    REQUIRE(deserialized.x == original.x);
    REQUIRE(deserialized.y == original.y);
    REQUIRE(deserialized.health == original.health);
}

TEST_CASE("PLAYER_HIT message serialization/deserialization", "[protocol][server]") {
    PlayerHitMessage original;
    original.networkId = 333;
    original.newHealth = 75.0f;
    original.hitX = 200.0f;
    original.hitY = 300.0f;

    auto buffer = serializeMessage(MessageType::PLAYER_HIT, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(PlayerHitMessage));

    PlayerHitMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.networkId == original.networkId);
    REQUIRE(deserialized.newHealth == original.newHealth);
    REQUIRE(deserialized.hitX == original.hitX);
    REQUIRE(deserialized.hitY == original.hitY);
}

// ============================================================
// Player Death System Message Tests
// ============================================================

TEST_CASE("PLAYER_DEATH message serialization/deserialization", "[protocol][server][death-system]") {
    PlayerDeathMessage original;
    original.networkId = 444;
    original.remainingLives = 2;
    original.deathX = 500.0f;
    original.deathY = 400.0f;

    auto buffer = serializeMessage(MessageType::PLAYER_DEATH, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(PlayerDeathMessage));

    PlayerDeathMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.networkId == original.networkId);
    REQUIRE(deserialized.remainingLives == original.remainingLives);
    REQUIRE(deserialized.deathX == original.deathX);
    REQUIRE(deserialized.deathY == original.deathY);
}

TEST_CASE("PLAYER_RESPAWN message serialization/deserialization", "[protocol][server][death-system]") {
    PlayerRespawnMessage original;
    original.networkId = 555;
    original.playerSlot = 2;
    original.x = 150.0f;
    original.y = 520.0f;
    original.health = 100.0f;

    auto buffer = serializeMessage(MessageType::PLAYER_RESPAWN, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(PlayerRespawnMessage));

    PlayerRespawnMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.networkId == original.networkId);
    REQUIRE(deserialized.playerSlot == original.playerSlot);
    REQUIRE(deserialized.x == original.x);
    REQUIRE(deserialized.y == original.y);
    REQUIRE(deserialized.health == original.health);
}

TEST_CASE("GAME_OVER message serialization/deserialization", "[protocol][server][death-system]") {
    GameOverMessage original;
    original.survivorCount = 0;

    auto buffer = serializeMessage(MessageType::GAME_OVER, original);

    REQUIRE(buffer.size() == sizeof(MessageHeader) + sizeof(GameOverMessage));

    GameOverMessage deserialized;
    REQUIRE(deserializeMessage(buffer, deserialized));

    REQUIRE(deserialized.survivorCount == original.survivorCount);
}

// ============================================================
// Message Header Tests
// ============================================================

TEST_CASE("Message header is correctly written", "[protocol][header]") {
    ClientHelloMessage message;
    message.protocolVersion = 1;

    auto buffer = serializeMessage(MessageType::CLIENT_HELLO, message);

    REQUIRE(buffer.size() >= sizeof(MessageHeader));

    MessageHeader header;
    REQUIRE(deserializeHeader(buffer, header));

    REQUIRE(header.type == MessageType::CLIENT_HELLO);
    REQUIRE(header.payloadSize == sizeof(ClientHelloMessage));
}

TEST_CASE("Message header deserialization fails on too small buffer", "[protocol][header]") {
    std::vector<uint8_t> tinyBuffer(3, 0);  // Less than header size

    MessageHeader header;
    REQUIRE_FALSE(deserializeHeader(tinyBuffer, header));
}

// ============================================================
// Buffer Overflow Protection Tests
// ============================================================

TEST_CASE("Message deserialization fails on truncated buffer", "[protocol][security]") {
    EntitySpawnMessage original;
    original.networkId = 123;
    original.entityType = EntityType::PLAYER;
    original.x = 100.0f;
    original.y = 200.0f;
    original.rotation = 0.0f;
    original.vx = 0.0f;
    original.vy = 0.0f;

    auto buffer = serializeMessage(MessageType::ENTITY_SPAWN, original);

    // Truncate buffer by removing last few bytes
    buffer.resize(buffer.size() - 5);

    EntitySpawnMessage deserialized;
    REQUIRE_FALSE(deserializeMessage(buffer, deserialized));
}

TEST_CASE("Message deserialization fails on empty buffer", "[protocol][security]") {
    std::vector<uint8_t> emptyBuffer;

    MessageHeader header;
    REQUIRE_FALSE(deserializeHeader(emptyBuffer, header));

    ClientHelloMessage message;
    REQUIRE_FALSE(deserializeMessage(emptyBuffer, message));
}

// ============================================================
// Message Type Name Tests
// ============================================================

TEST_CASE("getMessageTypeName returns correct names", "[protocol][utility]") {
    REQUIRE(std::string(getMessageTypeName(MessageType::CLIENT_HELLO)) == "CLIENT_HELLO");
    REQUIRE(std::string(getMessageTypeName(MessageType::CLIENT_INPUT)) == "CLIENT_INPUT");
    REQUIRE(std::string(getMessageTypeName(MessageType::CLIENT_DISCONNECT)) == "CLIENT_DISCONNECT");
    REQUIRE(std::string(getMessageTypeName(MessageType::PLAYER_READY)) == "PLAYER_READY");
    REQUIRE(std::string(getMessageTypeName(MessageType::SERVER_WELCOME)) == "SERVER_WELCOME");
    REQUIRE(std::string(getMessageTypeName(MessageType::ENTITY_SPAWN)) == "ENTITY_SPAWN");
    REQUIRE(std::string(getMessageTypeName(MessageType::ENTITY_STATE)) == "ENTITY_STATE");
    REQUIRE(std::string(getMessageTypeName(MessageType::ENTITY_DESTROY)) == "ENTITY_DESTROY");
    REQUIRE(std::string(getMessageTypeName(MessageType::PLAYER_SPAWN)) == "PLAYER_SPAWN");
    REQUIRE(std::string(getMessageTypeName(MessageType::PLAYER_HIT)) == "PLAYER_HIT");
    REQUIRE(std::string(getMessageTypeName(MessageType::PLAYER_DEATH)) == "PLAYER_DEATH");
    REQUIRE(std::string(getMessageTypeName(MessageType::PLAYER_RESPAWN)) == "PLAYER_RESPAWN");
    REQUIRE(std::string(getMessageTypeName(MessageType::GAME_OVER)) == "GAME_OVER");
    REQUIRE(std::string(getMessageTypeName(MessageType::SERVER_SNAPSHOT)) == "SERVER_SNAPSHOT");
}

TEST_CASE("getMessageTypeName returns UNKNOWN for invalid type", "[protocol][utility]") {
    REQUIRE(std::string(getMessageTypeName(static_cast<MessageType>(255))) == "UNKNOWN");
}

// ============================================================
// Input Flags Tests
// ============================================================

TEST_CASE("Input flags combine correctly", "[protocol][input]") {
    uint8_t flags = INPUT_UP | INPUT_LEFT | INPUT_SHOOT;

    REQUIRE(flags & INPUT_UP);
    REQUIRE(flags & INPUT_LEFT);
    REQUIRE(flags & INPUT_SHOOT);
    REQUIRE_FALSE(flags & INPUT_DOWN);
    REQUIRE_FALSE(flags & INPUT_RIGHT);
}

TEST_CASE("INPUT_NONE has no bits set", "[protocol][input]") {
    REQUIRE(INPUT_NONE == 0);
    REQUIRE_FALSE(INPUT_NONE & INPUT_UP);
    REQUIRE_FALSE(INPUT_NONE & INPUT_DOWN);
    REQUIRE_FALSE(INPUT_NONE & INPUT_LEFT);
    REQUIRE_FALSE(INPUT_NONE & INPUT_RIGHT);
    REQUIRE_FALSE(INPUT_NONE & INPUT_SHOOT);
}
