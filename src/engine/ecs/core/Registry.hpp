/*
** R-Type ECS - Registry
** Central manager for entities and components
*/

#pragma once

#include "ComponentArray.hpp"
#include "Entity.hpp"
#include "IComponent.hpp"
#include "IComponentArray.hpp"
#include "Types.hpp"

#include <memory>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

namespace rtype::ecs {

    /**
     * @brief Central registry managing all entities and their components
     * 
     * The Registry is the core of the ECS. It handles:
     * - Entity creation and destruction
     * - Component storage and retrieval
     * - Entity signature management
     */
    class Registry {
    public:
        Registry() = default;
        ~Registry() = default;

        Registry(const Registry&) = delete;
        Registry& operator=(const Registry&) = delete;

        Registry(Registry&&) = default;
        Registry& operator=(Registry&&) = default;

        // ==================== Entity Management ====================

        /**
         * @brief Create a new entity
         * @return The created entity
         */
        Entity createEntity() {
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

        /**
         * @brief Destroy an entity and all its components
         * @param entity The entity to destroy
         */
        void destroyEntity(EntityId entity) {
            assert(entityExists(entity) && "Entity does not exist");

            for (auto& [typeId, componentArray] : m_componentArrays) {
                componentArray->entityDestroyed(entity);
            }

            m_entities.erase(entity);
            m_signatures.erase(entity);

            m_availableIds.push(entity);
        }

        /**
         * @brief Check if an entity exists
         * @param entity The entity ID to check
         * @return true if the entity exists
         */
        bool entityExists(EntityId entity) const {
            return m_entities.find(entity) != m_entities.end();
        }

        /**
         * @brief Get the number of active entities
         * @return Count of entities
         */
        std::size_t getEntityCount() const {
            return m_entities.size();
        }

        // ==================== Component Management ====================

        /**
         * @brief Register a component type with the registry
         * @tparam T The component type
         * 
         * This must be called before using a component type.
         * Creates the storage array for this component type.
         */
        template <typename T>
        void registerComponent() {
            ComponentTypeId typeId = getComponentTypeId<T>();

            assert(m_componentArrays.find(typeId) == m_componentArrays.end() &&
                "Component type already registered");

            m_componentArrays[typeId] = std::make_shared<ComponentArray<T>>();
        }

        /**
         * @brief Add a component to an entity
         * @tparam T The component type
         * @param entity The entity
         * @param component The component data
         */
        template <typename T>
        void addComponent(EntityId entity, T component) {
            assert(entityExists(entity) && "Entity does not exist");

            getComponentArray<T>()->insertComponent(entity, std::move(component));

            auto& signature = m_signatures[entity];
            signature.set(getComponentTypeId<T>(), true);
        }

        /**
         * @brief Add a component to an entity (in-place construction)
         * @tparam T The component type
         * @tparam Args Constructor argument types
         * @param entity The entity
         * @param args Constructor arguments
         * @return Reference to the created component
         */
        template <typename T, typename... Args>
        T& emplaceComponent(EntityId entity, Args&&... args) {
            T component(std::forward<Args>(args)...);
            addComponent<T>(entity, std::move(component));
            return getComponent<T>(entity);
        }

        /**
         * @brief Remove a component from an entity
         * @tparam T The component type
         * @param entity The entity
         */
        template <typename T>
        void removeComponent(EntityId entity) {
            assert(entityExists(entity) && "Entity does not exist");

            getComponentArray<T>()->removeComponent(entity);

            auto& signature = m_signatures[entity];
            signature.set(getComponentTypeId<T>(), false);
        }

        /**
         * @brief Get a component from an entity
         * @tparam T The component type
         * @param entity The entity
         * @return Reference to the component
         */
        template <typename T>
        T& getComponent(EntityId entity) {
            assert(entityExists(entity) && "Entity does not exist");
            return getComponentArray<T>()->getComponent(entity);
        }

        template <typename T>
        const T& getComponent(EntityId entity) const {
            assert(entityExists(entity) && "Entity does not exist");
            return getComponentArray<T>()->getComponent(entity);
        }

        /**
         * @brief Try to get a component (returns nullptr if not found)
         * @tparam T The component type
         * @param entity The entity
         * @return Pointer to component or nullptr
         */
        template <typename T>
        T* tryGetComponent(EntityId entity) {
            if (!entityExists(entity)) return nullptr;
            return getComponentArray<T>()->tryGetComponent(entity);
        }

        template <typename T>
        const T* tryGetComponent(EntityId entity) const {
            if (!entityExists(entity)) return nullptr;
            return getComponentArray<T>()->tryGetComponent(entity);
        }

        /**
         * @brief Check if an entity has a component
         * @tparam T The component type
         * @param entity The entity
         * @return true if the entity has the component
         */
        template <typename T>
        bool hasComponent(EntityId entity) const {
            if (!entityExists(entity)) return false;
            return m_signatures.at(entity).test(getComponentTypeId<T>());
        }

        /**
         * @brief Check if an entity has all specified components
         * @tparam Components The component types to check
         * @param entity The entity
         * @return true if the entity has all components
         */
        template <typename... Components>
        bool hasComponents(EntityId entity) const {
            return (hasComponent<Components>(entity) && ...);
        }

        // ==================== Entity Queries ====================

        /**
         * @brief Get all entities with the specified components
         * @tparam Components The required component types
         * @return Vector of matching entity IDs
         */
        template <typename... Components>
        std::vector<EntityId> getEntitiesWith() const {
            std::vector<EntityId> result;

            Signature required;
            (required.set(getComponentTypeId<Components>()), ...);

            for (const auto& [entityId, signature] : m_signatures) {
                if ((signature & required) == required) {
                    result.push_back(entityId);
                }
            }

            return result;
        }

        /**
         * @brief Get all entity IDs
         * @return Vector of all entity IDs
         */
        std::vector<EntityId> getAllEntities() const {
            std::vector<EntityId> result;
            result.reserve(m_entities.size());

            for (const auto& [id, entity] : m_entities) {
                result.push_back(id);
            }

            return result;
        }

        /**
         * @brief Get an entity's signature
         * @param entity The entity
         * @return The entity's component signature
         */
        Signature getSignature(EntityId entity) const {
            assert(entityExists(entity) && "Entity does not exist");
            return m_signatures.at(entity);
        }

    private:
        /**
         * @brief Get the typed component array for a component type
         * @tparam T The component type
         * @return Shared pointer to the component array
         */
        template <typename T>
        std::shared_ptr<ComponentArray<T>> getComponentArray() {
            ComponentTypeId typeId = getComponentTypeId<T>();

            auto it = m_componentArrays.find(typeId);
            if (it == m_componentArrays.end()) {
                registerComponent<T>();
                it = m_componentArrays.find(typeId);
            }

            return std::static_pointer_cast<ComponentArray<T>>(it->second);
        }

        template <typename T>
        std::shared_ptr<const ComponentArray<T>> getComponentArray() const {
            ComponentTypeId typeId = getComponentTypeId<T>();

            auto it = m_componentArrays.find(typeId);
            assert(it != m_componentArrays.end() && "Component type not registered");

            return std::static_pointer_cast<const ComponentArray<T>>(it->second);
        }

    private:
        std::unordered_map<EntityId, Entity> m_entities;
        std::unordered_map<EntityId, Signature> m_signatures;

        EntityId m_nextEntityId = 1; // Start at 1, 0 is NULL_ENTITY
        std::queue<EntityId> m_availableIds;

        std::unordered_map<ComponentTypeId, std::shared_ptr<IComponentArray>> m_componentArrays;
    };

} // namespace rtype::ecs
