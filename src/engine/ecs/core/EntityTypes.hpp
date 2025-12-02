/*
** R-Type ECS - EntityTypes
** Entity ID with generation for safe entity references
*/

#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>

namespace rtype::ecs {

    /**
     * @brief Entity ID with generation counter for safe references
     * 
     * Combines an index (for fast lookup) with a generation counter
     * to detect when an entity slot has been recycled.
     */
    struct EntityHandle {
        uint32_t index = 0;
        uint32_t generation = 0;

        EntityHandle() = default;
        constexpr EntityHandle(uint32_t idx, uint32_t gen) : index(idx), generation(gen) {}

        bool operator==(const EntityHandle& other) const {
            return index == other.index && generation == other.generation;
        }

        bool operator!=(const EntityHandle& other) const {
            return !(*this == other);
        }

        bool operator<(const EntityHandle& other) const {
            if (index != other.index) return index < other.index;
            return generation < other.generation;
        }

        /**
         * @brief Check if this handle is null/invalid
         */
        bool isNull() const {
            return index == 0 && generation == 0;
        }

        /**
         * @brief Create a null entity handle
         */
        static EntityHandle null() {
            return EntityHandle{0, 0};
        }

        /**
         * @brief Pack into a single 64-bit value for hashing/storage
         */
        uint64_t packed() const {
            return (static_cast<uint64_t>(generation) << 32) | index;
        }

        /**
         * @brief Unpack from a 64-bit value
         */
        static EntityHandle unpack(uint64_t value) {
            return EntityHandle{
                static_cast<uint32_t>(value & 0xFFFFFFFF),
                static_cast<uint32_t>(value >> 32)
            };
        }
    };

    constexpr EntityHandle NULL_ENTITY_HANDLE = {0, 0};

} // namespace rtype::ecs

namespace std {
    template <>
    struct hash<rtype::ecs::EntityHandle> {
        std::size_t operator()(const rtype::ecs::EntityHandle& handle) const {
            return std::hash<uint64_t>{}(handle.packed());
        }
    };
}
