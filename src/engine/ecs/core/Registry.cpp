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
