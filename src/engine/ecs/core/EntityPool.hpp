/*
** R-Type ECS - EntityPool
** Pre-allocated entity pool with generation tracking
*/

#pragma once

#include "Types.hpp"
#include "EntityTypes.hpp"

#include <vector>
#include <cstdint>
#include <cassert>

namespace rtype::ecs {

    /**
     * @brief Pre-allocated pool of entities with generation tracking
     * 
     * Benefits:
     * - Eliminates per-entity heap allocations
     * - Generation counters prevent dangling reference bugs
     * - Free-list for O(1) entity recycling
     */
    class EntityPool {
    public:
        static constexpr uint32_t INITIAL_CAPACITY = 1024;
        static constexpr uint32_t FREE_LIST_END = std::numeric_limits<uint32_t>::max();

        /**
         * @brief Entity slot in the pool
         */
        struct Slot {
            uint32_t generation = 0;
            uint32_t next = FREE_LIST_END;  // Next free slot (if this slot is free)
            bool alive = false;
        };

        explicit EntityPool(uint32_t initialCapacity = INITIAL_CAPACITY) {
            m_slots.reserve(initialCapacity);
            // Index 0 is reserved for null entity
            m_slots.push_back({0, FREE_LIST_END, false});
        }

        /**
         * @brief Create a new entity
         * @return Handle to the created entity
         */
        EntityHandle create() {
            uint32_t index;

            if (m_freeListHead != FREE_LIST_END) {
                index = m_freeListHead;
                m_freeListHead = m_slots[index].next;
            } else {
                index = static_cast<uint32_t>(m_slots.size());
                m_slots.push_back({0, FREE_LIST_END, false});
            }

            Slot& slot = m_slots[index];
            slot.alive = true;
            slot.next = FREE_LIST_END;
            ++m_aliveCount;

            return EntityHandle{index, slot.generation};
        }

        /**
         * @brief Destroy an entity
         * @param handle Handle to the entity to destroy
         * @return true if entity was destroyed, false if already dead/invalid
         */
        bool destroy(EntityHandle handle) {
            if (!isAlive(handle)) {
                return false;
            }

            Slot& slot = m_slots[handle.index];
            slot.alive = false;
            ++slot.generation;  // Increment generation to invalidate old handles
            slot.next = m_freeListHead;
            m_freeListHead = handle.index;
            --m_aliveCount;

            return true;
        }

        /**
         * @brief Check if an entity handle is still valid
         */
        bool isAlive(EntityHandle handle) const {
            if (handle.index >= m_slots.size()) return false;
            const Slot& slot = m_slots[handle.index];
            return slot.alive && slot.generation == handle.generation;
        }

        /**
         * @brief Get entity index (for backward compatibility with EntityId)
         */
        EntityId toEntityId(EntityHandle handle) const {
            assert(isAlive(handle) && "Entity is not alive");
            return static_cast<EntityId>(handle.index);
        }

        /**
         * @brief Create handle from EntityId (unsafe - no generation check)
         */
        EntityHandle fromEntityId(EntityId id) const {
            if (id >= m_slots.size()) {
                return EntityHandle::null();
            }
            return EntityHandle{static_cast<uint32_t>(id), m_slots[id].generation};
        }

        /**
         * @brief Get number of alive entities
         */
        std::size_t aliveCount() const { return m_aliveCount; }

        /**
         * @brief Get total capacity (including dead slots)
         */
        std::size_t capacity() const { return m_slots.size(); }

        /**
         * @brief Reserve space for expected entities
         */
        void reserve(uint32_t capacity) {
            m_slots.reserve(capacity);
        }

        /**
         * @brief Clear all entities (does NOT reset generations)
         */
        void clear() {
            for (uint32_t i = 1; i < m_slots.size(); ++i) {
                if (m_slots[i].alive) {
                    ++m_slots[i].generation;
                    m_slots[i].alive = false;
                    m_slots[i].next = m_freeListHead;
                    m_freeListHead = i;
                }
            }
            m_aliveCount = 0;
        }

        /**
         * @brief Iterate over all alive entities
         */
        template <typename Func>
        void forEach(Func&& func) const {
            for (uint32_t i = 1; i < m_slots.size(); ++i) {
                if (m_slots[i].alive) {
                    func(EntityHandle{i, m_slots[i].generation});
                }
            }
        }

    private:
        std::vector<Slot> m_slots;
        uint32_t m_freeListHead = FREE_LIST_END;
        std::size_t m_aliveCount = 0;
    };

} // namespace rtype::ecs
