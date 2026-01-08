/*
** R-Type ECS - Registry
** Central manager for entities and components
*/

#pragma once

#include "ComponentArray.hpp"
#include "Entity.hpp"
#include "IComponent.hpp"
#include "IComponentArray.hpp"
#include "SparseSet.hpp"
#include "Types.hpp"

#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <functional>

namespace rtype::ecs {

    // Forward declarations for command system
    class CommandBuffer;
    class Transaction;

    /**
     * @brief Central registry managing all entities and their components
     * 
     * The Registry is the core of the ECS. It handles:
     * - Entity creation and destruction
     * - Component storage and retrieval
     * - Entity signature management
     * - Efficient entity queries with caching
     */
    class Registry {
        public:
            /**
             * @brief Construct a new Registry object
             */
            Registry() = default;
            /**
             * @brief Destroy the Registry object
             */
            ~Registry() = default;

            /**
             * @brief Disable copy semantics
             */
            Registry(const Registry&) = delete;
            /**
             * @brief Disable copy assignment
             */
            Registry& operator=(const Registry&) = delete;

            /**
             * @brief Enable move semantics
             */
            Registry(Registry&&) = default;
            /**
             * @brief Enable move assignment
             */
            Registry& operator=(Registry&&) = default;

            // ==================== Entity Management ====================

            /**
             * @brief Create a new entity
             * @return The created entity
             */
            Entity createEntity();

            /**
             * @brief Destroy an entity and all its components
             * @param entity The entity to destroy
             */
            void destroyEntity(EntityId entity);

            /**
             * @brief Batch destroy multiple entities (more efficient than individual destroys)
             * @param entities Vector of entity IDs to destroy
             * 
             * This is significantly faster than calling destroyEntity() in a loop
             * when destroying many entities (e.g., offscreen bullets).
             */
            void destroyEntities(const std::vector<EntityId>& entities);

            /**
             * @brief Check if an entity exists
             * @param entity The entity ID to check
             * @return true if the entity exists
             */
            bool entityExists(EntityId entity) const;

            /**
             * @brief Get the number of active entities
             * @return Count of entities
             */
            std::size_t getEntityCount() const;

            // ==================== Deferred Operations ====================

            /**
             * @brief Queue an entity for deferred destruction
             * @param entity The entity to destroy
             * 
             * Entity will be destroyed when flushDeferred() is called.
             * Safe to call during iteration.
             */
            void destroyEntityDeferred(EntityId entity);

            /**
             * @brief Queue multiple entities for deferred destruction
             * @param entities Vector of entity IDs to destroy
             */
            void destroyEntitiesDeferred(const std::vector<EntityId>& entities);

            /**
             * @brief Execute all deferred operations
             * @return Number of operations executed
             * 
             * Call once per frame, typically at end of update cycle.
             */
            std::size_t flushDeferred();

            /**
             * @brief Get count of pending deferred operations
             */
            std::size_t getDeferredCount() const;

            // ==================== Component Management ====================

            /**
             * @brief Register a component type with the registry
             * @tparam T The component type to register
             * 
             * This must be called before using a component type.
             * Creates the storage array for this component type.
             * 
             * @note Automatically called by other methods if not already registered
             */
            template <typename T>
                void registerComponent()
                {
                    const ComponentTypeId typeId = getComponentTypeId<T>();
                    
                    assert(m_componentArrays.find(typeId) == m_componentArrays.end() && 
                        "Component type already registered");
                    
                    m_componentArrays[typeId] = std::make_shared<ComponentArray<T>>();
                }

            /**
             * @brief Add a component to an entity
             * @tparam T The component type
             * @param entity The target entity
             * @param component The component data to add
             */
            template <typename T>
                void addComponent(EntityId entity, T component)
                {
                    assert(entityExists(entity) && "Entity does not exist");
                    
                    getComponentArray<T>()->insertComponent(entity, std::move(component));
                    
                    auto& signature = m_signatures[entity];
                    signature.set(getComponentTypeId<T>(), true);
                }
            
