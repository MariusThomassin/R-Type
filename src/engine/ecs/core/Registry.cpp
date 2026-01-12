/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Registry - Central manager for entities and components
*/

#include "Registry.hpp"

namespace rtype::ecs {

    Entity Registry::createEntity()
    {
        EntityId id;

        if (!m_availableIds.empty()) {
            // Reuse a recycled ID
            id = m_availableIds.front();
            m_availableIds.pop();
        } else {
            // Generate new ID
            id = m_nextEntityId++;
        }

        Entity entity(id);
        m_entities[id] = entity;
        m_signatures[id] = Signature();

        return entity;
    }

    void Registry::destroyEntity(EntityId entity)
    {
        assert(entityExists(entity) && "Entity does not exist");

        for (auto& [typeId, componentArray] : m_componentArrays) {
            componentArray->entityDestroyed(entity);
        }

        m_entities.erase(entity);
        m_signatures.erase(entity);

        m_availableIds.push(entity);
    }

    void Registry::destroyEntities(const std::vector<EntityId>& entities)
    {
        if (entities.empty()) return;

        for (EntityId entityId : entities) {
            if (!entityExists(entityId)) continue;

            for (auto& [typeId, componentArray] : m_componentArrays) {
                componentArray->entityDestroyed(entityId);
            }

            m_entities.erase(entityId);
            m_signatures.erase(entityId);
            m_availableIds.push(entityId);
        }
    }

    bool Registry::entityExists(EntityId entity) const
    {
        return m_entities.find(entity) != m_entities.end();
    }

    std::size_t Registry::getEntityCount() const
    {
        return m_entities.size();
    }

    void Registry::destroyEntityDeferred(EntityId entity)
    {
        std::lock_guard<std::mutex> lock(m_deferredMutex);
        m_deferredDestroyQueue.push_back(entity);
    }

    void Registry::destroyEntitiesDeferred(const std::vector<EntityId>& entities)
    {
        std::lock_guard<std::mutex> lock(m_deferredMutex);
        m_deferredDestroyQueue.insert(m_deferredDestroyQueue.end(), 
                                       entities.begin(), entities.end());
    }

    std::size_t Registry::flushDeferred()
    {
        std::vector<EntityId> entitiesToDestroy;
        
        {
            std::lock_guard<std::mutex> lock(m_deferredMutex);
            std::swap(entitiesToDestroy, m_deferredDestroyQueue);
        }

        if (entitiesToDestroy.empty()) {
            return 0;
        }

        // Remove duplicates
        std::sort(entitiesToDestroy.begin(), entitiesToDestroy.end());
        entitiesToDestroy.erase(
            std::unique(entitiesToDestroy.begin(), entitiesToDestroy.end()),
            entitiesToDestroy.end()
        );

        std::size_t count = entitiesToDestroy.size();
        destroyEntities(entitiesToDestroy);
        
        return count;
    }

    std::size_t Registry::getDeferredCount() const
    {
        std::lock_guard<std::mutex> lock(m_deferredMutex);
        return m_deferredDestroyQueue.size();
    }

    std::vector<EntityId> Registry::getAllEntities() const
    {
        std::vector<EntityId> result;
        result.reserve(m_entities.size());

        for (const auto& [id, entity] : m_entities) {
            result.push_back(id);
        }

        return result;
    }

    Signature Registry::getSignature(EntityId entity) const
    {
        assert(entityExists(entity) && "Entity does not exist");
        return m_signatures.at(entity);
    }

} // namespace rtype::ecs
