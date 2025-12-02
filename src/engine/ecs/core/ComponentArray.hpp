/*
** R-Type ECS - ComponentArray
** Type-safe, cache-friendly component storage using sparse set
*/

#pragma once

#include "IComponent.hpp"
#include "IComponentArray.hpp"
#include "SparseSet.hpp"
#include "Types.hpp"

#include <cassert>
#include <vector>

namespace rtype::ecs {

    /**
     * @brief Packed array for storing components of a single type
     * @tparam T The component type to store
     * 
     * Uses a sparse set pattern for O(1) access without hash table overhead
     * while keeping components packed in memory for cache-friendly iteration.
     */
    template <typename T>
    class ComponentArray : public IComponentArray {
        static_assert(std::is_base_of<IComponent, T>::value,
            "T must derive from IComponent");

    public:
        ComponentArray() = default;
        ~ComponentArray() override = default;

        /**
         * @brief Reserve space for expected number of entities
         * @param maxEntityId Maximum entity ID expected
         */
        void reserve(std::size_t maxEntityId) {
            m_sparseSet.reserve(maxEntityId);
            m_components.reserve(maxEntityId);
        }

        /**
         * @brief Add a component to an entity
         * @param entity The entity ID
         * @param component The component data to store
         */
        void insertComponent(EntityId entity, T component) {
            assert(!hasComponent(entity) && "Component already exists for entity");

            m_sparseSet.insert(entity);
            m_components.push_back(std::move(component));
        }

        /**
         * @brief Remove a component from an entity
         * @param entity The entity ID
         * 
         * Uses swap-and-pop to maintain packed array.
         */
        void removeComponent(EntityId entity) {
            assert(hasComponent(entity) && "Component does not exist for entity");

            std::size_t removedIndex = m_sparseSet.getIndex(entity);
            std::size_t lastIndex = m_components.size() - 1;

            if (removedIndex != lastIndex) {
                m_components[removedIndex] = std::move(m_components[lastIndex]);
            }

            m_components.pop_back();
            m_sparseSet.remove(entity);
        }

        /**
         * @brief Get a component for an entity
         * @param entity The entity ID
         * @return Reference to the component
         */
        T& getComponent(EntityId entity) {
            assert(hasComponent(entity) && "Component does not exist for entity");
            return m_components[m_sparseSet.getIndex(entity)];
        }

        /**
         * @brief Get a const component for an entity
         * @param entity The entity ID
         * @return Const reference to the component
         */
        const T& getComponent(EntityId entity) const {
            assert(hasComponent(entity) && "Component does not exist for entity");
            return m_components[m_sparseSet.getIndex(entity)];
        }

        /**
         * @brief Try to get a component (returns nullptr if not found)
         * @param entity The entity ID
         * @return Pointer to component or nullptr
         */
        T* tryGetComponent(EntityId entity) {
            if (!m_sparseSet.contains(entity)) {
                return nullptr;
            }
            return &m_components[m_sparseSet.getIndex(entity)];
        }

        const T* tryGetComponent(EntityId entity) const {
            if (!m_sparseSet.contains(entity)) {
                return nullptr;
            }
            return &m_components[m_sparseSet.getIndex(entity)];
        }

        void entityDestroyed(EntityId entity) override {
            if (hasComponent(entity)) {
                removeComponent(entity);
            }
        }

        bool hasComponent(EntityId entity) const override {
            return m_sparseSet.contains(entity);
        }

        std::size_t size() const override {
            return m_components.size();
        }

        auto begin() { return m_components.begin(); }
        auto end() { return m_components.end(); }
        auto begin() const { return m_components.begin(); }
        auto end() const { return m_components.end(); }

        /**
         * @brief Get entity ID at a specific index
         * @param index Index in the packed array
         * @return The entity ID
         */
        EntityId getEntityAtIndex(std::size_t index) const {
            assert(index < m_components.size() && "Index out of bounds");
            return m_sparseSet.getEntityAt(index);
        }

        /**
         * @brief Get all entities with this component type
         * @return Reference to vector of entity IDs
         */
        const std::vector<EntityId>& getEntities() const {
            return m_sparseSet.dense();
        }

        /**
         * @brief Check if storage contains entity
         */
        bool contains(EntityId entity) const {
            return m_sparseSet.contains(entity);
        }

        /**
         * @brief Access components directly (for cache-friendly iteration)
         */
        std::vector<T>& components() { return m_components; }
        const std::vector<T>& components() const { return m_components; }

    private:
        SparseSet m_sparseSet;
        std::vector<T> m_components;
    };

} // namespace rtype::ecs
