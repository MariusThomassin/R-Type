// R-Type NetworkIdManager Tests
// Tests for network ID management system

#include <catch2/catch_all.hpp>
#include <iostream>
#include "server/NetworkIdManager.hpp"
#include "engine/ecs/core/Entity.hpp"

using namespace rtype::server;
using namespace rtype::ecs;

TEST_CASE("NetworkIdManager allocates unique IDs", "[NetworkIdManager][basic]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity1(1);
    rtype::ecs::Entity entity2(2);

    uint32_t netId1 = manager.allocate(entity1);
    uint32_t netId2 = manager.allocate(entity2);

    REQUIRE(netId1 == 1);
    REQUIRE(netId2 == 2);
    REQUIRE(netId1 != netId2);
}

TEST_CASE("NetworkIdManager starts IDs from 1", "[NetworkIdManager][basic]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity(1);
    uint32_t netId = manager.allocate(entity);

    REQUIRE(netId == 1);
}

TEST_CASE("NetworkIdManager handles entity recycling", "[NetworkIdManager][recycling]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity(1);
    uint32_t netId1 = manager.allocate(entity);
    REQUIRE(netId1 == 1);

    manager.remove(entity);

    // Allocate same entity ID again (simulating Registry recycling)
    uint32_t netId2 = manager.allocate(entity);
    // Should get a NEW network ID, not the old one
    REQUIRE(netId2 == 2);
    REQUIRE(netId2 != netId1);
}

TEST_CASE("NetworkIdManager removes entity correctly", "[NetworkIdManager][remove]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity(1);
    manager.allocate(entity);

    REQUIRE(manager.hasNetworkId(entity));

    manager.remove(entity);

    REQUIRE_FALSE(manager.hasNetworkId(entity));
    REQUIRE(manager.getNetworkId(entity) == 0);
}

TEST_CASE("NetworkIdManager removes by network ID", "[NetworkIdManager][remove]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity(1);
    uint32_t netId = manager.allocate(entity);

    REQUIRE(manager.hasNetworkId(entity));

    manager.removeByNetworkId(netId);

    REQUIRE_FALSE(manager.hasNetworkId(entity));
    REQUIRE(manager.getEntity(netId).id == rtype::ecs::NULL_ENTITY);
}

TEST_CASE("NetworkIdManager getNetworkId returns 0 for unknown entity", "[NetworkIdManager][lookup]") {
    NetworkIdManager manager;

    rtype::ecs::Entity unknownEntity(999);
    uint32_t netId = manager.getNetworkId(unknownEntity);

    REQUIRE(netId == 0);
}

TEST_CASE("NetworkIdManager getEntity returns NULL_ENTITY for unknown network ID", "[NetworkIdManager][lookup]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity = manager.getEntity(999);

    REQUIRE(entity.id == rtype::ecs::NULL_ENTITY);
}

TEST_CASE("NetworkIdManager hasNetworkId returns false for untracked entity", "[NetworkIdManager][lookup]") {
    NetworkIdManager manager;

    rtype::ecs::Entity unknownEntity(999);

    REQUIRE_FALSE(manager.hasNetworkId(unknownEntity));
}

TEST_CASE("NetworkIdManager bidirectional mapping works", "[NetworkIdManager][mapping]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity(5);
    uint32_t netId = manager.allocate(entity);

    REQUIRE(manager.getNetworkId(entity) == netId);
    REQUIRE(manager.getEntity(netId) == entity);
}

TEST_CASE("NetworkIdManager tracks multiple entities correctly", "[NetworkIdManager][multiple]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entities[] = {rtype::ecs::Entity(1), rtype::ecs::Entity(2), rtype::ecs::Entity(3)};
    uint32_t netIds[3];

    for (int i = 0; i < 3; ++i) {
        netIds[i] = manager.allocate(entities[i]);
    }

    // Verify all mappings
    for (int i = 0; i < 3; ++i) {
        REQUIRE(manager.getNetworkId(entities[i]) == netIds[i]);
        REQUIRE(manager.getEntity(netIds[i]) == entities[i]);
        REQUIRE(manager.hasNetworkId(entities[i]));
    }
}

TEST_CASE("NetworkIdManager getEntityCount is accurate", "[NetworkIdManager][count]") {
    NetworkIdManager manager;

    REQUIRE(manager.getEntityCount() == 0);

    rtype::ecs::Entity entity1(1);
    manager.allocate(entity1);

    REQUIRE(manager.getEntityCount() == 1);

    rtype::ecs::Entity entity2(2);
    manager.allocate(entity2);

    REQUIRE(manager.getEntityCount() == 2);

    manager.remove(entity1);

    REQUIRE(manager.getEntityCount() == 1);
}

TEST_CASE("NetworkIdManager clear resets all state", "[NetworkIdManager][reset]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity1(1);
    rtype::ecs::Entity entity2(2);
    manager.allocate(entity1);
    manager.allocate(entity2);

    REQUIRE(manager.getEntityCount() == 2);

    manager.clear();

    REQUIRE(manager.getEntityCount() == 0);
    REQUIRE_FALSE(manager.hasNetworkId(entity1));
    REQUIRE_FALSE(manager.hasNetworkId(entity2));
}

TEST_CASE("NetworkIdManager increments IDs after clear", "[NetworkIdManager][reset]") {
    NetworkIdManager manager;

    rtype::ecs::Entity entity(1);
    uint32_t netId1 = manager.allocate(entity);
    manager.clear();

    uint32_t netId2 = manager.allocate(entity);

    // After clear, IDs should still increment (never reuse)
    REQUIRE(netId2 > netId1);
}

TEST_CASE("NetworkIdManager removing unknown entity is safe", "[NetworkIdManager][safety]") {
    NetworkIdManager manager;

    rtype::ecs::Entity unknownEntity(999);

    // Should not crash
    REQUIRE_NOTHROW(manager.remove(unknownEntity));
}

TEST_CASE("NetworkIdManager removing by unknown network ID is safe", "[NetworkIdManager][safety]") {
    NetworkIdManager manager;

    // Should not crash
    REQUIRE_NOTHROW(manager.removeByNetworkId(999));
}
