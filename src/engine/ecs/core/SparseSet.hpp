/*
** R-Type ECS - SparseSet
** High-performance sparse set data structure for component storage
*/

#pragma once

#include "Types.hpp"
#include <vector>
#include <cassert>
#include <cstdint>
#include <limits>

namespace rtype::ecs {

    /**
     * @brief Sparse set for O(1) operations with cache-friendly iteration
     * 
     * Uses a sparse array (indexed by entity ID) pointing to a dense packed array.
     * This eliminates hash table overhead while maintaining O(1) operations.
     */
    class SparseSet {
    public:
        static constexpr std::size_t INVALID_INDEX = std::numeric_limits<std::size_t>::max();

        SparseSet() = default;
        ~SparseSet() = default;

        SparseSet(const SparseSet&) = default;
        SparseSet& operator=(const SparseSet&) = default;
        SparseSet(SparseSet&&) noexcept = default;
        SparseSet& operator=(SparseSet&&) noexcept = default;

        /**
         * @brief Reserve space for entities up to a maximum ID
         * @param maxId Maximum entity ID that will be stored
         */
        void reserve(std::size_t maxId) {
            if (maxId >= m_sparse.size()) {
                m_sparse.resize(maxId + 1, INVALID_INDEX);
            }
        }

        /**
         * @brief Add an entity to the set
         * @param entity The entity ID to add
         * @return Index in the dense array
         */
        std::size_t insert(EntityId entity) {
            assert(!contains(entity) && "Entity already in set");

            if (entity >= m_sparse.size()) {
                m_sparse.resize(entity + 1, INVALID_INDEX);
            }

            std::size_t denseIndex = m_dense.size();
            m_sparse[entity] = denseIndex;
            m_dense.push_back(entity);

            return denseIndex;
        }

        /**
         * @brief Remove an entity from the set using swap-and-pop
         * @param entity The entity ID to remove
         * @return The entity that was swapped into this position (or entity if it was last)
         */
        EntityId remove(EntityId entity) {
            assert(contains(entity) && "Entity not in set");

            std::size_t removedIndex = m_sparse[entity];
            EntityId lastEntity = m_dense.back();

            m_dense[removedIndex] = lastEntity;
            m_sparse[lastEntity] = removedIndex;

            m_dense.pop_back();
            m_sparse[entity] = INVALID_INDEX;

            return lastEntity;
        }

        /**
         * @brief Check if an entity is in the set
         */
        bool contains(EntityId entity) const {
            return entity < m_sparse.size() && m_sparse[entity] != INVALID_INDEX;
        }

        /**
         * @brief Get the dense index for an entity
         * @return Index in dense array, or INVALID_INDEX if not found
         */
        std::size_t getIndex(EntityId entity) const {
            if (entity >= m_sparse.size()) return INVALID_INDEX;
            return m_sparse[entity];
        }

        /**
         * @brief Get entity at dense index
         */
        EntityId getEntityAt(std::size_t index) const {
            assert(index < m_dense.size() && "Index out of bounds");
            return m_dense[index];
        }

        /**
         * @brief Get number of entities in the set
         */
        std::size_t size() const { return m_dense.size(); }

        /**
         * @brief Check if set is empty
         */
        bool empty() const { return m_dense.empty(); }

        /**
         * @brief Clear all entities
         */
        void clear() {
            for (EntityId entity : m_dense) {
                m_sparse[entity] = INVALID_INDEX;
            }
            m_dense.clear();
        }

        auto begin() { return m_dense.begin(); }
        auto end() { return m_dense.end(); }
        auto begin() const { return m_dense.begin(); }
        auto end() const { return m_dense.end(); }

        /**
         * @brief Get the dense array directly (for advanced iteration)
         */
        const std::vector<EntityId>& dense() const { return m_dense; }

    private:
        std::vector<std::size_t> m_sparse;  // Entity ID -> dense index (or INVALID_INDEX)
        std::vector<EntityId> m_dense;       // Packed array of entity IDs
    };

} // namespace rtype::ecs
