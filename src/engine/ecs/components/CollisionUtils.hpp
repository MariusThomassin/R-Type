/*
** R-Type ECS - CollisionUtils
** Collision detection utilities and helper functions
*/

#pragma once

#include "ColliderComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Static utility class for collision operations and calculations
     */
    class CollisionUtils {
        public:
            /**
             * @brief Combine collision layers using bitwise OR
             * @param a First collision layer
             * @param b Second collision layer
             * @return Combined collision layer
             */
            static CollisionLayer combineLayer(CollisionLayer a, CollisionLayer b);

            /**
             * @brief Check if collision layers intersect using bitwise AND
             * @param a First collision layer
             * @param b Second collision layer
             * @return Intersection result
             */
            static CollisionLayer intersectLayer(CollisionLayer a, CollisionLayer b);

            /**
             * @brief Check if two collision layers can collide
             * @param layerA First entity's collision layer
             * @param maskA First entity's collision mask
             * @param layerB Second entity's collision layer
             * @param maskB Second entity's collision mask
             * @return true if layers can collide
             */
            static bool canCollide(CollisionLayer layerA, CollisionLayer maskA, CollisionLayer layerB, CollisionLayer maskB);

            /**
             * @brief Check if a layer is contained within a mask
             * @param layer The layer to check
             * @param mask The mask to check against
             * @return true if layer is in mask
             */
            static bool isLayerInMask(CollisionLayer layer, CollisionLayer mask);

            // Prevent instantiation
            /**
             * @brief Deleted constructor to prevent instantiation
             */
            CollisionUtils() = delete;
            /**
             * @brief Deleted destructor to prevent instantiation
             */
            ~CollisionUtils() = delete;
            /**
             * @brief Deleted copy constructor to prevent instantiation
             */
            CollisionUtils(const CollisionUtils&) = delete;
            /**
             * @brief Deleted assignment operator to prevent instantiation
             */
            CollisionUtils& operator=(const CollisionUtils&) = delete;
        };
} // namespace rtype::ecs
