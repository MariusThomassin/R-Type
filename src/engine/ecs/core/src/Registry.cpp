/*
** R-Type ECS - Registry Implementation
** Compiled implementation of Registry functions
*/

#include "../RegistryNew.hpp"

namespace rtype::ecs {

    Registry::Registry() = default;
    Registry::~Registry() = default;

    Entity Registry::createEntity() {
        EntityHandle handle = m_entityPool.create();
        EntityId id = m_entityPool.toEntityId(handle);
        
        Entity entity(id);
        m_entities[id] = entity;
        m_signatures[id] = Signature();
        
        return entity;
    }

    EntityHandle Registry::createEntityHandle() {
        EntityHandle handle = m_entityPool.create();
        EntityId id = m_entityPool.toEntityId(handle);
        
        m_entities[id] = Entity(id);
        m_signatures[id] = Signature();
        
        return handle;
    }

    void Registry::destroyEntity(EntityId entity) {
        if (!entityExists(entity)) return;

        for (auto& [typeId, storage] : m_componentStorages) {
            storage->entityDestroyed(entity);
        }

        m_entities.erase(entity);
        m_signatures.erase(entity);
        
        EntityHandle handle = m_entityPool.fromEntityId(entity);
        m_entityPool.destroy(handle);
        
        invalidateCache();
    }

    void Registry::destroyEntity(EntityHandle handle) {
        if (!m_entityPool.isAlive(handle)) return;
        destroyEntity(m_entityPool.toEntityId(handle));
    }

    void Registry::queueDestroy(EntityId entity) {
        m_destroyQueue.push_back(entity);
    }

    void Registry::processDestroyQueue() {
        for (EntityId entity : m_destroyQueue) {
            destroyEntity(entity);
        }
        m_destroyQueue.clear();
    }

    bool Registry::entityExists(EntityId entity) const {
        return m_entities.find(entity) != m_entities.end();
    }

    bool Registry::isAlive(EntityHandle handle) const {
        return m_entityPool.isAlive(handle);
    }

    std::size_t Registry::getEntityCount() const {
        return m_entities.size();
    }

    std::vector<EntityId> Registry::getAllEntities() const {
        std::vector<EntityId> result;
        result.reserve(m_entities.size());
        for (const auto& [id, entity] : m_entities) {
            result.push_back(id);
        }
        return result;
    }

    Signature Registry::getSignature(EntityId entity) const {
        assert(entityExists(entity) && "Entity does not exist");
        return m_signatures.at(entity);
    }

    void Registry::reserve(std::size_t entityCount) {
        m_entityPool.reserve(static_cast<uint32_t>(entityCount));
        m_entities.reserve(entityCount);
        m_signatures.reserve(entityCount);
    }

} // namespace rtype::ecs
