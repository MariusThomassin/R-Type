/*
** R-Type ECS - SafeRegistry
** Registry with generation-tracked entity handles
** Prevents dangling reference bugs when entities are recycled
*/

#pragma once

#include "ComponentArray.hpp"
#include "EntityPool.hpp"
#include "EntityTypes.hpp"
#include "IComponent.hpp"
#include "IComponentArray.hpp"
#include "SparseSet.hpp"
#include "Types.hpp"

#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

namespace rtype::ecs {

    /**
     * @brief Registry with safe entity handles using generation tracking
     * 
     * This registry uses EntityHandle (index + generation) instead of raw EntityId.
     * When an entity is destroyed and its slot is recycled, the generation increments,
     * automatically invalidating any stale handles to the old entity.
     * 
     * Usage:
     *   auto entity = registry.createEntity();
     *   registry.addComponent<Position>(entity, 0.0f, 0.0f);
     *   registry.destroyEntity(entity);
     *   // entity is now invalid - any code holding this handle will fail isValid() check
     */
    class SafeRegistry {
    public:
        SafeRegistry() : m_entityPool(1024) {}
        ~SafeRegistry() = default;

        SafeRegistry(const SafeRegistry&) = delete;
        SafeRegistry& operator=(const SafeRegistry&) = delete;

        SafeRegistry(SafeRegistry&&) = default;
        SafeRegistry& operator=(SafeRegistry&&) = default;

        // ==================== Entity Management ====================

        /**
         * @brief Create a new entity with a safe handle
         * @return Handle to the created entity
         */
        EntityHandle createEntity() {
            EntityHandle handle = m_entityPool.create();
            m_signatures[handle.index] = Signature();
            return handle;
        }

        /**
         * @brief Destroy an entity
         * @param entity Handle to the entity to destroy
         * @return true if entity was destroyed, false if handle was invalid
         */
        bool destroyEntity(EntityHandle entity) {
            if (!isValid(entity)) {
                return false;
            }

            // Notify all component arrays
            for (auto& [typeId, componentArray] : m_componentArrays) {
                componentArray->entityDestroyed(entity.index);
            }

            m_signatures.erase(entity.index);
            return m_entityPool.destroy(entity);
        }

        /**
         * @brief Check if an entity handle is still valid
         * 
         * Returns false if:
         * - The entity was never created
         * - The entity was destroyed
         * - The slot was recycled (generation mismatch)
         */
        bool isValid(EntityHandle entity) const {
            return m_entityPool.isAlive(entity);
        }

        /**
         * @brief Get the number of active entities
         */
        std::size_t getEntityCount() const {
            return m_entityPool.aliveCount();
        }

        // ==================== Component Management ====================

        /**
         * @brief Register a component type with the registry
         */
        template <typename T>
        void registerComponent() {
            ComponentTypeId typeId = getComponentTypeId<T>();

            if (m_componentArrays.find(typeId) == m_componentArrays.end()) {
                m_componentArrays[typeId] = std::make_shared<ComponentArray<T>>();
            }
        }

        /**
         * @brief Add a component to an entity
         */
        template <typename T>
        void addComponent(EntityHandle entity, T component) {
            if (!isValid(entity)) return;

            auto array = getComponentArray<T>();
            array->insertComponent(entity.index, std::move(component));

            m_signatures[entity.index].set(getComponentTypeId<T>(), true);
        }

        /**
         * @brief Add a component to an entity (in-place construction)
         */
        template <typename T, typename... Args>
        T& emplaceComponent(EntityHandle entity, Args&&... args) {
            T component(std::forward<Args>(args)...);
            addComponent<T>(entity, std::move(component));
            return getComponent<T>(entity);
        }

        /**
         * @brief Remove a component from an entity
         */
        template <typename T>
        void removeComponent(EntityHandle entity) {
            if (!isValid(entity)) return;

            getComponentArray<T>()->removeComponent(entity.index);
            m_signatures[entity.index].set(getComponentTypeId<T>(), false);
        }

        /**
         * @brief Get a component from an entity
         * @throws if entity is invalid or component doesn't exist
         */
        template <typename T>
        T& getComponent(EntityHandle entity) {
            assert(isValid(entity) && "Entity is not valid");
            return getComponentArray<T>()->getComponent(entity.index);
        }

        template <typename T>
        const T& getComponent(EntityHandle entity) const {
            assert(isValid(entity) && "Entity is not valid");
            return getComponentArray<T>()->getComponent(entity.index);
        }

        /**
         * @brief Try to get a component (returns nullptr if not found)
         */
        template <typename T>
        T* tryGetComponent(EntityHandle entity) {
            if (!isValid(entity)) return nullptr;
            return getComponentArray<T>()->tryGetComponent(entity.index);
        }

        template <typename T>
        const T* tryGetComponent(EntityHandle entity) const {
            if (!isValid(entity)) return nullptr;
            return getComponentArray<T>()->tryGetComponent(entity.index);
        }

