/*
** R-Type ECS - ComponentStorage
** Optimized component storage using sparse sets
*/

#pragma once

#include "IComponentArray.hpp"
#include "IComponent.hpp"
#include "SparseSet.hpp"
#include "Types.hpp"

#include <cassert>
#include <vector>
#include <optional>

namespace rtype::ecs {

    /**
     * @brief Optimized component storage using sparse sets
     * @tparam T The component type to store
     * 
     * Replaces hash-map based ComponentArray with sparse set pattern:
     * - O(1) lookup without hash overhead
     * - Cache-friendly packed iteration
     * - Swap-and-pop removal
     */
    template <typename T>
    class ComponentStorage : public IComponentArray {
        static_assert(std::is_base_of<IComponent, T>::value,
            "T must derive from IComponent");

    public:
        ComponentStorage() = default;
        ~ComponentStorage() override = default;

        /**
         * @brief Reserve space for expected number of entities
         */
        void reserve(std::size_t capacity) {
            m_sparseSet.reserve(capacity);
            m_components.reserve(capacity);
        }

        /**
         * @brief Add a component to an entity
         * @param entity The entity ID
         * @param component The component data to store
         */
        void insert(EntityId entity, T component) {
            assert(!m_sparseSet.contains(entity) && "Component already exists for entity");

            m_sparseSet.insert(entity);
            m_components.push_back(std::move(component));
        }

        /**
         * @brief Construct component in-place
         * @param entity The entity ID
         * @param args Constructor arguments
         * @return Reference to the created component
         */
        template <typename... Args>
        T& emplace(EntityId entity, Args&&... args) {
            assert(!m_sparseSet.contains(entity) && "Component already exists for entity");

            m_sparseSet.insert(entity);
            m_components.emplace_back(std::forward<Args>(args)...);
            return m_components.back();
        }

        /**
         * @brief Remove a component from an entity
         * @param entity The entity ID
         */
        void remove(EntityId entity) {
            assert(m_sparseSet.contains(entity) && "Component does not exist for entity");

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
         */
        T& get(EntityId entity) {
            assert(m_sparseSet.contains(entity) && "Component does not exist for entity");
            return m_components[m_sparseSet.getIndex(entity)];
        }

        const T& get(EntityId entity) const {
            assert(m_sparseSet.contains(entity) && "Component does not exist for entity");
            return m_components[m_sparseSet.getIndex(entity)];
        }

        /**
         * @brief Try to get a component (returns nullptr if not found)
         */
        T* tryGet(EntityId entity) {
            if (!m_sparseSet.contains(entity)) return nullptr;
            return &m_components[m_sparseSet.getIndex(entity)];
        }

        const T* tryGet(EntityId entity) const {
            if (!m_sparseSet.contains(entity)) return nullptr;
            return &m_components[m_sparseSet.getIndex(entity)];
        }

        void entityDestroyed(EntityId entity) override {
            if (m_sparseSet.contains(entity)) {
                remove(entity);
            }
        }

        bool hasComponent(EntityId entity) const override {
            return m_sparseSet.contains(entity);
        }

        std::size_t size() const override {
            return m_components.size();
        }

        /**
         * @brief Check if storage contains entity
         */
        bool contains(EntityId entity) const {
            return m_sparseSet.contains(entity);
        }

        /**
         * @brief Get entity ID at a specific index in the packed array
         */
        EntityId getEntityAt(std::size_t index) const {
            return m_sparseSet.getEntityAt(index);
        }

        /**
         * @brief Get all entities with this component
         */
        const std::vector<EntityId>& entities() const {
            return m_sparseSet.dense();
        }

        auto begin() { return m_components.begin(); }
        auto end() { return m_components.end(); }
        auto begin() const { return m_components.begin(); }
        auto end() const { return m_components.end(); }

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
