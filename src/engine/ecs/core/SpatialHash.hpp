/*
** R-Type Engine - SpatialHash
** Spatial partitioning for efficient collision detection
*/

#pragma once

#include "engine/ecs/core/Types.hpp"
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <functional>

namespace rtype::ecs {

    /**
     * @brief Axis-Aligned Bounding Box
     */
    struct AABB {
        float x, y;           // Top-left corner
        float width, height;

        float left() const { return x; }
        float right() const { return x + width; }
        float top() const { return y; }
        float bottom() const { return y + height; }

        float centerX() const { return x + width * 0.5f; }
        float centerY() const { return y + height * 0.5f; }

        bool intersects(const AABB& other) const {
            return left() < other.right() && right() > other.left() &&
                   top() < other.bottom() && bottom() > other.top();
        }
    };

    /**
     * @brief Spatial hash grid for O(1) broad-phase collision detection
     *
     * Divides the world into a grid of cells. Entities are inserted into
     * all cells they overlap. Collision queries only check entities in
     * the same or neighboring cells.
     *
     * Performance:
     * - Insert: O(cells overlapped), typically O(1)
     * - Query: O(entities in overlapping cells), typically O(k) where k << N
     * - Overall collision detection: O(N) instead of O(N²)
     */
    class SpatialHash {
    public:
        /**
         * @brief Construct a spatial hash with given cell size
         * @param cellSize Size of each grid cell (should be ~largest entity size)
         */
        explicit SpatialHash(float cellSize = 64.0f)
            : m_cellSize(cellSize)
            , m_invCellSize(1.0f / cellSize)
        {}

        /**
         * @brief Clear all entities from the grid
         */
        void clear() {
            m_cells.clear();
            m_entityCells.clear();
        }

        /**
         * @brief Insert an entity into the spatial hash
         * @param entity Entity ID to insert
         * @param bounds AABB of the entity
         */
        void insert(EntityId entity, const AABB& bounds) {
            // Calculate which cells this entity overlaps
            int minCellX = static_cast<int>(std::floor(bounds.left() * m_invCellSize));
            int maxCellX = static_cast<int>(std::floor(bounds.right() * m_invCellSize));
            int minCellY = static_cast<int>(std::floor(bounds.top() * m_invCellSize));
            int maxCellY = static_cast<int>(std::floor(bounds.bottom() * m_invCellSize));

            std::vector<CellKey> cellKeys;
            cellKeys.reserve((maxCellX - minCellX + 1) * (maxCellY - minCellY + 1));

            for (int x = minCellX; x <= maxCellX; ++x) {
                for (int y = minCellY; y <= maxCellY; ++y) {
                    CellKey key = makeCellKey(x, y);
                    m_cells[key].push_back(entity);
                    cellKeys.push_back(key);
                }
            }

            m_entityCells[entity] = std::move(cellKeys);
        }

        /**
         * @brief Remove an entity from the spatial hash
         * @param entity Entity ID to remove
         */
        void remove(EntityId entity) {
            auto it = m_entityCells.find(entity);
            if (it == m_entityCells.end()) {
                return;
            }

            for (CellKey key : it->second) {
                auto cellIt = m_cells.find(key);
                if (cellIt != m_cells.end()) {
                    auto& entities = cellIt->second;
                    entities.erase(
                        std::remove(entities.begin(), entities.end(), entity),
                        entities.end()
                    );
                    if (entities.empty()) {
                        m_cells.erase(cellIt);
                    }
                }
            }

            m_entityCells.erase(it);
        }

        /**
         * @brief Query entities that potentially collide with the given bounds
         * @param bounds AABB to query
         * @param results Vector to store potential collision candidates
         */
        void query(const AABB& bounds, std::vector<EntityId>& results) const {
            results.clear();

            int minCellX = static_cast<int>(std::floor(bounds.left() * m_invCellSize));
            int maxCellX = static_cast<int>(std::floor(bounds.right() * m_invCellSize));
            int minCellY = static_cast<int>(std::floor(bounds.top() * m_invCellSize));
            int maxCellY = static_cast<int>(std::floor(bounds.bottom() * m_invCellSize));

            // Use a set to avoid duplicates when entity spans multiple cells
            std::unordered_map<EntityId, bool> seen;

            for (int x = minCellX; x <= maxCellX; ++x) {
                for (int y = minCellY; y <= maxCellY; ++y) {
                    CellKey key = makeCellKey(x, y);
                    auto it = m_cells.find(key);
                    if (it != m_cells.end()) {
                        for (EntityId entity : it->second) {
                            if (!seen[entity]) {
                                seen[entity] = true;
                                results.push_back(entity);
                            }
                        }
                    }
                }
            }
        }

        /**
         * @brief Get all unique collision pairs (no duplicates)
         * @param callback Function to call for each potential collision pair
         * 
         * This iterates all entities and their potential collisions once,
         * ensuring each pair is only reported once.
         */
        template <typename Func>
        void forEachPotentialPair(Func&& callback) const {
            std::unordered_map<uint64_t, bool> checkedPairs;

            for (const auto& [cellKey, entities] : m_cells) {
                // Check all pairs within this cell
                for (size_t i = 0; i < entities.size(); ++i) {
                    for (size_t j = i + 1; j < entities.size(); ++j) {
                        EntityId a = entities[i];
                        EntityId b = entities[j];

                        // Ensure consistent ordering for pair key
                        if (a > b) std::swap(a, b);
                        uint64_t pairKey = (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);

                        if (!checkedPairs[pairKey]) {
                            checkedPairs[pairKey] = true;
                            callback(a, b);
                        }
                    }
                }
            }
        }

        /**
         * @brief Get the number of cells in use
         */
        std::size_t getCellCount() const { return m_cells.size(); }

        /**
         * @brief Get the number of entities tracked
         */
        std::size_t getEntityCount() const { return m_entityCells.size(); }

        /**
         * @brief Get cell size
         */
        float getCellSize() const { return m_cellSize; }

        /**
         * @brief Set cell size (will require rebuild)
         */
        void setCellSize(float size) {
            m_cellSize = size;
            m_invCellSize = 1.0f / size;
        }

    private:
        using CellKey = int64_t;

        /**
         * @brief Create a unique key for a cell coordinate
         */
        static CellKey makeCellKey(int x, int y) {
            // Pack x and y into a single 64-bit key
            // Using high bits for x, low bits for y
            return (static_cast<int64_t>(x) << 32) | (static_cast<uint32_t>(y));
        }

        float m_cellSize;
        float m_invCellSize;

        // Map from cell key to list of entities in that cell
        std::unordered_map<CellKey, std::vector<EntityId>> m_cells;

        // Map from entity to the cells it occupies (for removal)
        std::unordered_map<EntityId, std::vector<CellKey>> m_entityCells;
    };

} // namespace rtype::ecs