            /**
             * @brief Construct and add a component to an entity in-place
             * @tparam T The component type
             * @tparam Args Constructor argument types
             * @param entity The target entity  
             * @param args Arguments forwarded to component constructor
             * @return Reference to the newly created component
             */
            template <typename T, typename... Args>
                T& emplaceComponent(EntityId entity, Args&&... args)
                {
                    assert(entityExists(entity) && "Entity does not exist");
                    
                    T component(std::forward<Args>(args)...);
                    addComponent<T>(entity, std::move(component));
                    return getComponent<T>(entity);
                }

            /**
             * @brief Remove a component from an entity
             * @tparam T The component type to remove
             * @param entity The target entity
             */
            template <typename T>
                void removeComponent(EntityId entity)
                {
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
                T& getComponent(EntityId entity)
                {
                    assert(entityExists(entity) && "Entity does not exist");
                    return getComponentArray<T>()->getComponent(entity);
                }
            
            template <typename T>
                const T& getComponent(EntityId entity) const
                {
                    assert(entityExists(entity) && "Entity does not exist");
                    return getComponentArray<T>()->getComponent(entity);
                }

            /**
             * @brief Safely try to get a component (returns nullptr if not found)
             * @tparam T The component type
             * @param entity The target entity
             * @return Pointer to component or nullptr if entity doesn't exist or lacks component
             */
            template <typename T>
                T* tryGetComponent(EntityId entity)
                {
                    if (!entityExists(entity)) {
                        return nullptr;
                    }
                    return getComponentArray<T>()->tryGetComponent(entity);
                }
            
            template <typename T>
                const T* tryGetComponent(EntityId entity) const
                {
                    if (!entityExists(entity)) {
                        return nullptr;
                    }
                    return getComponentArray<T>()->tryGetComponent(entity);
                }

            /**
             * @brief Check if an entity has a specific component
             * @tparam T The component type to check
             * @param entity The target entity
             * @return true if the entity exists and has the component
             */
            template <typename T>
                bool hasComponent(EntityId entity) const
                {
                    if (!entityExists(entity)) {
                        return false;
                    }
                    return m_signatures.at(entity).test(getComponentTypeId<T>());
                }
            
            /**
             * @brief Check if an entity has all specified components
             * @tparam Components The component types to check for
             * @param entity The target entity
             * @return true if the entity has all specified components
             */
            template <typename... Components>
                bool hasComponents(EntityId entity) const
                {
                    return (hasComponent<Components>(entity) && ...);
                }

            // ==================== Entity Queries ====================

            /**
             * @brief Get all entities that have the specified components
             * @tparam Components The required component types
             * @return Vector of matching entity IDs
             * 
             * @note This allocates memory. For performance-critical code,
             *       prefer forEach() methods which don't allocate.
             */
            template <typename... Components>
                std::vector<EntityId> getEntitiesWith() const
                {
                    static_assert(sizeof...(Components) > 0, "Must specify at least one component type");
                    
                    std::vector<EntityId> matchingEntities;
                    
                    // Optimization: start with the smallest component array to minimize iterations
                    const auto* smallestArray = findSmallestComponentArray<Components...>();
                    if (!smallestArray) {
                        return matchingEntities;
                    }
                    
                    // Build the required signature once
                    const Signature requiredSignature = buildSignature<Components...>();
                    
                    // Pre-allocate for best case (all entities match)
                    const auto& candidateEntities = smallestArray->getEntities();
                    matchingEntities.reserve(candidateEntities.size());
                    
                    // Check each candidate entity
                    for (const EntityId entityId : candidateEntities) {
                        if (const auto signatureIt = m_signatures.find(entityId);
                            signatureIt != m_signatures.end() &&
                            hasAllComponents(signatureIt->second, requiredSignature)) {
                            matchingEntities.push_back(entityId);
                        }
                    }
                    
                    return matchingEntities;
                }

            /**
             * @brief Efficiently iterate over entities with specified components (zero allocation)
             * @tparam Components The required component types
             * @tparam Func Function type (EntityId) -> void or (EntityId) -> bool
             * @param func Function to call for each matching entity
             * 
             * Features:
             * - Zero memory allocation
             * - Optimized iteration using smallest component array
             * - Early termination support (return false to break)
             */
            template <typename... Components, typename Func>
                void forEach(Func&& func)
                {
                    static_assert(sizeof...(Components) > 0, "Must specify at least one component type");
                    
                    const auto* smallestArray = findSmallestComponentArray<Components...>();
                    if (!smallestArray) {
                        return;
                    }
                    
                    const Signature requiredSignature = buildSignature<Components...>();
                    const auto& candidateEntities = smallestArray->getEntities();
                    
                    for (const EntityId entityId : candidateEntities) {
                        if (const auto signatureIt = m_signatures.find(entityId);
                            signatureIt != m_signatures.end() &&
                            hasAllComponents(signatureIt->second, requiredSignature)) {
                            
                            // Handle both void and bool-returning functions
                            if constexpr (std::is_same_v<std::invoke_result_t<Func, EntityId>, bool>) {
                                if (!func(entityId)) {
                                    break; // Early termination
                                }
                            } else {
                                func(entityId);
                            }
                        }
                    }
                }

            /**
             * @brief Ergonomic iteration with automatic component retrieval
             * @tparam Components The required component types
             * @tparam Func Function type (EntityId, Components&...) -> void
             * @param func Function receiving entity and all requested components
             * 
             * Most convenient iteration method - automatically retrieves and passes
             * all requested components to your callback function.
             */
            template <typename... Components, typename Func>
                void forEachWith(Func&& func)
                {
                    static_assert(sizeof...(Components) > 0, "Must specify at least one component type");
                    
                    const auto* smallestArray = findSmallestComponentArray<Components...>();
                    if (!smallestArray) {
                        return;
                    }
                    
                    const Signature requiredSignature = buildSignature<Components...>();
                    const auto& candidateEntities = smallestArray->getEntities();
                    
                    for (const EntityId entityId : candidateEntities) {
                        if (const auto signatureIt = m_signatures.find(entityId);
                            signatureIt != m_signatures.end() &&
                            hasAllComponents(signatureIt->second, requiredSignature)) {
                            
                            func(entityId, getComponent<Components>(entityId)...);
                        }
                    }
                }

            /**
             * @brief Maximum performance iteration for single component type
             * @tparam T The component type
             * @tparam Func Function type (EntityId, T&) -> void
             * @param func Function to call for each entity-component pair
             * 
             * Fastest possible iteration - directly iterates packed arrays without
             * any signature checks. Use when you only need one component type.
             */
            template <typename T, typename Func>
                void forEachDirect(Func&& func)
                {
                    auto* componentArray = getComponentArray<T>();
                    if (!componentArray) {
                        return;
                    }
                    
                    const auto& entities = componentArray->getEntities();
                    auto& components = componentArray->components();
                    const std::size_t count = entities.size();
                    
                    for (std::size_t i = 0; i < count; ++i) {
                        func(entities[i], components[i]);
                    }
                }

            /**
             * @brief Get all entity IDs
             * @return Vector of all entity IDs
             */
            std::vector<EntityId> getAllEntities() const;

            /**
             * @brief Get an entity's signature
             * @param entity The entity
             * @return The entity's component signature
             */
            Signature getSignature(EntityId entity) const;

        private:
            /**
             * @brief Get or create the typed component array for a component type
             * @tparam T The component type
             * @return Shared pointer to the component array
             */
            template <typename T>
                std::shared_ptr<ComponentArray<T>> getComponentArray()
                {
                    const ComponentTypeId typeId = getComponentTypeId<T>();
                    
                    auto it = m_componentArrays.find(typeId);
                    if (it == m_componentArrays.end()) {
                        registerComponent<T>();
                        it = m_componentArrays.find(typeId);
                    }
                    
                    return std::static_pointer_cast<ComponentArray<T>>(it->second);
                }
            
            template <typename T>
                std::shared_ptr<const ComponentArray<T>> getComponentArray() const
                {
                    const ComponentTypeId typeId = getComponentTypeId<T>();
                    
                    const auto it = m_componentArrays.find(typeId);
                    assert(it != m_componentArrays.end() && "Component type not registered");
                    
                    return std::static_pointer_cast<const ComponentArray<T>>(it->second);
                }

            /**
             * @brief Find the smallest component array among requested types
             * @tparam First First component type (determines return type)
             * @tparam Rest Remaining component types to compare
             * @return Pointer to smallest array as ComponentArray<First>*, or nullptr if any type is unregistered
             * 
             * Used to optimize iteration by starting with the smallest set.
             */
            template <typename First, typename... Rest>
                const ComponentArray<First>* findSmallestComponentArray() const
                {
                    ComponentTypeId smallestTypeId = getComponentTypeId<First>();
                    std::size_t minSize = std::numeric_limits<std::size_t>::max();
                    
                    // Check first type
                    auto it = m_componentArrays.find(getComponentTypeId<First>());
                    if (it == m_componentArrays.end()) {
                        return nullptr;
                    }
                    
                    minSize = it->second->size();
                    
                    // Check remaining types and find the smallest
                    if constexpr (sizeof...(Rest) > 0) {
                        bool allFound = ((findAndUpdateSmallest<Rest>(minSize, smallestTypeId)) && ...);
                        if (!allFound) {
                            return nullptr;
                        }
                    }
                    
                    // Return the smallest array cast to the appropriate type
                    auto smallestIt = m_componentArrays.find(smallestTypeId);
                    return static_cast<const ComponentArray<First>*>(smallestIt->second.get());
                }
            
            /**
             * @brief Helper to find and update the smallest array for a single component type
             */
            template <typename T>
                bool findAndUpdateSmallest(std::size_t& minSize, ComponentTypeId& smallestTypeId) const
                {
                    const ComponentTypeId typeId = getComponentTypeId<T>();
                    const auto it = m_componentArrays.find(typeId);
                    
                    if (it == m_componentArrays.end()) {
                        return false; // Component type not registered
                    }
                    
                    if (it->second->size() < minSize) {
                        minSize = it->second->size();
                        smallestTypeId = typeId;
                    }
                    
                    return true;
                }
            
            /**
             * @brief Build a signature from multiple component types
             */
            template <typename... Components>
                Signature buildSignature() const
                {
                    Signature signature;
                    (signature.set(getComponentTypeId<Components>()), ...);
                    return signature;
                }
            
            /**
             * @brief Check if an entity signature has all required components
             */
            static bool hasAllComponents(const Signature& entitySignature, const Signature& requiredSignature)
            {
                return (entitySignature & requiredSignature) == requiredSignature;
            }
            
            // ==================== Member Variables ====================
            
            /**
             * @brief Map of entity IDs to Entity objects
             */
            std::unordered_map<EntityId, Entity> m_entities;
            
            /**
             * @brief Map of entity IDs to their component signatures
             */
            std::unordered_map<EntityId, Signature> m_signatures;
            
            /**
             * @brief Next entity ID to assign (starts at 1, since 0 is NULL_ENTITY)
             */
            EntityId m_nextEntityId = 1;
            
            /**
             * @brief Queue of recycled entity IDs for reuse
             */
            std::queue<EntityId> m_availableIds;
            
            /**
             * @brief Map of component type IDs to their storage arrays
             */
            std::unordered_map<ComponentTypeId, std::shared_ptr<IComponentArray>> m_componentArrays;

            /**
             * @brief Queue of entities pending deferred destruction
             */
            std::vector<EntityId> m_deferredDestroyQueue;

            /**
             * @brief Mutex for deferred operations queue
             */
            mutable std::mutex m_deferredMutex;
        };
} // namespace rtype::ecs