        /**
         * @brief Check if an entity has a component
         */
        template <typename T>
        bool hasComponent(EntityHandle entity) const {
            if (!isValid(entity)) return false;
            auto it = m_signatures.find(entity.index);
            if (it == m_signatures.end()) return false;
            return it->second.test(getComponentTypeId<T>());
        }

        /**
         * @brief Check if an entity has all specified components
         */
        template <typename... Components>
        bool hasComponents(EntityHandle entity) const {
            return (hasComponent<Components>(entity) && ...);
        }

        // ==================== Entity Queries ====================

        /**
         * @brief Iterate over all entities with specified components without allocation
         */
        template <typename... Components, typename Func>
        void forEach(Func&& func) {
            const auto* smallestArray = findSmallestArray<Components...>();
            if (!smallestArray) return;

            Signature required;
            (required.set(getComponentTypeId<Components>()), ...);

            const auto& entities = smallestArray->getEntities();
            for (EntityId entityId : entities) {
                auto sigIt = m_signatures.find(entityId);
                if (sigIt != m_signatures.end() && 
                    (sigIt->second & required) == required) {
                    
                    // Convert to EntityHandle for callback
                    EntityHandle handle = m_entityPool.fromEntityId(entityId);
                    if (m_entityPool.isAlive(handle)) {
                        func(handle);
                    }
                }
            }
        }

        /**
         * @brief Iterate with components passed to callback
         */
        template <typename... Components, typename Func>
        void forEachWith(Func&& func) {
            const auto* smallestArray = findSmallestArray<Components...>();
            if (!smallestArray) return;

            Signature required;
            (required.set(getComponentTypeId<Components>()), ...);

            const auto& entities = smallestArray->getEntities();
            for (EntityId entityId : entities) {
                auto sigIt = m_signatures.find(entityId);
                if (sigIt != m_signatures.end() && 
                    (sigIt->second & required) == required) {
                    
                    EntityHandle handle = m_entityPool.fromEntityId(entityId);
                    if (m_entityPool.isAlive(handle)) {
                        func(handle, getComponentArray<Components>()->getComponent(entityId)...);
                    }
                }
            }
        }

        /**
         * @brief Get all entities with the specified components
         * @note Allocates a new vector - prefer forEach() for hot paths
         */
        template <typename... Components>
        std::vector<EntityHandle> getEntitiesWith() const {
            std::vector<EntityHandle> result;

            const auto* smallestArray = findSmallestArray<Components...>();
            if (!smallestArray) return result;

            Signature required;
            (required.set(getComponentTypeId<Components>()), ...);

            const auto& entities = smallestArray->getEntities();
            result.reserve(entities.size());

            for (EntityId entityId : entities) {
                auto sigIt = m_signatures.find(entityId);
                if (sigIt != m_signatures.end() && 
                    (sigIt->second & required) == required) {
                    
                    EntityHandle handle = m_entityPool.fromEntityId(entityId);
                    if (m_entityPool.isAlive(handle)) {
                        result.push_back(handle);
                    }
                }
            }

            return result;
        }

        // ==================== Conversion Utilities ====================

        /**
         * @brief Convert EntityHandle to raw EntityId (for legacy code)
         * @note Prefer using EntityHandle throughout for safety
         */
        EntityId toEntityId(EntityHandle handle) const {
            return m_entityPool.toEntityId(handle);
        }

        /**
         * @brief Create EntityHandle from raw EntityId
         * @note Handle may be invalid if entity was destroyed
         */
        EntityHandle fromEntityId(EntityId id) const {
            return m_entityPool.fromEntityId(id);
        }

    private:
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

        template <typename First, typename... Rest>
        const ComponentArray<First>* findSmallestArray() const {
            ComponentTypeId typeId = getComponentTypeId<First>();
            auto it = m_componentArrays.find(typeId);
            if (it == m_componentArrays.end()) return nullptr;

            const IComponentArray* smallest = it->second.get();
            std::size_t minSize = smallest->size();

            if constexpr (sizeof...(Rest) > 0) {
                findSmallestArrayHelper<Rest...>(minSize, smallest);
            }

            return static_cast<const ComponentArray<First>*>(smallest);
        }

        template <typename First, typename... Rest>
        void findSmallestArrayHelper(std::size_t& minSize, 
                                      const IComponentArray*& smallest) const {
            ComponentTypeId typeId = getComponentTypeId<First>();
            auto it = m_componentArrays.find(typeId);
            if (it != m_componentArrays.end() && it->second->size() < minSize) {
                minSize = it->second->size();
                smallest = it->second.get();
            }

            if constexpr (sizeof...(Rest) > 0) {
                findSmallestArrayHelper<Rest...>(minSize, smallest);
            }
        }

    private:
        EntityPool m_entityPool;
        std::unordered_map<EntityId, Signature> m_signatures;
        std::unordered_map<ComponentTypeId, std::shared_ptr<IComponentArray>> m_componentArrays;
    };

} // namespace rtype::ecs
