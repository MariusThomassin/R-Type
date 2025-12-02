/*
** R-Type ECS - Registry (New Implementation)
** Central manager for entities and components with optimized storage
*/

#pragma once

#include "ComponentStorage.hpp"
#include "Entity.hpp"
#include "EntityPool.hpp"
#include "EntityTypes.hpp"
#include "IComponent.hpp"
#include "IComponentArray.hpp"
#include "SparseSet.hpp"
#include "Types.hpp"
#include "View.hpp"

#include <cassert>
#include <memory>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace rtype::ecs {

    /**
     * @brief Central registry managing all entities and their components
     * 
     * Optimized implementation with:
     * - Entity pooling with generation tracking
     * - Sparse-set based component storage (no hash maps)
     * - Cached views for efficient queries
     * - Deferred entity destruction
     */
    class Registry {
    public:
        Registry();
        ~Registry();

        Registry(const Registry&) = delete;
        Registry& operator=(const Registry&) = delete;
        Registry(Registry&&) = default;
        Registry& operator=(Registry&&) = default;

        // ==================== Entity Management ====================

        /**
         * @brief Create a new entity
         * @return The created entity (backward compatible EntityId)
         */
        Entity createEntity();

        /**
         * @brief Create a new entity with handle (includes generation)
         * @return Handle with generation for safe references
         */
        EntityHandle createEntityHandle();

        /**
         * @brief Destroy an entity and all its components
         * @param entity The entity to destroy
         */
        void destroyEntity(EntityId entity);

        /**
         * @brief Destroy an entity by handle (safer)
         */
        void destroyEntity(EntityHandle handle);

        /**
         * @brief Queue entity for destruction at end of frame
         */
        void queueDestroy(EntityId entity);

        /**
         * @brief Process all queued entity destructions
         */
        void processDestroyQueue();

        /**
         * @brief Check if an entity exists
         */
        bool entityExists(EntityId entity) const;

        /**
         * @brief Check if entity handle is valid
         */
        bool isAlive(EntityHandle handle) const;

        /**
         * @brief Get the number of active entities
         */
        std::size_t getEntityCount() const;

        // ==================== Component Management ====================

        /**
         * @brief Register a component type with the registry
         */
        template <typename T>
        void registerComponent() {
            ComponentTypeId typeId = getComponentTypeId<T>();
            if (m_componentStorages.find(typeId) != m_componentStorages.end()) {
                return;  // Already registered
            }
            m_componentStorages[typeId] = std::make_shared<ComponentStorage<T>>();
        }

        /**
         * @brief Add a component to an entity
         */
        template <typename T>
        void addComponent(EntityId entity, T component) {
            assert(entityExists(entity) && "Entity does not exist");
            getStorage<T>()->insert(entity, std::move(component));
            m_signatures[entity].set(getComponentTypeId<T>(), true);
            invalidateCache();
        }

        /**
         * @brief Add a component to an entity (in-place construction)
         */
        template <typename T, typename... Args>
        T& emplaceComponent(EntityId entity, Args&&... args) {
            assert(entityExists(entity) && "Entity does not exist");
            auto* storage = getStorage<T>();
            T& comp = storage->emplace(entity, std::forward<Args>(args)...);
            m_signatures[entity].set(getComponentTypeId<T>(), true);
            invalidateCache();
            return comp;
        }

        /**
         * @brief Remove a component from an entity
         */
        template <typename T>
        void removeComponent(EntityId entity) {
            assert(entityExists(entity) && "Entity does not exist");
            getStorage<T>()->remove(entity);
            m_signatures[entity].set(getComponentTypeId<T>(), false);
            invalidateCache();
        }

        /**
         * @brief Get a component from an entity
         */
        template <typename T>
        T& getComponent(EntityId entity) {
            assert(entityExists(entity) && "Entity does not exist");
            return getStorage<T>()->get(entity);
        }

        template <typename T>
        const T& getComponent(EntityId entity) const {
            assert(entityExists(entity) && "Entity does not exist");
            return getStorage<T>()->get(entity);
        }

        /**
         * @brief Try to get a component (returns nullptr if not found)
         */
        template <typename T>
        T* tryGetComponent(EntityId entity) {
            if (!entityExists(entity)) return nullptr;
            return getStorage<T>()->tryGet(entity);
        }

        template <typename T>
        const T* tryGetComponent(EntityId entity) const {
            if (!entityExists(entity)) return nullptr;
            return getStorage<T>()->tryGet(entity);
        }

        /**
         * @brief Check if an entity has a component
         */
        template <typename T>
        bool hasComponent(EntityId entity) const {
            if (!entityExists(entity)) return false;
            auto it = m_signatures.find(entity);
            if (it == m_signatures.end()) return false;
            return it->second.test(getComponentTypeId<T>());
        }

        /**
         * @brief Check if an entity has all specified components
         */
        template <typename... Components>
        bool hasComponents(EntityId entity) const {
            return (hasComponent<Components>(entity) && ...);
        }

        // ==================== Entity Queries ====================

        /**
         * @brief Get a cached view of entities with specified components
         * 
         * This is the preferred way to query entities - no allocation per call.
         */
        template <typename... Components>
        View<Components...> view() {
            return View<Components...>(getStorage<Components>()...);
        }

        /**
         * @brief Single-component view (most efficient)
         */
        template <typename T>
        SingleView<T> view() {
            return SingleView<T>(getStorage<T>());
        }

        /**
         * @brief Get all entities with the specified components (legacy API)
         * 
         * Note: Prefer using view() for better performance.
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
         */
        std::vector<EntityId> getAllEntities() const;

        /**
         * @brief Get an entity's signature
         */
        Signature getSignature(EntityId entity) const;

        /**
         * @brief Reserve capacity for expected entities/components
         */
        void reserve(std::size_t entityCount);

    private:
        template <typename T>
        ComponentStorage<T>* getStorage() {
            ComponentTypeId typeId = getComponentTypeId<T>();
            auto it = m_componentStorages.find(typeId);
            if (it == m_componentStorages.end()) {
                registerComponent<T>();
                it = m_componentStorages.find(typeId);
            }
            return static_cast<ComponentStorage<T>*>(it->second.get());
        }

        template <typename T>
        const ComponentStorage<T>* getStorage() const {
            ComponentTypeId typeId = getComponentTypeId<T>();
            auto it = m_componentStorages.find(typeId);
            assert(it != m_componentStorages.end() && "Component type not registered");
            return static_cast<const ComponentStorage<T>*>(it->second.get());
        }

        void invalidateCache() {
            m_cacheValid = false;
        }

    private:
        EntityPool m_entityPool;
        
        // Active entities and their signatures
        std::unordered_map<EntityId, Signature> m_signatures;
        std::unordered_map<EntityId, Entity> m_entities;  // Legacy compatibility

        // Component storage
        std::unordered_map<ComponentTypeId, std::shared_ptr<IComponentArray>> m_componentStorages;

        // Deferred destruction
        std::vector<EntityId> m_destroyQueue;

        // Cache invalidation flag
        bool m_cacheValid = false;
    };

} // namespace rtype::ecs
